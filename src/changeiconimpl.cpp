#include "mainwindowimpl.h"
#include "changeiconimpl.h"
#include "channeliconitem.h"
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
//
ChangeIconImpl::ChangeIconImpl( QWidget * parent, QString filename, QString channelName)
        : QDialog(parent), m_filename(filename), m_channelName(channelName)
{
    setupUi(this);
    setWindowTitle(tr("Change Icon for \"%1\" channel").arg(channelName));
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setScene( new QGraphicsScene(this) );
    ChannelIconItem *activeItem = 0;
    QDir dir(":/channel");
    QFileInfoList list = dir.entryInfoList();
    m_x = m_y = 0;
    int w = 80;
    int h = 50;
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QPixmap pix1 = QPixmap(":/channel/"+fileInfo.fileName()).scaledToHeight(h-10, Qt::SmoothTransformation);
        QPixmap pix = QPixmap(w, h);
        QPainter *paint = new QPainter( &pix );
        paint->fillRect(pix.rect(), QBrush(Qt::white));

        paint->drawPixmap((w-pix1.width())/2, 0, pix1);
        paint->setFont(QFont("Times", 8));
        QString title = fileInfo.fileName().section(".", 0, -2);
        paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
        delete paint;
        ChannelIconItem *item = new ChannelIconItem(pix, ":/channel/"+fileInfo.fileName(), filename==":/channel/"+fileInfo.fileName(), this);
        m_list << PairIcon(":/channel/"+fileInfo.fileName(), QPixmap(":/channel/"+fileInfo.fileName()));
        if ( filename==":/channel/"+fileInfo.fileName() )
        {
            activeItem = item;
        }
        view->scene()->addItem( item );
        item->setPos(m_x, m_y);
        m_x += (w + 10);
        if ( m_x+w+10 >= 560 )
        {
            m_x = 0;
            m_y += (h + 10);
        }
        //QD << item->pos();
    }
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("externalIcons");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString s = settings.value("filename").toString();
        QPixmap pix1 = QPixmap(s).scaledToHeight(h-10, Qt::SmoothTransformation);
        QPixmap pix = QPixmap(w, h);
        QPainter *paint = new QPainter( &pix );
        paint->fillRect(pix.rect(), QBrush(Qt::white));

        paint->drawPixmap((w-pix1.width())/2, 0, pix1);
        paint->setFont(QFont("Times", 8));
        QString title = s.section("/", -1).section(".", 0, -2);
        paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
        delete paint;
        ChannelIconItem *item = new ChannelIconItem(pix, s, m_filename==s, this);
        if ( m_filename==s )
        {
            activeItem = item;
        }
        view->scene()->addItem( item );
        item->setPos(m_x, m_y);
        m_x += (w + 10);
        if ( m_x+w+10 >= 560 )
        {
            m_x = 0;
            m_y += (h + 10);
        }
    }
    settings.endArray();
    if ( activeItem )
    {
        view->centerOn(activeItem);
    }
}
//
void ChangeIconImpl::channelIconClicked(ChannelIconItem *item, bool doubleClick)
{
    QList<QGraphicsItem *> list = view->scene()->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        if ( *it )
        {
            ChannelIconItem *iconItem = (ChannelIconItem *)*it;
            iconItem->setSelected(*it == item);
        }
    }
    m_selectedFilename = item->filename();
    if ( doubleClick )
    {
        on_buttonBox_accepted();
        accept();
    }
}


void ChangeIconImpl::on_buttonBox_accepted()
{
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Options");
    settings.setValue("iconchannel-"+m_channelName, m_selectedFilename);
    settings.endGroup();
}


void ChangeIconImpl::on_filterEdit_textChanged(QString s)
{
    delete view->scene();
    view->setScene( new QGraphicsScene(this) );
    ChannelIconItem *activeItem = 0;
    m_x = m_y = 0;
    int w = 80;
    int h = 50;
    foreach(PairIcon pairIcon, m_list)
    {
        if ( pairIcon.icon().toLower().contains( s.toLower() ) )
        {
            QPixmap pix1 = pairIcon.pixmap().scaledToHeight(h-10, Qt::SmoothTransformation);
            QPixmap pix = QPixmap(w, h);
            QPainter *paint = new QPainter( &pix );
            paint->fillRect(pix.rect(), QBrush(Qt::white));

            paint->drawPixmap((w-pix1.width())/2, 0, pix1);
            paint->setFont(QFont("Times", 8));
            QString title = pairIcon.icon().section("/", -1).section(".", 0, -2);
            paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
            delete paint;
            ChannelIconItem *item = new ChannelIconItem(pix, pairIcon.icon(), m_filename==pairIcon.icon(), this);
            if ( m_filename==pairIcon.icon() )
            {
                activeItem = item;
            }
            view->scene()->addItem( item );
            item->setPos(m_x, m_y);
            m_x += (w + 10);
            if ( m_x+w+10 >= 560 )
            {
                m_x = 0;
                m_y += (h + 10);
            }

        }
        //QD << item->pos();
    }
    if ( activeItem )
    {
        view->centerOn(activeItem);
    }
}

void ChangeIconImpl::on_addButton_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Image"),
                "",
                tr("Images Files (*.* *)"));
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    if ( QPixmap( s ).isNull() )
    {
        QMessageBox::warning(this, tr("Image File"), tr("The file is not a valid image."));
        return;
    }
    int w = 80;
    int h = 50;
    QPixmap pix1 = QPixmap(s).scaledToHeight(h-10, Qt::SmoothTransformation);
    QPixmap pix = QPixmap(w, h);
    QPainter *paint = new QPainter( &pix );
    paint->fillRect(pix.rect(), QBrush(Qt::white));

    paint->drawPixmap((w-pix1.width())/2, 0, pix1);
    paint->setFont(QFont("Times", 8));
    QString title = s.section("/", -1).section(".", 0, -2);
    paint->drawText(pix.rect(), Qt::AlignHCenter|Qt::AlignBottom, title);
    delete paint;
    ChannelIconItem *item = new ChannelIconItem(pix, s, false, this);
    m_list << PairIcon(s, QPixmap(s));
    view->scene()->addItem( item );
    item->setPos(m_x, m_y);
    m_x += (w + 10);
    if ( m_x+w+10 >= 560 )
    {
        m_x = 0;
        m_y += (h + 10);
    }
    view->centerOn(item);
    QStringList list;
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("externalIcons");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        list << settings.value("filename").toString();
    }
    settings.endArray();
    list << s;
    settings.beginWriteArray("externalIcons");
    for (int i = 0; i < list.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("filename", list.at(i));
    }
    settings.endArray();

}

void ChangeIconImpl::deleteIcon(ChannelIconItem *item)
{
    m_selectedFilename = item->filename();
    QStringList list;
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("externalIcons");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        list << settings.value("filename").toString();
    }
    settings.endArray();
    list.removeAll(m_selectedFilename);
    settings.beginWriteArray("externalIcons");
    for (int i = 0; i < list.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("filename", list.at(i));
    }
    settings.endArray();
    delete item;
    if( m_filename == m_selectedFilename )
    {
        m_filename = QString();
    }
}

