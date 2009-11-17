#ifndef GOOGLEIMAGE_H
#define GOOGLEIMAGE_H
//
#include <QObject>
#include <QHttp>
#include <QSqlQuery>
#include "defs.h"
//
typedef QPair<QString, QString> Pair;
class GetImages;
class MainWindowImpl;

class GoogleImage : public QObject
{
Q_OBJECT
public:
	void stop();
	GoogleImage(MainWindowImpl *parent=0);
	void setList(QStringList list, QSqlQuery query, QString proxyAddress=QString(), int proxyPort=0, QString proxyUsername=QString(), QString proxyPassword=QString());
	~GoogleImage();
	void imageToTmp(QString icon, QSqlQuery query, bool isChannel);
	PairIcon pairIcon(QString icon, QSqlQuery query);
private:	
	void google_search(QString s);
	void getThumbnail(QString URL);
    QString parse_html();
    QString *html;
    QHttp *httpURL;
    QString *resultHtml;
    QRegExp rx_href;
    QRegExp rx_data,rx_start,rx_other;
    MainWindowImpl *m_main;
    QStringList m_list;
    Pair m_pair;
    QSqlQuery m_query;
    QString m_proxyAddress;
    int m_proxyPort;
    QString m_proxyUsername;
    QString m_proxyPassword;
	QHttp *m_httpThumbnail;
private slots:
	void httpURL_done ( bool err );
	void httpThumbnail_done(bool err);
signals:
	void imageAvailable(PairIcon);
};
#endif
