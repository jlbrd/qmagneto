#include "googleimage.h"
#include "mainwindowimpl.h"
#include <QDir>
#include <QUrl>
#include <QSqlError>
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

GoogleImage::GoogleImage(MainWindowImpl *parent)
        : QObject(parent), m_main(parent)
{
    httpURL = 0;
    m_httpThumbnail = 0;
    html=new QString();
    resultHtml=new QString();
    rx_start.setPattern("setResults[(][[]");

    rx_data.setPattern("[[]\".*[]],");
    rx_data.setMinimal(true);

    rx_href.setPattern("(http://.*)[\\\\|\"]");
    rx_href.setMinimal(true);

    rx_other.setPattern(",[\"](.*)[\"]");
    rx_other.setMinimal(true);

}
void GoogleImage::setList(QStringList list, QSqlQuery query, QString proxyAddress, int proxyPort, QString proxyUsername, QString proxyPassword)
{
    m_query = query;
    m_proxyAddress = proxyAddress;
    m_proxyPort = proxyPort;
    m_proxyUsername = proxyUsername;
    m_proxyPassword = proxyPassword;
    QD<<list;
    m_list = list;
    if ( m_list.count() )
        google_search(m_list.first());
}
//
GoogleImage::~GoogleImage()
{
    if ( httpURL )
    {
        //httpURL->clearPendingRequests();
        httpURL->abort();
        httpURL->close();
        httpURL->deleteLater();
    }
    if ( m_httpThumbnail )
    {
        //m_httpThumbnail->clearPendingRequests();
        m_httpThumbnail->abort();
        m_httpThumbnail->close();
        m_httpThumbnail->deleteLater();
        m_httpThumbnail = 0;
    }
    delete html;
    delete resultHtml;

}
void GoogleImage::google_search(QString ss)
{
    QString s,search_string,content_type,image_size,coloration,site_search,safeFilter,search_url;
    safeFilter="active";
    search_string=ss.replace(" ","+").toLocal8Bit();

    if ( httpURL )
        delete httpURL;
    httpURL = new QHttp();
    if ( !m_proxyAddress.isEmpty() )
    {
        httpURL->setProxy(m_proxyAddress, m_proxyPort, m_proxyUsername, m_proxyPassword);
    }
    connect(this->httpURL, SIGNAL(done(bool)), this, SLOT(httpURL_done(bool)));

    httpURL->setHost("images.google.com");
    search_url="/images?&q="+search_string+"&safe="+safeFilter;
    httpURL->get(search_url);

    QD << "Searching URL:"<< search_url;
}
void GoogleImage::httpURL_done ( bool err )
{
    if ( err )
    {
        QD << httpURL->errorString();
    }
    else
    {
        QByteArray r;

        r=QByteArray::fromPercentEncoding(httpURL->readAll());
        *html=r;
        QString URL = parse_html();
        QD << "URL " << URL;
        if ( m_list.count() )
        {
            if ( !URL.isEmpty() )
            {
                m_pair = Pair(m_list.first(), URL);
                getThumbnail(URL);
            }
            else
            {
                m_list.pop_front();
                if( m_list.count() )
                google_search(m_list.first());

            }
        }
    }
}
QString GoogleImage::parse_html()
{
    QString s;
    QString href_original_image, href_thumbnail_at_google, href_original_page, href_google_thumb_download;
    QString ID_google_thumb, href_thumbnail;
    int pos,pos2,i;

    pos = 0;
    pos = rx_start.indexIn(*html, pos);
    if (pos==-1)
    {
        QD << "Something changed in the google image page html source. /nThis software must be rewritten";
        return QString();
    }
    html->remove(0,pos+rx_start.cap(0).size());
    pos=0;
    if ((pos = rx_data.indexIn(*html, pos)) != -1 )
    {
        pos2 = i = 0;
        s=rx_data.cap(0);
        s.replace("\\","\\\\");
        while ((pos2 = rx_href.indexIn( s, pos2)) != -1)
        {
            switch (i)
            {
            case 0:
                href_thumbnail_at_google = rx_href.cap(1);
                break;
            case 1:
                href_original_page = rx_href.cap(1);
                break;
            case 2:
                href_original_image= rx_href.cap(1);
                href_original_image=QUrl(href_original_image).fromPercentEncoding(href_original_image.toLocal8Bit());
                break;
            case 3:
                href_google_thumb_download = rx_href.cap(1);
                break;
            }
            pos2 += rx_href.matchedLength();
            i++;
        }
        pos2 = i = 0;
        while ((pos2 = rx_other.indexIn( s, pos2)) != -1)
        {
            switch (i)
            {
            case 1:
                ID_google_thumb=rx_other.cap(1);
                href_thumbnail = href_google_thumb_download
                                 + "?q=tbn:" + ID_google_thumb +
                                 href_thumbnail_at_google;
                break;
            }
            pos2 += rx_other.matchedLength();
            i++;
        }
    }
    return href_thumbnail;
}
//
void GoogleImage::getThumbnail(QString URL)
{
    QD << "getThumbnail" <<URL << m_list.first();
    QUrl url(URL);
    if ( m_httpThumbnail )
        delete m_httpThumbnail;
    m_httpThumbnail = new QHttp();
    if ( !m_proxyAddress.isEmpty() )
    {
        m_httpThumbnail->setProxy(m_proxyAddress, m_proxyPort, m_proxyUsername, m_proxyPassword);
    }
    connect(this->m_httpThumbnail, SIGNAL(done(bool)), this, SLOT(httpThumbnail_done(bool)));
    m_httpThumbnail->setHost(url.host());
    m_httpThumbnail->get( url.toString());
}
//
void GoogleImage::httpThumbnail_done(bool err)
{
    if ( err )
    {
        QD << m_httpThumbnail->errorString();
    }
    else
    {
        QByteArray data;
        data = m_httpThumbnail->readAll();
        if ( data.isEmpty() )
            return;
        QVariant clob(data);
        m_query.prepare("update images set ok='1', data=:data where icon='" +m_pair.first.replace("'", "$")+ "'");
        m_query.bindValue(":data", clob);
        bool rc = m_query.exec();
        if (rc == false)
        {
            QD << "Failed to insert record to db" << m_query.lastError();
        }
        emit imageAvailable(
            PairIcon(
                m_pair.first,
                QPixmap::fromImage( QImage::fromData( ( data ) ) )
            )
        );
        QD << tr("download ok for:") << m_pair.first << tr("size:") << data.size();

    }
    m_list.pop_front();
    if ( m_list.count() )
        google_search(m_list.first());
}
//
void GoogleImage::stop()
{
    m_list.clear();
}

void GoogleImage::imageToTmp(QString icon, QSqlQuery query, bool isChannel)
{
    m_query = query;
    QString queryString = "select * from images where icon = '" + icon.replace("'", "$")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return;
    }
    if ( m_query.next() )
    {
        QFile file;
        QImage image = QImage::fromData( ( m_query.value(2).toByteArray() ) );
        if ( isChannel )
            file.setFileName(QDir::tempPath()+"/qmagnetochannel.jpg");
        else
            file.setFileName(QDir::tempPath()+"/qmagnetoprog.jpg");
        if (!file.open(QIODevice::WriteOnly ))
            return;
        image.save(&file, "JPG");
        file.close();
        while ( m_query.next() );
    }
}

PairIcon GoogleImage::pairIcon(QString icon, QSqlQuery query)
{
    PairIcon pair;
    m_query = query;
    QString queryString = "select * from images where icon = '" + icon.replace("'", "$")+ "'";
    bool rc = m_query.exec(queryString);
    if (rc == false)
    {
        QD << "Failed to select record to db" << m_query.lastError();
        QD << queryString;
        return pair;
    }

    if ( m_query.next() )
    {
        pair = PairIcon(icon,
                        QPixmap::fromImage( QImage::fromData( ( m_query.value(2).toByteArray() ) ) )
                       );
    }
    return pair;
}
