#ifndef RECUPIMAGES_H
#define RECUPIMAGES_H
//
#include <QSqlQuery>
#include <QHttp>
#include <QPixmap>
#include "xmldefaulthandler.h"

//
class GetImages : public QObject
{
Q_OBJECT
public:
	PairIcon pairIcon(QString icon, QSqlQuery query);
	void setList(QStringList list, QSqlQuery query, QString proxyAddress=QString(), int proxyPort=0);
	void imageToTmp(QString icon, QSqlQuery query, bool isChannel);
	void get();
	GetImages(QStringList list, QSqlQuery query);
	~GetImages();
private:
	QStringList m_list;
	QSqlQuery m_query;
	QHttp *m_http;
private slots:
	void slotRequestFinished(bool err);
signals:
	void imageAvailable(PairIcon);
};
#endif
