#ifndef RECUPIMAGES_H
#define RECUPIMAGES_H
//
#include <QSqlQuery>
#include <QHttp>
#include <QPixmap>
//
class RecupImages : public QObject
{
Q_OBJECT
public:
	QPixmap pixmap(QString icon, QSqlQuery query);
	void setListe(QStringList liste, QSqlQuery query);
	void imageToTmp(QString icon, QSqlQuery query, bool isChaine);
	void recup();
	RecupImages(QStringList list, QSqlQuery query);
	~RecupImages();
private:
	QStringList m_liste;
	QSqlQuery m_query;
	QHttp *m_http;
private slots:
	void slotRequestFinished(bool err);
};
#endif
