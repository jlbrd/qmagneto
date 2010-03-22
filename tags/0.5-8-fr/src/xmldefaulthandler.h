#ifndef XMLDEFAULTHANDLER_H
#define XMLDEFAULTHANDLER_H
//
#include <QXmlDefaultHandler>
#include <QSqlQuery>
#include <QList>
#include <QDateTime>
#include <QMetaType>
#include <QFont>
#include "graphicsrectitem.h"
#include "recupimages.h"

typedef struct
{
	QString id;
	QString name;
	QString icon;	
	bool enabled;
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
	QStringList resume;
	QString histoire;
	QString aspect;
	QString credits;
	QString director;
	QString star;
	QString icon;
} ProgrammeTV;
Q_DECLARE_METATYPE(ProgrammeTV)

class QGraphicsView;
class MainWindowImpl;
class RecupImages;
//
class XmlDefaultHandler : public QXmlDefaultHandler
{
private:	
	float m_hourHeight;
	float m_progHeight;
	float m_progWidth;
	void clearView();
	QList<ProgrammeTV> sortedPrograms(QList<ProgrammeTV> list);
	QList<ChaineTV> sortedChannels();
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
	QStringList m_listeImages;
	QGraphicsLineItem *m_ligneHeureCourante;
	QString m_ch;
	QSqlQuery m_query;
  	bool connectDB();
  	RecupImages *m_recupImages;
protected:
	virtual bool startDocument();
public:
	void setHourHeight(float value) { m_hourHeight = value; }
	float hourHeight() { return m_hourHeight; }
	void setProgHeight(float value) { m_progHeight = value; }
	float progHeight() { return m_progHeight; }
	void setProgWidth(float value) { m_progWidth = value; }
	float progWidth() { return m_progWidth; }
	QDate maximumDate();
	QDate minimumDate();
	void centreMaintenant();
	QPixmap pixmap(QString icon);
	void imageToTmp(QString icon, bool isChaine);
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
	~XmlDefaultHandler();
	QList<GraphicsRectItem *> listeItemProgrammes() { return m_listeItemProgrammes; };
  	bool readFromDB();
};
#endif