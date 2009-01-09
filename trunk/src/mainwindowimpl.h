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
//

//
class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT
protected:
	virtual void closeEvent( QCloseEvent * event );
	void resizeEvent(QResizeEvent * event);
public:
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
	QString m_directory;
	QString m_filenameFormat;
	QAction *actionToggleFullScreen;
    int m_comboURL;
    bool m_fromFile;
    bool m_checkNewVersion;
   	QHttp *m_http;

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







