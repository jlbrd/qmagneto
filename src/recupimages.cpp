#include "recupimages.h"
#include <QFile>
#include <QDebug>
#include <QVariant>
#include <QSqlError>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
RecupImages::RecupImages( QStringList list, QSqlQuery query ) 
	: QHttp(this), m_liste(list), m_query(query)
{
	QD << m_liste.count();
	connect(this, SIGNAL(done(bool)), this, SLOT(slotRequestFinished(bool)) );
	recup();

}
//
void RecupImages::slotRequestFinished(bool err)
{
	QByteArray data;
	data = readAll();
	
	QVariant clob(data);
	m_query.prepare("INSERT INTO images (icon, ok, data)"
	"VALUES (:icon, :ok, :data)");
	m_query.bindValue(":icon", m_liste.first().replace("'", "$"));
	m_query.bindValue(":ok",QString::number(1));
	m_query.bindValue(":data", clob);
	bool rc = m_query.exec();
	
    if (rc == false)
    {
        qDebug() << "Failed to insert record to db" << m_query.lastError();
    }
	close();
	m_liste.pop_front();
	recup();
}

void RecupImages::recup()
{
	if( m_liste.count() == 0 )
		return;
	QString icon = m_liste.first();
	setHost(icon.section("/", 2, 2));
	get("/"+icon.section("/", 3) );
}


void RecupImages::imageToTmp(QString icon, QSqlQuery query)
{
	m_query = query;
	QString queryString = "select * from images where icon = '" + icon.replace("'", "$")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return false;
    }
    if( m_query.next() )
	{
		QFile file("/tmp/qmagneto.jpg");
		if (!file.open(QIODevice::WriteOnly ))
		    return;
		file.write(m_query.value(2).toByteArray());
		file.close();
	}
}

