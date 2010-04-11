/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2010  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qmagneto/
*
*/

#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H
//
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QProcess>
#include <QBuffer>
#include <QHttp>
#include "xmldefaulthandler.h"
#include "ui_mainwindow.h"
#include "ui_config.h"
#include "ui_findwidget.h"
class FindGlobalImpl;
class DownloadManager;
class GetLastVersion;
//
//
class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT
protected:
	virtual void closeEvent( QCloseEvent * event );
	//virtual void changeEvent(QEvent * event);
	void resizeEvent(QResizeEvent * event);
public:
	QGraphicsView *view() { return graphicsViewProgrammes; }
	void emitShowIconsStatus();
	void showAlertWhenStarts(int id, bool active);
	void setDatabaseName(QString value);
	QString databaseName() { return m_databaseName; }
	FindGlobalImpl *findGlobalImpl() { return m_findGlobalImpl; }
	bool proxyEnabled() { return m_proxyEnabled; };
	int proxyPort() { return m_proxyPort; };
	QString proxyAddress() { return m_proxyAddress; };
	QString proxyUsername() { return m_proxyUsername; };
	QString proxyPassword() { return m_proxyPassword; };
	enum Kind { Recording, Reading, Alert };
	enum ShowMode { Grid, Channel };
	QString numBox(QString s);
	QString showDescription(TvProgram prog, bool isExpandedItem=false);
	void saveRecording();
	void readRecording();
	bool systrayStarts() { return m_systrayStarts; }
	void slotItemClicked(GraphicsRectItem *item, int id);
	//void addProgram(QString channel, QString id, QDateTime start, QDateTime end, QString title=QString(), QString desc=QString(), bool showDialog=true);
	void addProgram(TvProgram prog=TvProgram(), QString title=QString(), bool showDialog=true, Kind kind=Recording, QString directory=QString(), int tableProgramsCurrentRow=-1);
	void init();
	void readTvGuide();
	MainWindowImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
	~MainWindowImpl();
	QString directory() { return m_directory;	}
    bool groupGoogleImage() { return m_groupGoogleImage;	}
    bool groupGoogleImageCategories() { return m_groupGoogleImageCategories;	}
    QStringList googleImageCategories() { return m_googleImageCategories;	}
	static QString iniPath();
	void setProgressBarFormat( QString title );
	XmlDefaultHandler *handler() { return m_handler; }
    ShowMode showMode() { return m_showMode; }

public slots:
	void slotItemClicked(QListWidgetItem *item);
	void slotPopulateDB(int source=-1, QString XmlFilename=QString());
    void slotScheduledUpdate(bool fromOptionDialog=false);
	void slotDataReadProgress(qint64 done,qint64 total);
	void slotTimerMinute();
private slots:
	void on_showGrid_clicked();
	void slotReadThumbsFromDB();
	void on_dayFirstButton_clicked();
	void on_dayLastButton_clicked();
	void on_actionFind_triggered();
	void on_reduceButton_clicked();
	void slotRestoreWindowFromSystray();
	void on_programsTable_doubleClicked(QModelIndex );
	void on_programsModify_clicked();
	void slotPopulateParse();
	void slotPopulateUnzip(bool error=false);
	void on_action_FindInPage_triggered();
	void on_dateEdit_dateChanged(QDate date);
	void on_action_Channels_triggered();
	void on_action_About_triggered();
	void on_action_AboutQt_triggered();
	void itemDoubleClicked(QListWidgetItem *item);
	void on_action_Programs_triggered();
	void on_now_clicked();    
	void on_action_Quit_triggered();
	void on_action_Options_triggered();
	void slotTimer3Seconde();
	void on_evening_clicked();
	void on_dayBeforeButton_clicked();
	void on_dayAfterButton_clicked();
	void slotHorizontalValueChanged(int value);
	void slotVerticalValueChanged(int value);
	void slotFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void slotReadyReadStandardOutput();
	void slotReadyReadStandardError();
	void slotTimer();
	void slotDelete(int row=-1);
	void slotIconActivated(QSystemTrayIcon::ActivationReason reason);
	void slotToggleFullScreen();
	void slotFindWidget_textChanged(QString text="", bool backward=false, bool fromBegin=true);
	void slotFindPrevious();
	void slotFindNext();
	void slotCustomCommandError();
private:
	QStringList m_listThumbs;
	FindGlobalImpl *m_findGlobalImpl;
	bool m_isMaximized;
	QString m_databaseName;
	int m_proxyPort;
	QString m_proxyAddress;
	QString m_proxyUsername;
	QString m_proxyPassword;
	bool m_proxyEnabled;
	QString m_xmlFilename;
	int m_hourBeginning;
	enum { Idle, Working, Completed };
	void saveIni();
	void readIni();
	void createTrayIcon();
	QString m_command;
	QString m_commandOptions;
	QString m_readingCommand;
	QString m_readingCommandOptions;
	XmlDefaultHandler *m_handler;
	QDate m_currentDate;
	QTimer *m_timerMinute;
	QTimer *m_timer3Seconde;
    QAction *restoreAction;
    QAction *quitAction;
    QMenu *trayIconMenu;
    QSystemTrayIcon *trayIcon;
    bool m_systrayStarts;
    QDialog *m_programsDialog;
    Ui::Config uiConfig;
	Ui::FindWidget uiFind;
	QString m_directory;
	QString m_filenameFormat;
	QAction *actionToggleFullScreen;
    int m_comboURL;
    int  m_sourceUpdate;
    QString m_customCommand;
    QString m_customCommandFile;
	QWidget *m_findWidget;
	QTimer *m_autoHideTimer;
	int m_httpId;
	DownloadManager *m_http;
	GetLastVersion *m_httpVersion;
    bool m_scheduledUpdate;
    bool m_atStartup;
    bool m_everyDay;
    int m_everyDayAt;
    bool m_onlyIfOutOfDate;
    int m_onlyIfOutOfDateDay;
    QTimer *m_scheduledUpdateTimer;
    bool m_groupGoogleImage;
    bool m_groupGoogleImageCategories;
    QStringList m_googleImageCategories;
    QString m_channel;
    ShowMode m_showMode;
signals:
	void showIcon(int, int, bool);
};
typedef struct  
{
	uint id;
	QString channel;
	QDateTime start;
	int before;
	QDateTime end;
	int after;
	int state;
	QString channelNum;
	QString directory;
	QTimer *timer;
	QProcess *process;
	MainWindowImpl::Kind kind;
} Program;
Q_DECLARE_METATYPE(Program)
//
#endif








