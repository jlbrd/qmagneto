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
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
float largeurProg = 180.0;
float hauteurProg = 60.0;
float hauteurHeure = 25.0;
QGraphicsView *viewP;

XmlDefaultHandler::XmlDefaultHandler(MainWindowImpl *main, QGraphicsView *programmes)
        : QXmlDefaultHandler(), m_main(main), m_viewProgrammes(programmes)
{
    m_recupImages = new RecupImages( m_listeImages, m_query);
    viewP = programmes;
}
//
XmlDefaultHandler::~XmlDefaultHandler()
{
    QD << "delete";
    if ( m_recupImages )
        delete m_recupImages;
}
//
bool XmlDefaultHandler::startElement( const QString & , const QString & , const QString & qName, const QXmlAttributes & atts )
{
    static bool isChannel = false;
    Balise balisePrecedente = m_balise;
    m_balise = Rien;
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
            m_programmeTV.icon = atts.value(0);
        m_listeImages.append( atts.value(0) );
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
            m_balise = Rien;
    }
    else if ( qName == "programme" )
    {
        isChannel = false;
        for (int i=0; i< atts.count(); i++)
        {
            if ( atts.qName(i) == "start" )
            {
                m_programmeTV.start = QDateTime::fromString(atts.value(i).section(" ",0,0), "yyyyMMddhhmmss");
            }
            else if ( atts.qName(i) == "stop" )
            {
                m_programmeTV.stop = QDateTime::fromString(atts.value(i).section(" ",0,0), "yyyyMMddhhmmss");
            }
            else if ( atts.qName(i) == "channel" )
            {
                m_programmeTV.channel = atts.value(i);
            }
        }
    }
    else
    {
        m_balise = Rien;
        //QD << qName;
    }
    return true;
}

bool XmlDefaultHandler::endElement( const QString & , const QString & , const QString & qName )
{
    QSettings settings(MainWindowImpl::cheminIni() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    if ( qName == "channel" )
    {
        if ( (Qt::CheckState)settings.value(m_chaineTV.id+"-isEnabled", Qt::Checked).toInt() != Qt::Checked )
        {
            QD << "Chaine " << m_chaineTV.name << "desactivee dans le dialogue Canaux";
            m_chaineTV.enabled = false;
        }
        else
            m_chaineTV.enabled = true;
        QString queryString = "insert into chaines values(";
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
        m_listeChainesTV << m_chaineTV;
        m_chaineTV = ChaineTV();
    }
    else if ( qName == "programme" )
    {
        if ( (Qt::CheckState)settings.value(m_programmeTV.channel+"-isEnabled", Qt::Checked).toInt() != Qt::Checked )
        {
            //QD << "Chaine non active : " << m_programmeTV.channel;
            return true;
        }
        foreach(ChaineTV chaine, m_listeChainesTV)
        {
            if ( chaine.id == m_programmeTV.channel )
            {
                m_programmeTV.channelName = chaine.name;
                break;
            }
        }
        QString queryString = "insert into programmes values(";
        queryString = queryString
                      + "'" + QString::number( m_programmeTV.start.toTime_t() ) + "', "
                      + "'" + QString::number( m_programmeTV.stop.toTime_t() ) + "', "
                      + "'" + m_programmeTV.channel.replace("'", "$") + "', "
                      + "'" + m_programmeTV.channelName.replace("'", "$") + "', "
                      + "'" + m_programmeTV.title.replace("'", "$") + "', "
                      + "'" + m_programmeTV.subTitle.replace("'", "$") + "', "
                      + "'" + m_programmeTV.category.join("|").replace("'", "$") + "', "
                      + "'" + m_programmeTV.resume.join("|").replace("'", "$") + "', "
                      + "'" + m_programmeTV.histoire.replace("'", "$") + "', "
                      + "'" + m_programmeTV.aspect.replace("'", "$") + "', "
                      + "'" + m_programmeTV.credits.replace("'", "$") + "', "
                      + "'" + m_programmeTV.director.replace("'", "$") + "', "
                      + "'" + m_programmeTV.star.replace("'", "$") + "', "
                      + "'" + m_programmeTV.icon.replace("'", "$")
                      +  "')";
        bool rc = m_query.exec(queryString);
        if (rc == false)
        {
            qDebug() << "Failed to insert record to db" << m_query.lastError();
            qDebug() << queryString;
        }
        if ( !m_programmeTV.icon.isEmpty() )
        {
            QByteArray data;
            QVariant clob(data);
            m_query.prepare("INSERT INTO images (icon, ok, data)"
                            "VALUES (:icon, :ok, :data)");
            m_query.bindValue(":icon", m_programmeTV.icon.replace("'", "$"));
            m_query.bindValue(":ok","0");
            m_query.bindValue(":data", clob);
            bool rc = m_query.exec();
            if (rc == false)
            {
                qDebug() << "Failed to insert record to db" << m_query.lastError();
                qDebug() << queryString;
            }
        }
        m_programmeTV = ProgrammeTV();
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
        m_programmeTV.title = ch;
        break;
    case SubTitle:
        m_programmeTV.subTitle = ch;
        break;
    case Desc:
        if ( !m_programmeTV.resume.count() )
            m_programmeTV.resume << ch;
        else
            m_programmeTV.histoire = ch;
        break;
    case Aspect:
        m_programmeTV.aspect = ch;
        break;
    case Category:
        m_programmeTV.category << ch;
        break;
    case DisplayName:
        m_chaineTV.name = ch;
        break;
    case Star:
        m_programmeTV.star = ch;
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
    //m_recupImages->setListe( m_listeImages, m_query );
    return true;
}

void XmlDefaultHandler::deplaceChaines(int )
{
    foreach(GraphicsRectItem *item, m_listeItemChaines)
    {
        item->setPos(m_viewProgrammes->horizontalScrollBar()->value(), item->y());
    }
}


void XmlDefaultHandler::deplaceHeures(int )
{
    foreach(GraphicsRectItem *item, m_listeItemHeures)
    {
        item->setPos(item->x(), m_viewProgrammes->verticalScrollBar()->value());
    }
}


bool XmlDefaultHandler::startDocument()
{
//ifndef Q_OS_WIN32
//    QSqlDatabase::removeDatabase( m_main->cheminIni() + "qmagneto.db");
//    QFile::remove( m_main->cheminIni() + "qmagneto.db" );
//#endif
    connectDB();
    m_query.exec("BEGIN TRANSACTION;");
//#ifdef Q_OS_WIN32
    foreach(QString table, QStringList() << "chaines" << "programmes" << "images")
    {
        QString queryString = "delete from " + table + ";";
        bool rc = m_query.exec(queryString);
        if (rc == false)
        {
            qDebug() << "Failed to delete table" << m_query.lastError();
            qDebug() << queryString;
        }
    }
//#endif
    return true;
}


void XmlDefaultHandler::init()
{
    m_listeChainesTV.clear();
    m_listeProgrammesTV.clear();
    m_listeImages.clear();
    //m_recupImages = 0;
}


void XmlDefaultHandler::posLigneHeureCourante()
{
    double x = QTime(0,0).secsTo( QTime::currentTime() )*(largeurProg/1800.0);
    m_ligneHeureCourante->setLine
    (
        100+x-((m_heureDebutJournee*2)*largeurProg),
        hauteurHeure,
        100+x-((m_heureDebutJournee*2)*largeurProg) ,
        hauteurHeure+(m_listeChainesTV.count()*hauteurProg)
    );
    foreach(GraphicsRectItem *item, m_listeItemProgrammes)
    {
        ProgrammeTV prog = item->data(0).value<ProgrammeTV>();
        if ( prog.start <= QDateTime::currentDateTime() && QDateTime::currentDateTime() <= prog.stop )
        {
            item->setDansHeureCourante(true);
        }
        else
        {
            item->setDansHeureCourante(false);
        }
        item->update();
    }
}


void XmlDefaultHandler::soiree()
{
    double x = QTime(0,0).secsTo( QTime(21, 30) )*(largeurProg/1800.0);
    x = 100+x-((m_heureDebutJournee*2)*largeurProg);
    m_viewProgrammes->centerOn(x ,0);
}


QList<ProgrammeTV> XmlDefaultHandler::programmesSoiree()
{
    QList<ProgrammeTV> listeProgrammes;
    QDateTime soiree = QDateTime( QDate(m_date), QTime(21,30) );
    foreach(ProgrammeTV prog, m_listeProgrammesTV)
    {
        if ( prog.start <= soiree && soiree < prog.stop )
        {
            foreach(ChaineTV chaine, m_listeChainesTV)
            {
                if ( chaine.id == prog.channel )
                {
                    prog.channelName = chaine.name;
                    break;
                }
            }
            listeProgrammes.append( prog );
        }
    }
    // On tri les chaines par le numero d'id des chaines
    QList<ProgrammeTV> listeTriee;
    while ( listeProgrammes.count() )
    {
        int channel = 99999;
        int index = -1;
        for (int i=0; i<listeProgrammes.count(); i++)
        {
            if ( listeProgrammes.at(i).channel.section(".", 0, 0).section('C', 1, 1).toInt() < channel )
            {
                channel = listeProgrammes.at(i).channel.section(".", 0, 0).section('C', 1, 1).toInt();
                index = i;
            }
        }
        listeTriee.append(listeProgrammes.at(index));
        listeProgrammes.removeAt(index);
    };
    return listeTriee;
}


QList<ProgrammeTV>  XmlDefaultHandler::programmesMaintenant()
{
    QList<ProgrammeTV> listeProgrammes;
    QDateTime maintenant = QDateTime::currentDateTime();
    foreach(ProgrammeTV prog, m_listeProgrammesTV)
    {
        if ( prog.start <= maintenant && maintenant < prog.stop )
        {
            foreach(ChaineTV chaine, m_listeChainesTV)
            {
                if ( chaine.id == prog.channel )
                {
                    prog.channelName = chaine.name;
                    break;
                }
            }
            listeProgrammes.append( prog );
        }
    }
    // On tri les chaines par le numero d'id des chaines
    QList<ProgrammeTV> listeTriee;
    while ( listeProgrammes.count() )
    {
        int channel = 99999;
        int index = 0;
        for (int i=0; i<listeProgrammes.count(); i++)
        {
            if ( listeProgrammes.at(i).channel.section(".", 0, 0).section('C', 1, 1).toInt() < channel )
            {
                channel = listeProgrammes.at(i).channel.section(".", 0, 0).section('C', 1, 1).toInt();
                index = i;
            }
        }
        listeTriee.append(listeProgrammes.at(index));
        listeProgrammes.removeAt(index);
    }
    return listeTriee;
}
bool XmlDefaultHandler::readFromDB()
{
    m_listeItemChaines.clear();
    m_listeItemHeures.clear();
    m_listeItemProgrammes.clear();
    // Suppression de tous les elements dans la vue
    QList<QGraphicsItem *> list = m_viewProgrammes->scene()->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        if ( *it )
        {
            m_viewProgrammes->scene()->removeItem(*it);
            delete *it;
        }
    }
    //QD << m_viewProgrammes->scene()->items().count();
    connectDB();
    m_query.exec("BEGIN TRANSACTION;");
    QString queryString;
    bool rc;
    QVariant v;
    //
    queryString = "select * from chaines";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return false;
    }
    if ( !m_query.next() )
        return false;
    // Case vide en haut/gauche
    GraphicsRectItem *item = new GraphicsRectItem(m_main,
                             QRectF(0, 0, 100, hauteurHeure),
                             "",
                             GraphicsRectItem::Chaine);
    item->setZValue(10);
    v.setValue( ProgrammeTV() );
    item->setData(0, v );
    m_viewProgrammes->scene()->addItem( item );
    m_listeItemChaines.insert(0, item );
    QStringList ids;
    do
    {
        ChaineTV chaine;
        chaine.id = m_query.value(0).toString().replace("$", "'");
        chaine.name = m_query.value(1).toString().replace("$", "'");
        chaine.icon = m_query.value(2).toString().replace("$", "'");
        chaine.enabled = m_query.value(3).toString().replace("$", "'").toInt();
        m_listeChainesTV << chaine;
    }
    while ( m_query.next() );
    // On tri les chaines par numero de id
    QSettings settings(MainWindowImpl::cheminIni() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    int i=0;
    QList<ChaineTV> listeTriee;
    do
    {
        QString id = settings.value("pos"+QString::number(i++)).toString();
        int n = 0;
        int index = 0;
        foreach( ChaineTV chaineTV, m_listeChainesTV)
        {
        	if( chaineTV.id == id )
        	{
        		index = n;
        		break;
       		}
       		n++;
       	}
        listeTriee.append(m_listeChainesTV.at(index));
        m_listeChainesTV.removeAt(index);
    }
    while ( m_listeChainesTV.count() );
    // Maintenant les chaines non presentes dans le fichier ini
    foreach( ChaineTV chaineTV, m_listeChainesTV)
    {
    	listeTriee.append(chaineTV);
   	}
    m_listeChainesTV = listeTriee;
    settings.endGroup();
    // Insertion des chaines triees dans la scene
    int ligne = 0;
    foreach(ChaineTV chaine, m_listeChainesTV)
    {
        if ( chaine.enabled )
        {
            ids << chaine.id;
            GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                     QRectF(0, hauteurHeure+(ligne*hauteurProg), 100, hauteurProg),
                                     chaine.name,
                                     GraphicsRectItem::Chaine,
                                     QPixmap(":/images/images/"+chaine.name+".png" )
                                                         );
            item->setZValue(17);
            m_viewProgrammes->scene()->addItem( item );
            m_listeItemChaines.append( item );
            ligne++;
        }
    }
    // Dimensionnement de la scene en fonction du nombre de demi-heure (largeur) et de chaines (hauteur).
    m_viewProgrammes->setSceneRect(
        m_viewProgrammes->rect().x(),
        m_viewProgrammes->rect().y(),
        100+((48-(m_heureDebutJournee*2))*largeurProg),
        hauteurHeure+(ligne*hauteurProg)
    );
    //
    m_viewProgrammes->setBackgroundBrush(QColor(Qt::red).light(188));
    // Ligne de l'heure courante
    m_ligneHeureCourante = new QGraphicsLineItem();
    m_ligneHeureCourante->setPen(QPen(QColor(Qt::red), 2, Qt::DashDotLine));
    if ( QDate::currentDate() != m_date )
        m_ligneHeureCourante->hide();
    m_viewProgrammes->scene()->addItem( m_ligneHeureCourante );
    v.setValue( ProgrammeTV() );
    m_ligneHeureCourante->setData(0, v );
    m_ligneHeureCourante->setZValue(16);
    // Creation de la colonne des chaines
    // Cadre jaune des heures
    GraphicsRectItem *cadreHeures = new GraphicsRectItem(m_main,
                                    QRectF(0, 0, (49-(m_heureDebutJournee*2))*largeurProg, hauteurHeure),
                                    "",
                                    GraphicsRectItem::CadreHeure);
    m_viewProgrammes->scene()->addItem( cadreHeures );
    cadreHeures->setZValue(20);
    v.setValue( ProgrammeTV() );
    cadreHeures->setData(0, v );
    m_listeItemHeures.append( cadreHeures );
    //
    QTime time(m_heureDebutJournee, 0);
    for (int i=0; i<48-(m_heureDebutJournee*2); i++)
    {
        // Ligne pointillee pour chaque demi-heure
        QGraphicsLineItem *ligne = new QGraphicsLineItem(100+(i*largeurProg), hauteurHeure, 100+(i*largeurProg) ,hauteurHeure+(m_listeChainesTV.count()*hauteurProg));
        ligne->setPen(QPen(QColor(Qt::blue).light(), 1, Qt::DashDotLine));
        QVariant v;
        v.setValue( ProgrammeTV() );
        ligne->setData(0, v );
        m_viewProgrammes->scene()->addItem( ligne );
        // Libelle de chacune des demi-heure
        GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                 QRectF(100+((i-1)*largeurProg),0, largeurProg*2, hauteurHeure),
                                 time.toString("hh:mm"),
                                 GraphicsRectItem::Heure);
        m_viewProgrammes->scene()->addItem( item );
        item->setZValue(21);
        v.setValue( ProgrammeTV() );
        item->setData(0, v );
        m_listeItemHeures.append( item );
        time = time.addSecs(1800);
    }
    // Programmes
    // Lecture dans la base de donnees ds programmes selectionnee presente dans m_date
    // ainsi que les programmes du jour pour renseigner la fenetre "Maintenant"
    queryString = "select * from programmes where "

                  + QString(" (start >= '") + QString::number(QDateTime(m_date).toTime_t())
                  + "' and start <= '" + QString::number(QDateTime(m_date).addDays(1).addSecs(-60).toTime_t()) + "')"

                  + " OR (start >= '" + QString::number(QDateTime(m_date).addDays(-1).toTime_t())
                  + "' and start < '" + QString::number(QDateTime(m_date).toTime_t())
                  + "' and stop > '" + QString::number(QDateTime(m_date).toTime_t()) + "')"
                  // Pour du jour courant
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
        ProgrammeTV prog;
        prog.start = QDateTime::fromTime_t( m_query.value(0).toInt() );
        prog.stop = QDateTime::fromTime_t( m_query.value(1).toInt() );
        prog.channel = m_query.value(2).toString().replace("$", "'");
        prog.channelName = m_query.value(3).toString().replace("$", "'");
        prog.title = m_query.value(4).toString().replace("$", "'");
        prog.subTitle = m_query.value(5).toString().replace("$", "'");
        prog.category = m_query.value(6).toString().replace("$", "'").split("|");
        prog.resume = m_query.value(7).toString().replace("$", "'").split("|");
        prog.histoire = m_query.value(8).toString().replace("$", "'");
        prog.aspect = m_query.value(9).toString().replace("$", "'");
        prog.credits = m_query.value(10).toString().replace("$", "'");
        prog.director = m_query.value(11).toString().replace("$", "'");
        prog.star = m_query.value(12).toString().replace("$", "'");
        prog.icon = m_query.value(13).toString().replace("$", "'");
        m_listeProgrammesTV.append( prog );
        if ( prog.start.date() == m_date || prog.stop.date() == m_date)
        {
            int ligne = ids.indexOf(prog.channel);
            double x = QDateTime(m_date).secsTo( prog.start )*(largeurProg/1800.0);
            //double x = QTime(0,0).secsTo( prog.start.time() )*(largeurProg/1800.0);
            x = x - ((m_heureDebutJournee*2)*largeurProg);
            double w =  prog.start.secsTo( prog.stop )*(largeurProg/1800.0);
            //QD << prog.start << prog.stop << prog.title << x << QDateTime(m_date) << QDateTime(m_date).secsTo( prog.start );
            GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                     QRectF(100+x,hauteurHeure+(ligne*hauteurProg),w,hauteurProg),
                                     prog.title,
                                     GraphicsRectItem::Programme,
                                     pixmap( prog.icon ),
                                     prog.star.section("/", 0, 0).toInt()
                                                         );
            item->setZValue(15);
            QVariant v;
            v.setValue( prog );
            item->setData(0, v );
            m_viewProgrammes->scene()->addItem( item );
            m_listeItemProgrammes.append( item );
        }
        //QD << prog.channel << ligne << x << w << prog.title << prog.start << prog.stop;
    }
    while ( m_query.next() );
    m_query.exec("END TRANSACTION;");
    //
    queryString = "select * from images where ok=\"0\"";
    rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return false;
    }
    while ( m_query.next() )
    {
        m_listeImages << m_query.value(0).toString().replace("$", "'");
    }
    m_recupImages->setListe( m_listeImages, m_query );
    centreMaintenant();
    return true;
}

bool XmlDefaultHandler::connectDB()
{
    QString dbName = m_main->cheminIni() + "qmagneto.db";
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
    else
    {
        QString queryString = "create table chaines ("
                              "id string,"
                              "name string,"
                              "icon string,"
                              "enabled string"
                              ")";

        m_query = QSqlQuery(database);
        m_query.exec(queryString);
        //
        queryString = "create table programmes ("
                      "start int,"
                      "stop int,"
                      "channel string,"
                      "channelName string,"
                      "title string,"
                      "subTitle string,"
                      "category string,"
                      "resume string,"
                      "histoire string,"
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
    }
    return true;
}


void XmlDefaultHandler::imageToTmp(QString icon, bool isChaine)
{
    connectDB();
    m_recupImages->imageToTmp(icon, m_query, isChaine);
}


QPixmap XmlDefaultHandler::pixmap(QString icon)
{
    return m_recupImages->pixmap(icon, m_query);
}


void XmlDefaultHandler::centreMaintenant()
{
    double x = QTime(0,0).secsTo( QTime::currentTime() )*(largeurProg/1800.0);
    x = 100+x-((m_heureDebutJournee*2)*largeurProg);
    m_viewProgrammes->centerOn(x ,0);
}


QDate XmlDefaultHandler::minimumDate()
{
    QString queryString = "select min(start) from programmes";
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
    QString queryString = "select max(stop) from programmes";
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

