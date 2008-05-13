#ifndef XMLDEFAULTHANDLER_H
#define XMLDEFAULTHANDLER_H
//
#include <QXmlDefaultHandler>
#include <QList>
#include <QDateTime>
#include <QMetaType>
#include "graphicsrectitem.h"

typedef struct
{
	QString id;
	QString name;
	QString icon;	
} ChaineTV;
Q_DECLARE_METATYPE(ChaineTV)
//
typedef struct
{
	QDateTime start;
	QDateTime stop;
	QString channel;
	QString channelName;
	QString title;
	QString subTitle;
	QStringList category;
	QStringList desc;
	QString aspect;
	QString credits;
	QString director;
	QString star;
} ProgrammeTV;
Q_DECLARE_METATYPE(ProgrammeTV)

class QGraphicsView;
class MainWindowImpl;
//
class XmlDefaultHandler : public QXmlDefaultHandler
{
protected:
	virtual bool startDocument();
public:
	QList<ChaineTV> chaines() { return m_listeChainesTV; }
	QList<ProgrammeTV>  programmesMaintenant();
	QList<ProgrammeTV> programmesSoiree();
	void soiree();
	void setHeureDebutJournee( int value) { m_heureDebutJournee = value; }
	void posLigneHeureCourante();
	void init();
	void setDate(QDate value) { m_date = value; }
	void deplaceHeures(int value);
	void deplaceChaines(int value);
	bool characters( const QString & ch );
	bool endElement( const QString & namespaceURI, const QString & localName, const QString & qName );
	bool startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
	XmlDefaultHandler(MainWindowImpl *main, QGraphicsView *programmes);
	void draw();
	QList<GraphicsRectItem *> listeItemProgrammes() { return m_listeItemProgrammes; };
  	bool readFromDB();
private:	
	QDate m_date;
	int m_heureDebutJournee;
	bool endDocument();
	enum Balise { Rien, Channel, Title, SubTitle, Desc, Category, Aspect, DisplayName, Star};
	Balise m_balise;
	MainWindowImpl *m_main;
	QGraphicsView *m_viewProgrammes;
	ChaineTV m_chaineTV;
	QList<ChaineTV> m_listeChainesTV;
	ProgrammeTV m_programmeTV;
	QList<ProgrammeTV> m_listeProgrammesTV;
	QList<GraphicsRectItem *> m_listeItemChaines;
	QList<GraphicsRectItem *> m_listeItemHeures;
	QList<GraphicsRectItem *> m_listeItemProgrammes;
	QGraphicsLineItem *m_ligneHeureCourante;
	QString m_ch;
  	bool connectDB();
};
#endif
