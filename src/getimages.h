#ifndef RECUPIMAGES_H
#define RECUPIMAGES_H
//
#include <QSqlQuery>
#include <QHttp>
#include <QPixmap>
//
class GetImages : public QObject
{
Q_OBJECT
public:
	QPixmap pixmap(QString icon, QSqlQuery query);
	void setList(QStringList list, QSqlQuery query);
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
};
#endif
