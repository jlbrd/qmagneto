#ifndef XMLDEFAULTHANDLER_H
#define XMLDEFAULTHANDLER_H
//
#include <QXmlDefaultHandler>
#include <QSqlQuery>
#include <QList>
#include <QDateTime>
#include <QMetaType>
#include <QFont>
#include <QPixmap>

#include "graphicsrectitem.h"
#include "getimages.h"
#include "defs.h"

class QGraphicsView;
class MainWindowImpl;
class GetImages;
//
class XmlDefaultHandler : public QXmlDefaultHandler
{
private:	
	float m_hourHeight;
	float m_progHeight;
	float m_progWidth;
	void clearView();
	QList<TvProgram> sortedPrograms(QList<TvProgram> list);
	QList<TvChannel> sortedChannels();
	QDate m_date;
	int m_hourBeginning;
	bool endDocument();
	enum Balise { None, Channel, Title, SubTitle, Desc, Category, Aspect, DisplayName, Star};
	Balise m_balise;
	MainWindowImpl *m_main;
	QGraphicsView *m_programsView;
	TvChannel m_chaineTV;
	QList<TvChannel> m_TvChannelsList;
	TvProgram m_programTV;
	QList<TvProgram> m_TvProgramsList;
	QList<GraphicsRectItem *> m_listeItemChaines;
	QList<GraphicsRectItem *> m_listeItemHeures;
	QList<GraphicsRectItem *> m_programsItemsList;
	QStringList m_imagesList;
	QGraphicsLineItem *m_currentTimeLine;
	QString m_ch;
	QSqlQuery m_query;
  	bool connectDB();
  	GetImages *m_getImages;
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
	void nowCenter();
	PairIcon pairIcon(QString icon);
	void imageToTmp(QString icon, bool isChannel);
	QList<TvChannel> channels() { return m_TvChannelsList; }
	QList<TvProgram>  programsMaintenant();
	QList<TvProgram> eveningPrograms();
	void evening();
	void setHeureDebutJournee( int value) { m_hourBeginning = value; }
	void currentTimeLinePosition();
	void init();
	void setDate(QDate value) { m_date = value; }
	void deplaceHeures(int value);
	void deplaceChaines(int value);
	bool characters( const QString & ch );
	bool endElement( const QString & namespaceURI, const QString & localName, const QString & qName );
	bool startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
	XmlDefaultHandler(MainWindowImpl *main, QGraphicsView *programs);
	~XmlDefaultHandler();
	QList<GraphicsRectItem *> listeItemProgrammes() { return m_programsItemsList; };
  	bool readFromDB();
};
#endif
