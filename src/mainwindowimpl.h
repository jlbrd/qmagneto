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
#include "ui_programmes.h"
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
	enum Type { Enregistrement, Lecture };
	QString numBox(QString s);
	QString afficheDescription(ProgrammeTV prog);
	void sauveEnregistrements();
	void litEnregistrements();
	bool demarrerEnIcone() { return m_demarrerEnIcone; }
	void itemClique(GraphicsRectItem *item);
	//void ajouterProgramme(QString chaine, QString id, QDateTime debut, QDateTime fin, QString titre=QString(), QString desc=QString(), bool afficherDialogue=true);
	void ajouterProgramme(ProgrammeTV prog=ProgrammeTV(), QString titre=QString(), bool afficherDialogue=true, Type type=Enregistrement);
	void init();
	void litProgrammeTV();
	MainWindowImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
	~MainWindowImpl();
	QString repertoire() { return m_repertoire;	}
	static QString cheminIni();
	void populateDB(bool depuisFichier, QString nomFichierXML);
public slots:
	void itemClique(QListWidgetItem *item);
private slots:
	void on_dateEdit_dateChanged(QDate date);
	void on_action_Canaux_triggered();
	void on_actionA_propos_triggered();
	void on_actionA_propos_de_Qt_triggered();
	void itemDoubleClicked(QListWidgetItem *item);
	void on_action_Programmes_triggered();
	void on_soiree_clicked();
	void on_action_Quitter_triggered();
	void on_action_Configurer_triggered();
	void slotTimerMinute();
	void slotTimer3Seconde();
	void on_maintenant_clicked();
	void on_boutonJourAvant_clicked();
	void on_boutonJourApres_clicked();
	void slotHorizontalValueChanged(int value);
	void slotVerticalValueChanged(int value);
	void slotFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void slotReadyReadStandardOutput();
	void slotReadyReadStandardError();
	void slotTimer();
	void slotSupprimer();
	void slotIconActivated(QSystemTrayIcon::ActivationReason reason);
	void slotToggleFullScreen();
private:
	QString m_nomFichierXML;
	int m_heureDebutJournee;
	enum { Attente, EnCours, Termine };
	void sauveINI();
	void litINI();
	void createTrayIcon();
	QString m_command;
	QString m_commandOptions;
	QString m_commandLecture;
	QString m_commandLectureOptions;
	XmlDefaultHandler *m_handler;
	QDate m_currentDate;
	QTimer *m_timerMinute;
	QTimer *m_timer3Seconde;
    QAction *restoreAction;
    QAction *quitAction;
    QMenu *trayIconMenu;
    QSystemTrayIcon *trayIcon;
    bool m_demarrerEnIcone;
    QDialog *m_dialogProgrammes;
    Ui::Programmes m_uiProgrammes;
    Ui::Config uiConfig;
	QString m_repertoire;
	QString m_formatNomFichier;
	QAction *actionToggleFullScreen;
    int m_comboURL;
    bool m_depuisFichier;
};
typedef struct  
{
	uint id;
	QString chaine;
	QDateTime debut;
	QDateTime fin;
	int etat;
	QString numChaine;
	QTimer *timer;
	QProcess *process;
	MainWindowImpl::Type type;
} Programme;
Q_DECLARE_METATYPE(Programme)
//
#endif







