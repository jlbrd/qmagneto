/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2009  Jean-Luc Biord
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
* Program URL   : http://code.google.com/p/qmagneto/
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
#include "ui_programs.h"
#include "ui_config.h"
#include "ui_findwidget.h"
//

//
class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT
protected:
	virtual void closeEvent( QCloseEvent * event );
	void resizeEvent(QResizeEvent * event);
public:
	bool proxyEnabled() { return m_proxyEnabled; };
	int proxyPort() { return m_proxyPort; };
	QString proxyAddress() { return m_proxyAddress; };
	enum Kind { Recording, Reading };
	QString numBox(QString s);
	QString showDescription(TvProgram prog);
	void saveRecording();
	void readRecording();
	bool systrayStarts() { return m_systrayStarts; }
	void slotItemClicked(GraphicsRectItem *item);
	//void addProgram(QString channel, QString id, QDateTime start, QDateTime end, QString title=QString(), QString desc=QString(), bool showDialog=true);
	void addProgram(TvProgram prog=TvProgram(), QString title=QString(), bool showDialog=true, Kind kind=Recording);
	void init();
	void readTvGuide();
	MainWindowImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
	~MainWindowImpl();
	QString directory() { return m_directory;	}
	static QString iniPath();
	void populateDB(bool fromFile, QString XmlFilename);
public slots:
	void slotItemClicked(QListWidgetItem *item);
private slots:
	void on_action_Find_triggered();
	void slotReleaseVersion(bool error);
	void on_dateEdit_dateChanged(QDate date);
	void on_action_Channels_triggered();
	void on_action_About_triggered();
	void on_action_AboutQt_triggered();
	void itemDoubleClicked(QListWidgetItem *item);
	void on_action_Programs_triggered();
	void on_now_clicked();
	void on_action_Quit_triggered();
	void on_action_Options_triggered();
	void slotTimerMinute();
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
	void slotDelete();
	void slotIconActivated(QSystemTrayIcon::ActivationReason reason);
	void slotToggleFullScreen();
	void slotFindWidget_textChanged(QString text="", bool backward=false, bool fromBegin=true);
	void slotFindPrevious();
	void slotFindNext();
private:
	int m_proxyPort;
	QString m_proxyAddress;
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
    Ui::Programs m_programsUi;
    Ui::Config uiConfig;
	Ui::FindWidget uiFind;
	QString m_directory;
	QString m_filenameFormat;
	QAction *actionToggleFullScreen;
    int m_comboURL;
    bool m_fromFile;
    bool m_checkNewVersion;
   	QHttp *m_http;
	QWidget *m_findWidget;
	QTimer *m_autoHideTimer;

};
typedef struct  
{
	uint id;
	QString channel;
	QDateTime start;
	QDateTime end;
	int state;
	QString channelNum;
	QTimer *timer;
	QProcess *process;
	MainWindowImpl::Kind kind;
} Program;
Q_DECLARE_METATYPE(Program)
//
#endif







