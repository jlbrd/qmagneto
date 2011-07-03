#include "mainwindowimpl.h"
#include "changethumbimpl.h"
#include "channeliconitem.h"
#include <QDir>
#include <QUrl>
#include <QSqlError>
#include <QNetworkProxy>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ChangeThumbImpl::ChangeThumbImpl( QWidget * parent, PairIcon pair, QString text, bool isChannel)
        : QDialog(parent), m_pairIcon(pair), m_isChannel(isChannel)
{
    setupUi(this);
    m_main = (MainWindowImpl *)parent;
    view->setScene( new QGraphicsScene(this) );
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    filterEdit->setText(text);
}
//

void ChangeThumbImpl::on_buttonBox_accepted()
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    m_selectedPixmap.save(&buffer, "PNG");
    QD << m_selectedPixmap.size();
    QVariant clob(data);
    m_main->handler()->writeThumbnailInDB(clob, m_pairIcon.icon(), true);
    emit imageAvailable(
        PairIcon(
            m_pairIcon.icon(),
            QPixmap::fromImage( QImage::fromData( ( data ) ) )
        )
    );
}

void ChangeThumbImpl::on_buttonBox_rejected()
{
    // TODO
}
void ChangeThumbImpl::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent( event );
    on_find_clicked();
}
void ChangeThumbImpl::on_find_clicked()
{
    m_x = m_y = 0;
    QString search_string = filterEdit->text();
    QString s,content_type,image_size,coloration,site_search,safeFilter,search_url;
    delete view->scene();
    view->setScene( new QGraphicsScene(this) );
    view->setSceneRect(view->rect() );
    search_string=search_string.simplified().replace(" ","+");

    search_url="http://images.google.com/images?&q="+search_string+"&safe=active";
    if( m_isChannel ) {
    	search_url += "&as_sitesearch=www.lyngsat-logo.com";
   	}
    QNetworkRequest request(search_url);
    reply = manager.get(request);
    connect(reply, SIGNAL(finished()),
            SLOT(httpURL_done()));

    QD << "Searching URL:"<< search_url;
}

void ChangeThumbImpl::httpURL_done ()
{
    qApp->processEvents();
    if (reply->error())
    {
        QD << reply->error();
    }
    else
    {
        QByteArray r;

        r=QByteArray::fromPercentEncoding(reply->readAll());
        m_urlList = parse_html(QString::fromUtf8(r));
        QD<<m_urlList;
        if ( m_urlList.count() )
        {
            QString img = m_urlList.first();
            QD << "getThumbnail" <<img;
            m_url = img;
            QNetworkRequest request(m_url);
            reply = manager.get(request);
            connect(reply, SIGNAL(finished()),
                    SLOT(httpThumbnail_done()));
            m_urlList.pop_front();
        }
    }
}
QStringList ChangeThumbImpl::parse_html(QString html)
{
    QRegExp rx_start;
    rx_start.setPattern("setResults[(][[]");

    QRegExp rx_data;
    rx_data.setPattern("[[]\".*[]],");
    rx_data.setMinimal(true);

    QRegExp rx_href;
    rx_href.setPattern("(http://.*)[\\\\|\"]");
    rx_href.setMinimal(true);

    QRegExp rx_other;
    rx_other.setPattern(",[\"](.*)[\"]");
    rx_other.setMinimal(true);
    QString s;
    QString href_original_image, href_thumbnail_at_google, href_original_page, href_google_thumb_download;
    QString ID_google_thumb, href_thumbnail;
    QStringList href_thumbnail_list;
    int pos,pos2,i;

    pos = 0;
    pos = rx_start.indexIn(html, pos);
    if (pos==-1)
    {
        QD << "Download error";
        return QStringList();
    }
    html.remove(0,pos+rx_start.cap(0).size());
    pos=0;
    while ((pos = rx_data.indexIn(html, pos)) != -1 )
    {
        pos2 = i = 0;
        s=rx_data.cap(0);
        s.replace("\\","\\\\");
        while ((pos2 = rx_href.indexIn( s, pos2)) != -1)
        {
            switch (i)
            {
            case 0:
                href_thumbnail_at_google = rx_href.cap(1);
                break;
            case 1:
                href_original_page = rx_href.cap(1);
                break;
            case 2:
                href_original_image= rx_href.cap(1);
                href_original_image=QUrl(href_original_image).fromPercentEncoding(href_original_image.toLocal8Bit());
                break;
            case 3:
                href_google_thumb_download = rx_href.cap(1);
                //QD<<href_google_thumb_download;
                break;
            }
            pos2 += rx_href.matchedLength();
            i++;
        }
        pos2 = i = 0;
        while ((pos2 = rx_other.indexIn( s, pos2)) != -1)
        {
            switch (i)
            {
            case 1:
                ID_google_thumb=rx_other.cap(1);
                href_thumbnail = href_google_thumb_download
                                 + "?q=tbn:" + ID_google_thumb +
                                 href_thumbnail_at_google;
                href_thumbnail_list << "http:" + href_thumbnail.section(":http:", 1, 1);
                //href_thumbnail_list << href_thumbnail;
                break;
            }
            pos2 += rx_other.matchedLength();
            i++;
        }
        pos += rx_data.matchedLength();
        QD<<pos;
    }
    return href_thumbnail_list;
}
void ChangeThumbImpl::httpThumbnail_done()
{
    int w = 160;
    int h = 100;
    ChannelIconItem *activeItem = 0;
    qApp->processEvents();
    if ( reply->error() )
    {
        QD << reply->error();
    }
    else
    {
    	QD;
        QByteArray data;
        data = reply->readAll();
        if ( data.isEmpty() )
            return;
        QVariant clob(data);
        QPixmap pixdata = QPixmap::fromImage( QImage::fromData( ( data ) ) );
        if ( pixdata.isNull() )
            return;
        if( pixdata.width() > 250 ) {
        	pixdata = pixdata.scaledToWidth(250, Qt::SmoothTransformation);
        }
        QPixmap pix = pixdata.scaledToHeight(h-10, Qt::FastTransformation);
        ChannelIconItem *item = new ChannelIconItem(pix, pixdata, m_url, m_url==m_pairIcon.icon(), this);
        connect(item, SIGNAL(channelIconClicked(ChannelIconItem *, bool)), this, SLOT(channelIconClicked(ChannelIconItem *, bool)) );
        if ( m_url==m_pairIcon.icon() )
        {
            activeItem = item;
        }
        view->scene()->addItem( item );
        item->setPos(m_x, m_y);
        m_x += (w + 10);
        if ( m_x+w+10 >= view->sceneRect().width() )
        {
            m_x = 0;
            m_y += (h + 10);
        }
        if ( m_urlList.count() )
        {
            QString img = m_urlList.first();
            QD << "getThumbnail" <<img;
            m_url = img;
            QNetworkRequest request(m_url);
            reply = manager.get(request);
            connect(reply, SIGNAL(finished()),
                    SLOT(httpThumbnail_done()));
            m_urlList.pop_front();
        }
        else
        {
	        view->setSceneRect(view->scene()->itemsBoundingRect() );
       	}
    }
}
//
void ChangeThumbImpl::channelIconClicked(ChannelIconItem *item, bool doubleClick)
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
    m_selectedPixmap = item->pixmap();
    if ( doubleClick )
    {
        on_buttonBox_accepted();
        accept();
    }
}

void ChangeThumbImpl::on_addFromURL_clicked()
{
	QString URL = QInputDialog::getText(
		this,
		tr("Add Image From URL"),
		tr("Enter the Image URL to add")
	);
	if( !URL.isEmpty() ) {
	    QNetworkRequest request(URL);
	    reply = manager.get(request);
	    connect(reply, SIGNAL(finished()),
	            SLOT(httpThumbnail_done()));
	}
}
void ChangeThumbImpl::on_addFromFile_clicked()
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
    QPixmap pixdata = QPixmap(s);
    QPixmap pix = QPixmap(s).scaledToHeight(h-10, Qt::SmoothTransformation);
    ChannelIconItem *item = new ChannelIconItem(pix, pixdata, m_url, true, this);
    connect(item, SIGNAL(channelIconClicked(ChannelIconItem *, bool)), this, SLOT(channelIconClicked(ChannelIconItem *, bool)) );
    m_selectedFilename = item->filename();
    m_selectedPixmap = item->pixmap();
    view->scene()->addItem( item );
    item->setPos(m_x, m_y);
    item->setPos(m_x, m_y);
    m_x += (w + 10);
    if ( m_x+w+10 >= view->sceneRect().width() )
    {
        m_x = 0;
        m_y += (h + 10);
    }
    view->centerOn(item);
}
