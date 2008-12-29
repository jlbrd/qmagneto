#include "recupimages.h"
#include <QFile>
#include <QVariant>
#include <QSqlError>
#include <QImage>
#include <QDir>
#include <QUrl>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
RecupImages::RecupImages( QStringList list, QSqlQuery query )
        : QObject(), m_liste(list), m_query(query)
{
    m_http = 0;
    QD;
}
//
RecupImages::~RecupImages()
{}
void RecupImages::slotRequestFinished(bool err)
{
    if ( err )
    {
        QD << m_http->errorString();
        return;
    }
    if ( !m_liste.count() )
        return;
    QByteArray data;
    data = m_http->readAll();
    if ( data.isEmpty() )
        return;
    QVariant clob(data);
    m_query.prepare("update images set ok='1', data=:data where icon='" +m_liste.first().replace("'", "$")+ "'");
    m_query.bindValue(":data", clob);
    bool rc = m_query.exec();
    QD << "recuperation ok pour:" << m_liste.first() << "taille:" << data.size();
    if (rc == false)
    {
        qDebug() << "Failed to insert record to db" << m_query.lastError();
    }
    m_http->close();
    if ( m_liste.count() )
    {
        m_liste.pop_front();
        recup();
    }
}

void RecupImages::recup()
{
    if ( m_liste.count() == 0 )
        return;
    QString icon = m_liste.first();
#ifdef Q_OS_WIN32
    QUrl url(icon);
    m_http->setHost(url.host());
    m_http->get( url.toString());
    //QD << "get" << url;
#else
    m_http->setHost(icon.section("/", 2, 2));
    m_http->get("/"+icon.section("/", 3) );
    //QD << "get" << m_liste.count() << icon.section("/", 2, 2)<< icon;
#endif
}


void RecupImages::imageToTmp(QString icon, QSqlQuery query, bool isChaine)
{
    m_query = query;
    QString queryString = "select * from images where icon = '" + icon.replace("'", "$")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return;
    }
    if ( m_query.next() )
    {
        QFile file;
        if ( isChaine )
            file.setFileName(QDir::tempPath()+"/qmagnetochaine.jpg");
        else
            file.setFileName(QDir::tempPath()+"/qmagnetoprog.jpg");
        if (!file.open(QIODevice::WriteOnly ))
            return;
        file.write(m_query.value(2).toByteArray());
        file.close();
    }
}


void RecupImages::setListe(QStringList liste, QSqlQuery query)
{
    if ( m_http )
    {
        m_http->clearPendingRequests();
        m_http->abort();
        m_http->close();
        delete m_http;
    }
    m_http = new QHttp( this );
    m_query = query;
    m_liste = liste;
    connect(m_http, SIGNAL(done(bool)), this, SLOT(slotRequestFinished(bool)) );
    recup();
}


QPixmap RecupImages::pixmap(QString icon, QSqlQuery query)
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

    if ( m_query.next() )
    {
        return QPixmap::fromImage( QImage::fromData( ( m_query.value(2).toByteArray() ) ) );
    }
    //QD << icon << "non trouvé";
    return QPixmap();
}
