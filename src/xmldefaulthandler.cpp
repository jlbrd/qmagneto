#include "xmldefaulthandler.h"
#include "graphicsrectitem.h"
#include "mainwindowimpl.h"
//
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScrollBar>
#include <QTextCodec>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
float largeurProg = 180.0;
float hauteurProg = 60.0;
float hauteurHeure = 25.0;

//QStringList m_categories;
XmlDefaultHandler::XmlDefaultHandler(MainWindowImpl *main, QGraphicsView *programmes)
        : QXmlDefaultHandler(), m_main(main), m_viewProgrammes(programmes)
{
    //QD << chaines->sceneRect();
    //m_listeChainesTV.clear();
}
//

bool XmlDefaultHandler::startElement( const QString & , const QString & , const QString & qName, const QXmlAttributes & atts )
{
	Balise balisePrecedente = m_balise;
    m_balise = Rien;
    if ( qName == "channel" )
    {
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
        //m_chaineTV.name = atts.value(0);
        m_balise = DisplayName;
    }
    else if ( qName == "icon" )
    {
        m_chaineTV.icon = atts.value(0);
    }
    else if ( qName == "title" )
    {
        m_balise = Title;
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
    	if( balisePrecedente == Star )
    		m_balise = Star;
    	else
    		m_balise = Rien;
    }
    else if ( qName == "programme" )
    {
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
    if ( qName == "channel" )
    {
        m_listeChainesTV.append( m_chaineTV );
        m_chaineTV = ChaineTV();
    }
    else if ( qName == "programme" )
    {
        if ( m_programmeTV.start.date() != m_date )
        {
            //QD << m_programmeTV.start << m_programmeTV.title;
        }
        else
        {
            m_listeProgrammesTV.append( m_programmeTV );
            m_programmeTV = ProgrammeTV();
        }
    }
    //else if ( qName == "star-rating" )
    //{
    	//if( m_balise == Star )
    	//{
    		//m_programmeTV.star = m_ch;
    		//m_balise = Rien;
    		//m_ch = "";
   		//}
    //}
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
        m_programmeTV.desc << ch;
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
    // On tri les chaines par numero de id
    QList<ChaineTV> listeTriee;
    do
    {
        int id = 99999;
        int index = 0;
        for (int i=0; i<m_listeChainesTV.count(); i++)
        {
            if ( m_listeChainesTV.at(i).id.toInt() < id )
            {
                id = m_listeChainesTV.at(i).id.toInt();
                index = i;
            }
        }
        listeTriee.append(m_listeChainesTV.at(index));
        m_listeChainesTV.removeAt(index);
    }
    while ( m_listeChainesTV.count() );
    m_listeChainesTV = listeTriee;
    return true;
}


void XmlDefaultHandler::draw()
{
    // Suppression de tous les items
    do
    {
        while ( m_viewProgrammes->scene()->items().count() )
        {
            delete m_viewProgrammes->scene()->items().first();
        }
        QCoreApplication::processEvents();
    }
    while ( m_viewProgrammes->scene()->items().count() );
    QD << m_viewProgrammes->scene()->items().count();
    m_viewProgrammes->setBackgroundBrush(QColor(Qt::red).light(188));
    // Ligne de l'heure courante
    m_ligneHeureCourante = new QGraphicsLineItem();
    m_ligneHeureCourante->setPen(QPen(QColor(Qt::red), 2, Qt::DashDotLine));
    if ( QDate::currentDate() != m_date )
        m_ligneHeureCourante->hide();
    m_viewProgrammes->scene()->addItem( m_ligneHeureCourante );
    QVariant v;
    v.setValue( ProgrammeTV() );
    m_ligneHeureCourante->setData(0, v );
    m_ligneHeureCourante->setZValue(16);
    // Dimensionnement de la scene en fonction du nombre de demi-heure (largeur) et de chaines (hauteur).
    m_viewProgrammes->setSceneRect(
        m_viewProgrammes->rect().x(),
        m_viewProgrammes->rect().y(),
        100+((48-(m_heureDebutJournee*2))*largeurProg),
        hauteurHeure+(m_listeChainesTV.count()*hauteurProg)
    );
    m_listeItemChaines.clear();
    m_listeItemHeures.clear();
    m_listeItemProgrammes.clear();
    // Case vide en haut/gauche
    GraphicsRectItem *item = new GraphicsRectItem(m_main,
                             QRectF(0, 0, 100, hauteurHeure),
                             "",
                             GraphicsRectItem::Chaine);
    item->setZValue(10);
    v.setValue( ProgrammeTV() );
    item->setData(0, v );
    m_viewProgrammes->scene()->addItem( item );
    m_listeItemChaines.append( item );
    // Creation de la colonne des chaines
    for (int i=0; i<m_listeChainesTV.count(); i++)
    {
        ChaineTV chaine = m_listeChainesTV.at(i);
        GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                 QRectF(0, hauteurHeure+(i*hauteurProg), 100, hauteurProg),
                                 chaine.name,
                                 GraphicsRectItem::Chaine);
        item->setZValue(17);
        m_viewProgrammes->scene()->addItem( item );
        m_listeItemChaines.append( item );
    }
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
    // Creation des programmes
    foreach(ProgrammeTV prog, m_listeProgrammesTV)
    {
        int ligne=0;
        foreach(ChaineTV chaine, m_listeChainesTV)
        {
            if ( chaine.id == prog.channel )
            {
                prog.channelName = chaine.name;
                break;
            }
            ligne++;
        }
        double x = QTime(0,0).secsTo( prog.start.time() )*(largeurProg/1800.0);
        x = x - ((m_heureDebutJournee*2)*largeurProg);
        double w =  prog.start.secsTo( prog.stop )*(largeurProg/1800.0);
        GraphicsRectItem *item = new GraphicsRectItem(m_main,
                                 QRectF(100+x,hauteurHeure+(ligne*hauteurProg),w,hauteurProg),
                                 prog.title,
                                 GraphicsRectItem::Programme
                                                     );
        item->setZValue(15);
        QVariant v;
        v.setValue( prog );
        item->setData(0, v );
        m_viewProgrammes->scene()->addItem( item );
        m_listeItemProgrammes.append( item );
        //QD << prog.channel << ligne << x << w << prog.title << prog.start << prog.stop;
    }
    posLigneHeureCourante();
    deplaceChaines(0);
    deplaceHeures(0);
    double x = QTime(0,0).secsTo( QTime::currentTime() )*(largeurProg/1800.0);
    x = 100+x-((m_heureDebutJournee*2)*largeurProg);
    m_viewProgrammes->centerOn(x ,0);
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
    return true;
}


void XmlDefaultHandler::init()
{
    m_listeChainesTV.clear();
    m_listeProgrammesTV.clear();
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
    foreach(GraphicsRectItem *item, m_listeItemProgrammes)
    {
        ProgrammeTV prog = item->data(0).value<ProgrammeTV>();
        if ( prog.start <= soiree && soiree < prog.stop )
        {
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
            if ( listeProgrammes.at(i).channel.toInt() < channel )
            {
                channel = listeProgrammes.at(i).channel.toInt();
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
    foreach(GraphicsRectItem *item, m_listeItemProgrammes)
    {
        ProgrammeTV prog = item->data(0).value<ProgrammeTV>();
        if ( prog.start <= maintenant && maintenant < prog.stop )
        {
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
            if ( listeProgrammes.at(i).channel.toInt() < channel )
            {
                channel = listeProgrammes.at(i).channel.toInt();
                index = i;
            }
        }
        listeTriee.append(listeProgrammes.at(index));
        listeProgrammes.removeAt(index);
    }
    return listeTriee;
}

