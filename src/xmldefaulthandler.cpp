#include "xmldefaulthandler.h"
#include "graphicsrectitem.h"
#include "mainwindowimpl.h"
//
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QGraphicsScene>
#include <QSettings>
#include <QGraphicsView>
#include <QScrollBar>
#include <QTextCodec>
#include <QObject>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
QGraphicsView *viewP;
PairIcon::PairIcon(QString s, QPixmap p)
{
    m_icon = s;
    m_pixmap = p;
}

XmlDefaultHandler::XmlDefaultHandler(MainWindowImpl *main, QGraphicsView *programs)
        : QXmlDefaultHandler(), m_main(main), m_programsView(programs)
{
    m_getImages = new GetImages( m_imagesList, m_query);
    viewP = programs;
}
//
XmlDefaultHandler::~XmlDefaultHandler()
{
    if ( m_getImages )
    {
        delete m_getImages;
    }
}
//
bool XmlDefaultHandler::startElement( const QString & , const QString & , const QString & qName, const QXmlAttributes & atts )
{
    static bool isChannel = false;
    Balise balisePrecedente = m_balise;
    m_balise = None;
    if ( qName == "channel" )
    {
        isChannel = true;
        m_balise = Channel;
        for (int i=0; i< atts.count(); i++)
        {
            if ( atts.qName(i) == "id" )
            {
                m_chaineTV.id = atts.value(i);
            }
        }
    }
    else if ( qName == "display-name" )
    {
        m_balise = DisplayName;
    }
    else if ( qName == "title" )
    {
        m_balise = Title;
    }
    else if ( qName == "icon" )
    {
        if ( isChannel )
            m_chaineTV.icon = atts.value(0);
        else
            m_programTV.icon = atts.value(0);
        m_imagesList.append( atts.value(0) );
    }
    else if ( qName == "sub-title" )
    {
        m_balise = SubTitle;
    }
    else if ( qName == "desc" )
    {
        m_balise = Desc;
    }
    else if ( qName == "aspect" )
    {
        m_balise = Aspect;
    }
    else if ( qName == "category" )
    {
        m_balise = Category;
    }
    else if ( qName == "star-rating" )
    {
        m_balise = Star;
    }
    else if ( qName == "value" )
    {
        if ( balisePrecedente == Star )
            m_balise = Star;
        else
            m_balise = None;
    }
    else if ( qName == "programme" )
    {
        isChannel = false;
        for (int i=0; i< atts.count(); i++)
        {
            if ( atts.qName(i) == "start" )
            {
                m_programTV.start = QDateTime::fromString(atts.value(i).section(" ",0,0), "yyyyMMddhhmmss");
            }
            else if ( atts.qName(i) == "stop" )
            {
                m_programTV.stop = QDateTime::fromString(atts.value(i).section(" ",0,0), "yyyyMMddhhmmss");
            }
            else if ( atts.qName(i) == "channel" )
            {
                m_programTV.channel = atts.value(i);
            }
        }
    }
    else
    {
        m_balise = None;
    }
    return true;
}

bool XmlDefaultHandler::endElement( const QString & , const QString & , const QString & qName )
{
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    if ( qName == "channel" )
    {
        if ( (Qt::CheckState)settings.value(m_chaineTV.id+"-isEnabled", Qt::Checked).toInt() != Qt::Checked )
        {
            QD << QObject::tr("Channel %1 disabled in Channels dialog").arg(m_chaineTV.name);
            m_chaineTV.enabled = false;
        }
        else
            m_chaineTV.enabled = true;
        QString queryString = "insert into channels values(";
        queryString = queryString
                      + "'" + m_chaineTV.id.replace("'", "$") + "', "
                      + "'" + m_chaineTV.name.replace("'", "$") + "', "
                      + "'" + m_chaineTV.icon.replace("'", "$") + "', "
                      + "'" + QString::number(m_chaineTV.enabled).replace("'", "$") + "')";
        bool rc = m_query.exec(queryString);
        if (rc == false)
        {
            qDebug() << "Failed to insert record to db" << m_query.lastError();
            qDebug() << queryString;
        }
        m_TvChannelsList << m_chaineTV;
        m_chaineTV = TvChannel();
    }
    else if ( qName == "programme" )
    {
        if ( (Qt::CheckState)settings.value(m_programTV.channel+"-isEnabled", Qt::Checked).toInt() != Qt::Checked )
        {
            return true;
        }
        foreach(TvChannel channel, m_TvChannelsList)
        {
            if ( channel.id == m_programTV.channel )
            {
                m_programTV.channelName = channel.name;
                break;
            }
        }
        QString queryString = "insert into programs values(";
        queryString = queryString
                      + "'" + QString::number( m_programTV.start.toTime_t() ) + "', "
                      + "'" + QString::number( m_programTV.stop.toTime_t() ) + "', "
                      + "'" + m_programTV.channel.replace("'", "$") + "', "
                      + "'" + m_programTV.channelName.replace("'", "$") + "', "
                      + "'" + m_programTV.title.replace("'", "$") + "', "
                      + "'" + m_programTV.subTitle.replace("'", "$") + "', "
                      + "'" + m_programTV.category.join("|").replace("'", "$") + "', "
                      + "'" + m_programTV.resume.join("|").replace("'", "$") + "', "
                      + "'" + m_programTV.story.replace("'", "$") + "', "
                      + "'" + m_programTV.aspect.replace("'", "$") + "', "
                      + "'" + m_programTV.credits.replace("'", "$") + "', "
                      + "'" + m_programTV.director.replace("'", "$") + "', "
                      + "'" + m_programTV.star.replace("'", "$") + "', "
                      + "'" + m_programTV.icon.replace("'", "$")
                      +  "')";
        bool rc = m_query.exec(queryString);
        if (rc == false)
        {
            qDebug() << "Failed to insert record to db" << m_query.lastError();
            qDebug() << queryString;
        }
        if ( !m_programTV.icon.isEmpty() )
        {
            QByteArray data;
            QVariant clob(data);
            m_query.prepare("INSERT INTO images (icon, ok, data)"
                            "VALUES (:icon, :ok, :data)");
            m_query.bindValue(":icon", m_programTV.icon.replace("'", "$"));
            m_query.bindValue(":ok","0");
            m_query.bindValue(":data", clob);
            bool rc = m_query.exec();
            if (rc == false)
            {
                qDebug() << "Failed to insert record to db" << m_query.lastError();
                qDebug() << queryString;
            }
        }
        m_programTV = TvProgram();
    }
    settings.endGroup();
    return true;
}


bool XmlDefaultHandler::characters( const QString & ch )
{
    if ( ch.simplified().isEmpty() )
        return true;
    switch ( m_balise )
    {
    case Title:
        m_programTV.title = ch;
        break;
    case SubTitle:
        m_programTV.subTitle = ch;
        break;
    case Desc:
        if ( !m_programTV.resume.count() )
            m_programTV.resume << ch;
        else
            m_programTV.story = ch;
        break;
    case Aspect:
        m_programTV.aspect = ch;
        break;
    case Category:
        m_programTV.category << ch;
        break;
    case DisplayName:
        m_chaineTV.name = ch;
        break;
    case Star:
        m_programTV.star = ch;
        break;
    default:
        m_ch = ch;
        break;
    }
    return true;
}


bool XmlDefaultHandler::endDocument()
{
    m_query.exec("END TRANSACTION;");
    return true;
}

void XmlDefaultHandler::deplaceChaines(int )
{
    foreach(GraphicsRectItem *item, m_listeItemChaines)
    {
        item->setPos(m_programsView->horizontalScrollBar()->value(), item->y());
    }
}


void XmlDefaultHandler::deplaceHeures(int )
{
    foreach(GraphicsRectItem *item, m_listeItemHeures)
    {
        item->setPos(item->x(), m_programsView->verticalScrollBar()->value());
    }
}


bool XmlDefaultHandler::startDocument()
{
    connectDB();
    QString queryString = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
    m_query.exec(queryString);
    QStringList listTables;
    while ( m_query.next() )
    {
        listTables << m_query.value(0).toString();
    }
    foreach(QString table, listTables)
    {
        queryString =  "drop table " + table + ";";
        bool rc = m_query.exec(queryString);
        if (rc == false)
        {
            qDebug() << "Failed to delete table" << m_query.lastError();
            qDebug() << queryString;
        }
    }
    queryString = "create table channels ("
                  "id string,"
                  "name string,"
                  "icon string,"
                  "enabled string"
                  ")";

    m_query.exec(queryString);
    //
    queryString = "create table programs ("
                  "start int,"
                  "stop int,"
                  "channel string,"
                  "channelName string,"
                  "title string,"
                  "subTitle string,"
                  "category string,"
                  "resume string,"
                  "story string,"
                  "aspect string,"
                  "credits string,"
                  "director string,"
                  "star string,"
                  "icon string"
                  ")";
    //
    m_query.exec(queryString);
    queryString = "create table images ("
                  "icon string,"
                  "ok string,"
                  "data blob"
                  ")";

    m_query.exec(queryString);
    m_query.exec("BEGIN TRANSACTION;");
    return true;
}


void XmlDefaultHandler::init()
{
    m_TvChannelsList.clear();
    m_TvProgramsList.clear();
    m_programsSortedItemsList.clear();
    m_programsItemsList.clear();
    m_imagesList.clear();
}


void XmlDefaultHandler::currentTimeLinePosition()
{
    double x = QTime(0,0).secsTo( QTime::currentTime() )*(m_progWidth/1800.0);
    m_currentTimeLine->setLine
    (
        100+x-((m_hourBeginning*2)*m_progWidth),
        m_hourHeight,
        100+x-((m_hourBeginning*2)*m_progWidth) ,
        m_hourHeight+(m_TvChannelsList.count()*m_progHeight)
    );
    foreach(GraphicsRectItem *item, m_programsItemsList)
    {
        TvProgram prog = item->data(0).value<TvProgram>();
        if ( prog.start <= QDateTime::currentDateTime() && QDateTime::currentDateTime() <= prog.stop )
        {
            item->setInCurrentHour(true);
        }
        else
        {
            item->setInCurrentHour(false);
        }
        item->update();
    }
}


void XmlDefaultHandler::evening()
{
    double x = QTime(0,0).secsTo( QTime(21, 30) )*(m_progWidth/1800.0);
    x = 100+x-((m_hourBeginning*2)*m_progWidth);
    m_programsView->centerOn(x ,0);
}


QList<TvProgram> XmlDefaultHandler::eveningPrograms()
{
    QList<TvProgram> programsList;
    QDateTime evening = QDateTime( QDate(m_date), QTime(21,30) );
    foreach(TvProgram prog, m_TvProgramsList)
    {
        if ( prog.start <= evening && evening < prog.stop )
        {
            foreach(TvChannel channel, m_TvChannelsList)
            {
                if ( channel.id == prog.channel )
                {
                    prog.channelName = channel.name;
                    break;
                }
            }
            programsList.append( prog );
        }
    }
    return sortedPrograms( programsList );
}


QList<TvProgram>  XmlDefaultHandler::programsMaintenant()
{
    QList<TvProgram> programsList;
    QDateTime now = QDateTime::currentDateTime();
    foreach(TvProgram prog, m_TvProgramsList)
    {
        if ( prog.start <= now && now < prog.stop )
        {
            foreach(TvChannel channel, m_TvChannelsList)
            {
                if ( channel.id == prog.channel )
                {
                    prog.channelName = channel.name;
                    break;
                }
            }
            programsList.append( prog );
        }
    }
    return sortedPrograms( programsList );
}
bool XmlDefaultHandler::readFromDB()
{
    clearView();
    connectDB();
    //m_query.exec("BEGIN TRANSACTION;");
    QString queryString;
    bool rc;
    QVariant v;
    //
    queryString = "select * from channels";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return false;
    }
    if ( !m_query.next() )
        return false;
    // Case vide en up/gauche
    GraphicsRectItem *item = new GraphicsRectItem(m_main,
                             QRectF(0, 0, 100, m_hourHeight),
                             "",
                             GraphicsRectItem::Channel);
    item->setZValue(10);
    v.setValue( TvProgram() );
    item->setData(0, v );
    m_programsView->scene()->addItem( item );
    m_listeItemChaines.insert(0, item );
    QStringList ids;
    do
    {
        TvChannel channel;
        channel.id = m_query.value(0).toString().replace("$", "'");
        channel.name = m_query.value(1).toString().replace("$", "'");
        channel.icon = m_query.value(2).toString().replace("$", "'");
        channel.enabled = m_query.value(3).toString().replace("$", "'").toInt();
        m_TvChannelsList << channel;
    }
    while ( m_query.next() );
    m_TvChannelsList = sortedChannels();
    // Insertion des channels triees dans la scene
    int line = 0;
    foreach(TvChannel channel, m_TvChannelsList)
    {
        if ( channel.enabled )
        {
            ids << channel.id;
            GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                     QRectF(0, m_hourHeight+(line*m_progHeight), 100, m_progHeight),
                                     channel.name,
                                     GraphicsRectItem::Channel,
                                     PairIcon(	":/images/images/"+channel.icon.section("/",-1,-1).section(".",0,0)+".png",
                                               QPixmap(":/images/images/"+channel.icon.section("/",-1,-1).section(".",0,0)+".png" )
                                             )
                                                         );
            item->setZValue(17);
            m_programsView->scene()->addItem( item );
            m_listeItemChaines.append( item );
            line++;
        }
    }
    // Dimensionnement de la scene en fonction du nombre de demi-heure (largeur) et de channels (upeur).
    m_programsView->setSceneRect(
        m_programsView->rect().x(),
        m_programsView->rect().y(),
        100+((48-(m_hourBeginning*2))*m_progWidth),
        m_hourHeight+(line*m_progHeight)
    );
    //
    m_programsView->setBackgroundBrush(QColor(Qt::red).light(188));
    // Ligne de l'heure courante
    m_currentTimeLine = new QGraphicsLineItem();
    m_currentTimeLine->setPen(QPen(QColor(Qt::red), 2, Qt::DashDotLine));
    if ( QDate::currentDate() != m_date )
        m_currentTimeLine->hide();
    m_programsView->scene()->addItem( m_currentTimeLine );
    v.setValue( TvProgram() );
    m_currentTimeLine->setData(0, v );
    m_currentTimeLine->setZValue(16);
    // Creation de la colonne des channels
    // Cadre jaune des heures
    GraphicsRectItem *hoursRect = new GraphicsRectItem(m_main,
                                  QRectF(0, 0, (49-(m_hourBeginning*2))*m_progWidth, m_hourHeight),
                                  "",
                                  GraphicsRectItem::HourRect);
    m_programsView->scene()->addItem( hoursRect );
    hoursRect->setZValue(20);
    v.setValue( TvProgram() );
    hoursRect->setData(0, v );
    m_listeItemHeures.append( hoursRect );
    //
    QTime time(m_hourBeginning, 0);
    for (int i=0; i<48-(m_hourBeginning*2); i++)
    {
        // Ligne pointillee pour chaque demi-heure
        QGraphicsLineItem *line = new QGraphicsLineItem(100+(i*m_progWidth), m_hourHeight, 100+(i*m_progWidth) ,m_hourHeight+(m_TvChannelsList.count()*m_progHeight));
        line->setPen(QPen(QColor(Qt::blue).light(), 1, Qt::DashDotLine));
        QVariant v;
        v.setValue( TvProgram() );
        line->setData(0, v );
        m_programsView->scene()->addItem( line );
        // Libelle de chacune des demi-heure
        GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                 QRectF(100+((i-1)*m_progWidth),0, m_progWidth*2, m_hourHeight),
                                 time.toString("hh:mm"),
                                 GraphicsRectItem::Hour);
        m_programsView->scene()->addItem( item );
        item->setZValue(21);
        v.setValue( TvProgram() );
        item->setData(0, v );
        m_listeItemHeures.append( item );
        time = time.addSecs(1800);
    }
    // Programmes
    // Reading dans la base de donnees des programs du jour choisi dans l'interface (variable m_date)
    // ainsi que du jour courant pour renseigner la fenetre "Maintenant"
    queryString = "select * from programs where "

                  + QString(" (start >= '") + QString::number(QDateTime(m_date).toTime_t())
                  + "' and start <= '" + QString::number(QDateTime(m_date).addDays(1).addSecs(-60).toTime_t()) + "')"

                  + " OR (start >= '" + QString::number(QDateTime(m_date).addDays(-1).toTime_t())
                  + "' and start < '" + QString::number(QDateTime(m_date).toTime_t())
                  + "' and stop > '" + QString::number(QDateTime(m_date).toTime_t()) + "')"
                  // Le jour courant
                  + " OR (start <= '" + QString::number(QDateTime::currentDateTime().toTime_t())
                  + "' and '" + QString::number(QDateTime::currentDateTime().toTime_t())+ "' < stop)";

    rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return false;
    }
    if ( !m_query.next() )
        return false;
    do
    {
        TvProgram prog;
        prog.start = QDateTime::fromTime_t( m_query.value(0).toInt() );
        prog.stop = QDateTime::fromTime_t( m_query.value(1).toInt() );
        prog.channel = m_query.value(2).toString().replace("$", "'");
        prog.channelName = m_query.value(3).toString().replace("$", "'");
        prog.title = m_query.value(4).toString().replace("$", "'");
        prog.subTitle = m_query.value(5).toString().replace("$", "'");
        prog.category = m_query.value(6).toString().replace("$", "'").split("|");
        prog.resume = m_query.value(7).toString().replace("$", "'").split("|");
        prog.story = m_query.value(8).toString().replace("$", "'");
        prog.aspect = m_query.value(9).toString().replace("$", "'");
        prog.credits = m_query.value(10).toString().replace("$", "'");
        prog.director = m_query.value(11).toString().replace("$", "'");
        prog.star = m_query.value(12).toString().replace("$", "'");
        prog.icon = m_query.value(13).toString().replace("$", "'");
        m_TvProgramsList.append( prog );
        if ( prog.start.date() == m_date || prog.stop.date() == m_date)
        {
            int line = ids.indexOf(prog.channel);
            double x = QDateTime(m_date).secsTo( prog.start )*(m_progWidth/1800.0);
            x = x - ((m_hourBeginning*2)*m_progWidth);
            double w =  prog.start.secsTo( prog.stop )*(m_progWidth/1800.0);
            GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                     QRectF(100+x,m_hourHeight+(line*m_progHeight),w,m_progHeight),
                                     prog.title,
                                     GraphicsRectItem::Program,
                                     pairIcon( prog.icon ),
                                     prog.star.section("/", 0, 0).toInt()
                                                         );
            QObject::connect(
                m_getImages,
                SIGNAL(imageAvailable(PairIcon)),
                item,
                SLOT(slotImageAvailable(PairIcon))
            );
            item->setZValue(15);
            QVariant v;
            v.setValue( prog );
            item->setData(0, v );
            m_programsView->scene()->addItem( item );
            m_programsItemsList.append( item );
        }
    }
    while ( m_query.next() );
    //m_query.exec("END TRANSACTION;");
    //
    queryString = "select * from images where ok='0'";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return false;
    }
    while ( m_query.next() )
    {
        m_imagesList << m_query.value(0).toString().replace("$", "'");
    }
    if ( !m_main->proxyAddress().isEmpty() )
        m_getImages->setList( m_imagesList, m_query, m_main->proxyAddress(), m_main->proxyPort() );
    else
        m_getImages->setList( m_imagesList, m_query );
    listProgrammesSortedByTime();
    nowCenter();
    return true;
}

bool XmlDefaultHandler::connectDB()
{
    QString dbName = m_main->iniPath() + "qmagneto.db";
    QSqlDatabase database;
    if ( QSqlDatabase::database(dbName).databaseName() != dbName )
    {
        database = QSqlDatabase::addDatabase("QSQLITE", dbName);
        database.setDatabaseName(dbName);
    }
    else
    {
        database = QSqlDatabase::database(dbName);
        if ( database.isOpen() )
            return true;
    }
    //
    if (!database.open())
    {
        QMessageBox::critical(0, "QMagneto",
                              QObject::tr("Unable to establish a database connection.")+"\n"+
                              QObject::tr("QMagneto needs SQLite support. Please read "
                                          "the Qt SQL driver documentation for information how "
                                          "to build it."), QMessageBox::Cancel,
                              QMessageBox::NoButton);
        return false;
    }
    m_query = QSqlQuery(database);
    return true;
}


void XmlDefaultHandler::imageToTmp(QString icon, bool isChannel)
{
    connectDB();
    m_getImages->imageToTmp(icon, m_query, isChannel);
}


PairIcon XmlDefaultHandler::pairIcon(QString icon)
{
    return m_getImages->pairIcon(icon, m_query);
}


void XmlDefaultHandler::nowCenter()
{
    double x = QTime(0,0).secsTo( QTime::currentTime() )*(m_progWidth/1800.0);
    x = 100+x-((m_hourBeginning*2)*m_progWidth);
    m_programsView->centerOn(x ,0);
}


QDate XmlDefaultHandler::minimumDate()
{
    QString queryString = "select min(start) from programs";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return QDate();
    }
    if ( !m_query.next() )
        return QDate();
    return QDateTime::fromTime_t( m_query.value(0).toInt() ).date();
}


QDate XmlDefaultHandler::maximumDate()
{
    QString queryString = "select max(stop) from programs";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return QDate();
    }
    if ( !m_query.next() )
        return QDate();
    return QDateTime::fromTime_t( m_query.value(0).toInt() ).date();
}


QList<TvChannel> XmlDefaultHandler::sortedChannels()
{
    // On tri les channels par numero de id
    QList<TvChannel> sortedList;
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    int i=0;
    do
    {
        QString id = settings.value("pos"+QString::number(i++)).toString();
        int n = 0;
        int index = 0;
        foreach( TvChannel chaineTV, m_TvChannelsList)
        {
            if ( chaineTV.id == id )
            {
                index = n;
                break;
            }
            n++;
        }
        sortedList.append(m_TvChannelsList.at(index));
        m_TvChannelsList.removeAt(index);
    }
    while ( m_TvChannelsList.count() );
    // Maintenant les channels non presentes dans le fichier ini
    foreach( TvChannel chaineTV, m_TvChannelsList)
    {
        sortedList.append(chaineTV);
    }
    m_TvChannelsList = sortedList;
    settings.endGroup();
    return sortedList;
}


QList<TvProgram> XmlDefaultHandler::sortedPrograms(QList<TvProgram> list)
{
    // On tri les programs par numero de id de la channel
    if ( !list.count() )
        return QList<TvProgram>();
    QList<TvProgram> sortedList;
    QSettings settings(MainWindowImpl::iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    int i=0;
    do
    {
        QString channel = settings.value("pos"+QString::number(i++)).toString();
        int n = 0;
        int index = 0;
        foreach( TvProgram programTV, list)
        {
            if ( programTV.channel == channel )
            {
                index = n;
                break;
            }
            n++;
        }
        sortedList.append(list.at(index));
        list.removeAt(index);
    }
    while ( list.count() );
    // Maintenant les channels non presentes dans le fichier ini
    foreach( TvProgram programTV, list)
    {
        sortedList.append(programTV);
    }
    settings.endGroup();
    return sortedList;
}


void XmlDefaultHandler::clearView()
{
    m_listeItemChaines.clear();
    m_listeItemHeures.clear();
    m_programsItemsList.clear();
    // Suppression de tous les elements dans la vue
    QList<QGraphicsItem *> list = m_programsView->scene()->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        if ( *it )
        {
            m_programsView->scene()->removeItem(*it);
            delete *it;
        }
    }
}


GraphicsRectItem * XmlDefaultHandler::findProgramme(QString text, bool backward, bool fromBegin, bool sensitive, bool wholeWord)
{
    if( wholeWord )
      text = "\\b("+text+")\\b";
    QRegExp regExp( text );
    regExp.setCaseSensitivity( ( Qt::CaseSensitivity) sensitive );
    QListIterator<GraphicsRectItem *> iterator(m_programsSortedItemsList);
    GraphicsRectItem *item = 0;
    bool found = false;
    if ( !fromBegin )
    {
        if ( backward )
        {
            iterator.toBack();
            while (iterator.hasPrevious())
            {
                item = iterator.previous();
                if ( item->isEnabled() )
                {
                    TvProgram prog = item->data(0).value<TvProgram>();
                    found = true;
                    break;
                }
            }
        }
        else
        {
            iterator.toFront();
            while (iterator.hasNext())
            {
                item = iterator.next();
                if ( item->isEnabled() )
                {
                    TvProgram prog = item->data(0).value<TvProgram>();
                    found = true;
                    break;
                }
            }
        }
    }
    if ( backward )
    {
        if ( !found )
        {
            iterator.toBack();
        }
        if ( item )
        {
            TvProgram prog = item->data(0).value<TvProgram>();
        }
        while (iterator.hasPrevious())
        {
            item = iterator.previous();
            TvProgram prog = item->data(0).value<TvProgram>();
            if ( prog.title.contains(regExp) && prog.stop.time() >= QTime(m_hourBeginning, 0) )
            {
                double x = QTime(0,0).secsTo( prog.start.time() )*(m_progWidth/1800.0);
                x = 100+x-((m_hourBeginning*2)*m_progWidth);
                QStringList ids;
                foreach(TvChannel channel, m_TvChannelsList)
                {
                    if ( channel.enabled )
                    {
                        ids << channel.id;
                    }
                }
                int line = ids.indexOf(prog.channel);
                double y = m_hourHeight+(line*m_progHeight);
                m_programsView->centerOn(x ,y);
                return item;
            }
        }
    }
    else
    {
        if ( !found )
        {
            iterator.toFront();
        }
        while (iterator.hasNext())
        {
            item = iterator.next();
            TvProgram prog = item->data(0).value<TvProgram>();
            if ( prog.title.contains(regExp) && prog.stop.time() >= QTime(m_hourBeginning, 0) )
            {
                double x = QTime(0,0).secsTo( prog.start.time() )*(m_progWidth/1800.0);
                x = 100+x-((m_hourBeginning*2)*m_progWidth);
                QStringList ids;
                foreach(TvChannel channel, m_TvChannelsList)
                {
                    if ( channel.enabled )
                    {
                        ids << channel.id;
                    }
                }
                int line = ids.indexOf(prog.channel);
                double y = m_hourHeight+(line*m_progHeight);
                m_programsView->centerOn(x ,y);
                return item;
            }
        }
    }
    return 0;
}

void XmlDefaultHandler::listProgrammesSortedByTime()
{
    QString queryString = "select * from programs where "

                          + QString(" (start >= '") + QString::number(QDateTime(m_date).toTime_t())
                          + "' and start <= '" + QString::number(QDateTime(m_date).addDays(1).addSecs(-60).toTime_t()) + "')"

                          + " OR (start >= '" + QString::number(QDateTime(m_date).addDays(-1).toTime_t())
                          + "' and start < '" + QString::number(QDateTime(m_date).toTime_t())
                          + "' and stop > '" + QString::number(QDateTime(m_date).toTime_t()) + "')";
    // Le jour courant
    //+ " OR (start <= '" + QString::number(QDateTime::currentDateTime().toTime_t())
    //+ "' and '" + QString::number(QDateTime::currentDateTime().toTime_t())+ "' < stop)";

    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
    }
    if ( !m_query.next() )
        return;
    do
    {
        TvProgram prog;
        prog.start = QDateTime::fromTime_t( m_query.value(0).toInt() );
        prog.stop = QDateTime::fromTime_t( m_query.value(1).toInt() );
        prog.channel = m_query.value(2).toString().replace("$", "'");
        foreach(GraphicsRectItem *item, m_programsItemsList)
        {
            TvProgram p = item->data(0).value<TvProgram>();
            if ( prog.start == p.start && prog.stop == p.stop && prog.channel == p.channel )
            {
                m_programsSortedItemsList.append(item);
            }
        }
    }
    while ( m_query.next() );
}
