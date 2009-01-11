#include "getimages.h"
#include <QFile>
#include <QVariant>
#include <QSqlError>
#include <QImage>
#include <QDir>
#include <QUrl>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
GetImages::GetImages( QStringList list, QSqlQuery query )
        : QObject(), m_list(list), m_query(query)
{
    m_http = 0;
}
//
GetImages::~GetImages()
{
    if ( m_http )
    {
        m_http->clearPendingRequests();
        m_http->abort();
        m_http->close();
        delete m_http;
    }
}
void GetImages::slotRequestFinished(bool err)
{
    if ( err )
    {
        QD << m_http->errorString();
        return;
    }
    if ( !m_list.count() )
        return;
    QByteArray data;
    data = m_http->readAll();
    if ( data.isEmpty() )
        return;
    QVariant clob(data);
    m_query.prepare("update images set ok='1', data=:data where icon='" +m_list.first().replace("'", "$")+ "'");
    m_query.bindValue(":data", clob);
    bool rc = m_query.exec();
    emit imageAvailable(PairIcon(m_list.first(),
                                 QPixmap::fromImage( QImage::fromData( ( data ) ) )
                                )
                       );
    QD << tr("download ok for:") << m_list.first() << tr("size:") << data.size();
    if (rc == false)
    {
        qDebug() << "Failed to insert record to db" << m_query.lastError();
    }
    m_http->close();
    if ( m_list.count() )
    {
        m_list.pop_front();
        get();
    }
}

void GetImages::get()
{
    if ( m_list.count() == 0 )
        return;
    QString icon = m_list.first();
//#ifdef Q_OS_WIN32
    QUrl url(icon);
    m_http->setHost(url.host());
    m_http->get( url.toString());
    //QD << "get" << url;
    /*#else
        m_http->setHost(icon.section("/", 2, 2));
        m_http->get("/"+icon.section("/", 3) );
        //QD << "get" << m_list.count() << icon.section("/", 2, 2)<< icon;
    #endif*/
}


void GetImages::imageToTmp(QString icon, QSqlQuery query, bool isChannel)
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
        if ( isChannel )
            file.setFileName(QDir::tempPath()+"/qmagnetochannel.jpg");
        else
            file.setFileName(QDir::tempPath()+"/qmagnetoprog.jpg");
        if (!file.open(QIODevice::WriteOnly ))
            return;
        file.write(m_query.value(2).toByteArray());
        file.close();
    }
}


void GetImages::setList(QStringList list, QSqlQuery query)
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
    m_list = list;
    connect(m_http, SIGNAL(done(bool)), this, SLOT(slotRequestFinished(bool)) );
    get();
}


PairIcon GetImages::pairIcon(QString icon, QSqlQuery query)
{
    PairIcon pair;
    m_query = query;
    QString queryString = "select * from images where icon = '" + icon.replace("'", "$")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to select record to db" << m_query.lastError();
        qDebug() << queryString;
        return pair;
    }

    if ( m_query.next() )
    {
        pair = PairIcon(icon,
                        QPixmap::fromImage( QImage::fromData( ( m_query.value(2).toByteArray() ) ) )
                       );
    }
    //QD << icon << "non trouvÃ©";
    return pair;
}
