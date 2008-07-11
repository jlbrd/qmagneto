#ifndef RECUPIMAGES_H
#define RECUPIMAGES_H
//
#include <QSqlQuery>
#include <QHttp>
//
class RecupImages : public QHttp
{
Q_OBJECT
public:
	void imageToTmp(QString icon, QSqlQuery query);
	void recup();
	RecupImages(QStringList list, QSqlQuery query);
private:
	QStringList m_liste;
	QSqlQuery m_query;
private slots:
	void slotRequestFinished(bool err);
};
#endif
