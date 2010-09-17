#ifndef CHANGETHUMBIMPL_H
#define CHANGETHUMBIMPL_H
//
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "defs.h"
#include "ui_changethumb.h"
//
class ChannelIconItem;
class MainWindowImpl;
class ChangeThumbImpl : public QDialog, public Ui::ChangeThumb
{
Q_OBJECT
public:
	ChangeThumbImpl( QWidget * parent, PairIcon pair, QString text, bool isChannel );
public slots:
	void channelIconClicked(ChannelIconItem *item, bool doubleClick);
private slots:
    void on_find_clicked();
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
    void on_addButton_clicked();
private:
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QStringList parse_html(QString html);
    QPixmap m_selectedPixmap;
    PairIcon m_pairIcon;
    QStringList m_urlList;
    QString m_url;
	int m_x;
	int m_y;
    QString m_selectedFilename;
    MainWindowImpl *m_main;
    bool m_isChannel;
private slots:
	void httpURL_done ();
	void httpThumbnail_done();
signals:
	void imageAvailable(PairIcon);
protected:
	void resizeEvent(QResizeEvent *event);
};
#endif





