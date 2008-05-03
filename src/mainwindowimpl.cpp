#include "mainwindowimpl.h"
#include "ui_programme.h"
#include "listmaintenant.h"
#include "visu.h"
#include "ui_about.h"
#include <QHeaderView>
#include <QTimer>
#include <QFileDialog>
#include <QProcess>
#include <QSettings>
//#include <signal.h>
#include <QScrollBar>
#include <QListWidget>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QClipboard>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
#define VERSION "1.0"
//
MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f)
        : QMainWindow(parent, f)
{
    setupUi(this);
    menu_Affichage->addAction(dockDesc->toggleViewAction());
    menu_Affichage->addAction(dockSoiree->toggleViewAction());
    menu_Affichage->addAction(dockMaintenant->toggleViewAction());
    menu_Affichage->addAction(dockVisu->toggleViewAction());
    connect(graphicsViewProgrammes->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotHorizontalValueChanged(int)) );
    connect(graphicsViewProgrammes->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotVerticalValueChanged(int)) );
    m_handler = new XmlDefaultHandler(this, graphicsViewProgrammes);
    graphicsViewProgrammes->setScene( new QGraphicsScene(this) );
    m_command = "mencoder";
    //m_commandOptions = "$STREAM -oac mp3lame -lameopts abr:br=64 -af volnorm -ovc lavc -lavcopts vcodec=mpeg4:aspect=15/9:vbitrate=512 -vf crop=0:0,scale=352:288 -idx -ffourcc DIVX -ofps 25.0 -o $OUT";
    m_commandOptions = "\"$STREAM\" -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=300 -vf scale=-2:240 -ffourcc DIVX -fps 25 -ofps 25 -o \"$OUT\"";
    //repertoire->setText( QDir::homePath() );
    m_repertoire = QDir::homePath();
    m_currentDate = QDate::currentDate();
    m_heureDebutJournee = 6;
    m_demarrerEnIcone = false;
    m_formatNomFichier = "[%n]-%t-%j.%m%a.avi";
    //labelDate->setText( m_currentDate.toString("ddd dd MMM yyyy") );
    m_timerMinute = new QTimer(this);
    connect(m_timerMinute, SIGNAL(timeout()), this, SLOT(slotTimerMinute()));
    //desc->setFixedHeight(120);
    createTrayIcon();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotIconActivated(QSystemTrayIcon::ActivationReason)));
    QIcon icon = QIcon(":/images/images/tv.png");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    m_dialogProgrammes = new QDialog(this);
    m_uiProgrammes.setupUi(m_dialogProgrammes);
    connect(listeMaintenant, SIGNAL(itemClicked ( QListWidgetItem * )), this, SLOT(itemClique ( QListWidgetItem * )) );
    connect(listeMaintenant, SIGNAL(itemDoubleClicked( QListWidgetItem * )), this, SLOT(itemDoubleClicked( QListWidgetItem * )) );
    connect(listeSoiree, SIGNAL(itemClicked ( QListWidgetItem * )), this, SLOT(itemClique ( QListWidgetItem * )) );
    connect(listeSoiree, SIGNAL(itemDoubleClicked( QListWidgetItem * )), this, SLOT(itemDoubleClicked( QListWidgetItem * )) );
    connect(m_uiProgrammes.supprimer, SIGNAL(clicked()), this, SLOT(slotSupprimer()) );
    actionToggleFullScreen = new QAction(this);
    actionToggleFullScreen->setShortcut( Qt::Key_F6 );
    actionToggleFullScreen->setShortcutContext( Qt::ApplicationShortcut );
    connect(actionToggleFullScreen, SIGNAL(triggered()), this, SLOT(slotToggleFullScreen()) );
    addAction( actionToggleFullScreen );
    m_visu = new Visu(this, visu);
    litEnregistrements();
    trayIcon->show();
}
//
MainWindowImpl::~MainWindowImpl()
{
    delete m_dialogProgrammes;
}
void MainWindowImpl::slotToggleFullScreen()
{
#ifdef MAEMO
    setWindowState(windowState() ^ Qt::WindowFullScreen);
#else
    setGeometry(80, 60, width()==800 ? 672 : 800, height()==480 ? 396 : 480);
#endif
}

void MainWindowImpl::slotSupprimer()
{
    QTableWidgetItem *item = m_uiProgrammes.table->item(m_uiProgrammes.table->currentRow(), 0);
    if ( !item )
        return;
    Programme programme = item->data(Qt::UserRole).value<Programme>();
    if ( programme.process )
    {
        //kill(programme.process->pid(), SIGINT);
        programme.process->terminate();
    }
    if ( programme.timer )
    {
        programme.timer->stop();
        delete programme.timer;
    }
    m_uiProgrammes.table->removeRow( m_uiProgrammes.table->currentRow() );
    sauveEnregistrements();
}

void MainWindowImpl::ajouterProgramme(QString chaine, QString id, QDateTime debut, QDateTime fin, QString titre, QString desc, bool afficherDialogue)
{
    if (  fin < QDateTime::currentDateTime() )
    {
        if ( afficherDialogue )
            QMessageBox::warning(this, QString::fromUtf8("Enregistrement"), QString::fromUtf8("Emission finie"));
        return;
    }
    QDialog *dialog = new QDialog(this);
    Ui::Programme ui;
    ui.setupUi(dialog);
    ui.dateDebut->setDate( debut.date() );
    ui.heureDebut->setTime( debut.time() );
    ui.dateFin->setDate( fin.date() );
    ui.heureFin->setTime( fin.time() );
    ui.chaine->setText( chaine );
    ui.nomFichier->setText( m_formatNomFichier );
    ui.nomProgramme->setTitle( titre );
    ui.desc->setText( desc );
    if( numFreebox(id) == "NONE" )
    	ui.boutonAjouter->setDisabled( true );
    if ( !afficherDialogue || dialog->exec() == QDialog::Accepted )
    {
        QDateTime debut;
        debut.setDate(ui.dateDebut->date());
        debut.setTime(ui.heureDebut->time());
        if ( afficherDialogue && ui.checkBoxAvant->isChecked() )
            debut = debut.addSecs(ui.avant->text().toInt()*-60);
        QDateTime fin;
        fin.setDate(ui.dateFin->date());
        fin.setTime(ui.heureFin->time());
        if ( afficherDialogue && ui.checkBoxAjouter->isChecked() )
            fin = fin.addSecs(ui.ajouter->text().toInt()*60);
        QString nouveauTitre = ui.nomFichier->text();
        if ( afficherDialogue )
        {
            nouveauTitre.replace("%n", chaine);
            nouveauTitre.replace("%t", titre);
            nouveauTitre.replace("%a", debut.date().toString("yyyy"));
            nouveauTitre.replace("%m", debut.date().toString("MMM"));
            nouveauTitre.replace("%j", debut.date().toString("dd"));
            nouveauTitre.remove('/').remove('\\');
        }
        else
        {
            nouveauTitre = titre;
        }
        m_uiProgrammes.table->setRowCount(m_uiProgrammes.table->rowCount()+1);
        QTableWidgetItem *item1 = new QTableWidgetItem(chaine);
        item1->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_uiProgrammes.table->setItem(m_uiProgrammes.table->rowCount()-1, 0, item1);
        //
        QTableWidgetItem *item = new QTableWidgetItem(debut.toString(Qt::LocaleDate));
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_uiProgrammes.table->setItem(m_uiProgrammes.table->rowCount()-1, 1, item);
        //
        //
        item = new QTableWidgetItem(fin.toString(Qt::LocaleDate));
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_uiProgrammes.table->setItem(m_uiProgrammes.table->rowCount()-1, 2, item);
        //
        item = new QTableWidgetItem("");
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_uiProgrammes.table->setItem(m_uiProgrammes.table->rowCount()-1, 4, item);
        //
        int msecs = QDateTime::currentDateTime().secsTo( debut ) * 1000;
        msecs = qMax(0, msecs);
        Programme programme;
        programme.id = QDateTime::currentDateTime().toTime_t();
        programme.chaine = chaine;
        programme.numChaine = id;
        programme.debut = debut;
        programme.fin = fin;
        programme.etat = Attente;
        programme.timer = new QTimer(this);
        programme.process = 0;
        connect(programme.timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
        programme.timer->start(msecs);
        //
        item = new QTableWidgetItem(nouveauTitre);
        m_uiProgrammes.table->setItem(m_uiProgrammes.table->rowCount()-1, 3, item);
        //
        //QD << msecs << programme.id << "debut :" <<programme.debut;
        QVariant v;
        v.setValue( programme );
        item1->setData(Qt::UserRole, v );
    }
    delete dialog;
    sauveEnregistrements();
}

void MainWindowImpl::slotTimer()
{
    for (int i=0; i<m_uiProgrammes.table->rowCount(); i++)
    {
        QTableWidgetItem *item = m_uiProgrammes.table->item(i, 0);
        Programme programme = item->data(Qt::UserRole).value<Programme>();
        if ( sender() == programme.timer )
        {
            int msecs;
            QString options;
            switch ( programme.etat )
            {
            case Attente:
                msecs = QDateTime::currentDateTime().secsTo( programme.fin ) * 1000;
                programme.timer->start(msecs);
                m_uiProgrammes.table->item(i, 4)->setText("En cours");
                programme.etat = EnCours;
                m_uiProgrammes.table->item(i, 3)->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

                programme.process = new QProcess( this );
                options = m_commandOptions;
                options.replace("$STREAM", "rtsp://mafreebox.freebox.fr/freeboxtv/stream?id="+numFreebox(programme.numChaine));
                options.replace("$OUT", m_repertoire+m_uiProgrammes.table->item(i, 3)->text());
                connect(programme.process, SIGNAL(readyReadStandardError()), this, SLOT(slotReadyReadStandardError()));
                connect(programme.process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadyReadStandardOutput()));
                connect(programme.process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotFinished(int, QProcess::ExitStatus)));
                programme.process->start(m_command+" "+/*, QStringList(*/options/*)*/);
                //programme.process->start("mencoder -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=300 -vf scale=-2:400 -ffourcc DIVX -fps 25 -ofps 25 /home/jlbrd/Bienvenue.Chez.Les.Chtis.FRENCH.DVD.avi -o /home/jlbrd/essai.avi");
                QD << "debut" << m_command+" "+options;
                QD << "fin prevue :" << programme.debut.addMSecs(msecs);

                break;
            case EnCours:
                programme.timer->stop();
                QD << "fin" << programme.id << programme.fin.toString(Qt::LocaleDate);
                programme.etat = Termine;
                //kill(programme.process->pid(), SIGINT);
                programme.process->terminate();
                break;
            case Termine:
                break;
            }
            QVariant v;
            v.setValue( programme );
            item->setData(Qt::UserRole, v );
        }
    }
}


void MainWindowImpl::slotChoixRepertoire()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the project directory"),
                    m_repertoire,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    uiConfig.repertoire->setText( s );
}

void MainWindowImpl::litINI()
{
    //QSettings settings(QDir::homePath()+"/qmagneto.ini", QSettings::IniFormat);
    QSettings settings(cheminIni() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Options");
    m_command = settings.value("m_command", m_command).toString();
    m_commandOptions = settings.value("m_commandOptions", m_commandOptions).toString();
    m_repertoire = settings.value("m_repertoire", m_repertoire).toString();
    m_heureDebutJournee = settings.value("m_heureDebutJournee", m_heureDebutJournee).toInt();
    m_demarrerEnIcone = settings.value("m_demarrerEnIcone", m_demarrerEnIcone).toBool();
    m_formatNomFichier = settings.value("m_formatNomFichier", m_formatNomFichier).toString();
    m_nomFichierXML = settings.value("m_nomFichierXML", m_nomFichierXML).toString();
    settings.endGroup();
    settings.beginGroup("mainwindowstate");
#ifdef Q_OS_WIN32
    // Restores position, size and state for both normal and maximized/fullscreen state. Problems reported unter X11.
    // See Qt doc: Geometry: Restoring a Window's Geometry for details.
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray()); // Window geometry and state.
#else
    // Restores position, size and state including maximized/fullscreen.
    move(settings.value("pos", pos()).toPoint()); // Window position.
    resize(settings.value("size", size()).toSize()); // Window size.
    // Note: Yes, the window can be maximized and fullscreen!
    if (settings.value("maximized", isMaximized()).toBool()) // Window maximized.
        setWindowState(windowState() | Qt::WindowMaximized);
    if (settings.value("fullscreen", isFullScreen()).toBool()) // Window fullscreen.
        setWindowState(windowState() | Qt::WindowFullScreen);
#endif
    restoreState(settings.value("state", saveState()).toByteArray()); // Toolbar and DockWidget state.
    settings.endGroup();
}


void MainWindowImpl::sauveINI()
{
    //QSettings settings(QDir::homePath()+"/qmagneto.ini", QSettings::IniFormat);
    QSettings settings(cheminIni() + "qmagneto.ini", QSettings::IniFormat);

    settings.beginGroup("Options");
    settings.setValue("m_command", m_command);
    settings.setValue("m_commandOptions", m_commandOptions);
    settings.setValue("m_repertoire", m_repertoire);
    settings.setValue("m_heureDebutJournee", m_heureDebutJournee);
    settings.setValue("m_demarrerEnIcone", m_demarrerEnIcone);
    settings.setValue("m_formatNomFichier", m_formatNomFichier);
    settings.setValue("m_nomFichierXML", m_nomFichierXML);
    settings.endGroup();
    //
    settings.beginGroup("mainwindowstate");
    if (!isMinimized() && !isMaximized() && !isFullScreen())
    {
        settings.setValue("pos", pos());
        settings.setValue("size", size());
    }
    settings.setValue("maximized", isMaximized());
    settings.setValue("fullscreen", isFullScreen());
    settings.setValue("geometry", saveGeometry()); // Window geometry and state (only needed for Windows!).
    settings.setValue("state", saveState()); // Toolbar and DockWidget state.
    settings.endGroup();
}


void MainWindowImpl::slotReadyReadStandardError()
{
    //QD << "slotReadyReadStandardError" << ((QProcess *)sender())->readAllStandardError();
}


void MainWindowImpl::slotReadyReadStandardOutput()
{
    //QD << "slotReadyReadStandardOutput" << ((QProcess *)sender())->readAllStandardOutput();
}


void MainWindowImpl::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QD << "slotFinished";
    QProcess *process = ((QProcess *)sender());
    for (int i=0; i<m_uiProgrammes.table->rowCount(); i++)
    {
        QTableWidgetItem *item = m_uiProgrammes.table->item(i, 0);
        Programme programme = item->data(Qt::UserRole).value<Programme>();
        if ( process == programme.process )
        {
            programme.process = 0;
            programme.timer->stop();
            delete programme.timer;
            programme.timer = 0;
            m_uiProgrammes.table->item(i, 4)->setText(QString::fromUtf8("Fini") );
            programme.etat = Termine;
            QVariant v;
            v.setValue( programme );
            item->setData(Qt::UserRole, v );
        }
    }
    process->deleteLater();
    QD << "Finished";
}


void MainWindowImpl::litProgrammeTV()
{
    if ( !QFile::exists(m_nomFichierXML) )
    {
        QMessageBox::warning(this, QString::fromUtf8("Fichier XML"), QString::fromUtf8("Le fichier XML des programmes n'existe pas."));
        return;
    }
    labelDate->setText( m_currentDate.toString("dddd dd MMM yyyy") );
    m_timerMinute->stop();
    m_handler->setDate(m_currentDate);
    m_handler->setHeureDebutJournee( m_heureDebutJournee );
    m_handler->init();
    QXmlSimpleReader xmlReader;
    QFile file(m_nomFichierXML);
    QXmlInputSource *source = new QXmlInputSource(&file);
    xmlReader.setContentHandler(m_handler);
    xmlReader.setErrorHandler(m_handler);
    bool ok = xmlReader.parse(source);
    delete source;

    if (!ok)
        QD << "Parsing failed.";
    else
        m_handler->draw();
    slotTimerMinute();
    if ( QDate::currentDate() == m_currentDate )
    {
        int msecs = 60000;
        //QDateTime::currentDateTime().secsTo( debut ) * 1000;
        msecs = qMax(0, msecs);

        if ( QTime::currentTime().minute() != 0 )
            msecs = QTime::currentTime().msecsTo( QTime(QTime::currentTime().hour(), QTime::currentTime().minute()+1) );
        m_timerMinute->start(msecs);
    }
}


void MainWindowImpl::init()
{
    QHeaderView *header = m_uiProgrammes.table->horizontalHeader();
    header->resizeSection( 0, 90 );
    header->resizeSection( 1, 110 );
    header->resizeSection( 2, 110 );
    header->resizeSection( 3, 270 );
    header->resizeSection( 4, 90 );
    m_uiProgrammes.table->verticalHeader()->hide();
    //graphicsViewProgrammes->setSceneRect(graphicsViewProgrammes->rect());
    litINI();
    litProgrammeTV();
}


void MainWindowImpl::resizeEvent(QResizeEvent * event)
{
    QMainWindow::resizeEvent( event );
    visu->setFixedWidth( visu->height() );
}
//
void MainWindowImpl::slotHorizontalValueChanged(int value)
{
    m_handler->deplaceChaines( value );
}
void MainWindowImpl::slotVerticalValueChanged(int value)
{
    m_handler->deplaceHeures( value );
}



void MainWindowImpl::on_boutonJourAvant_clicked()
{
    m_currentDate = m_currentDate.addDays(-1);
    //labelDate->setText( m_currentDate.toString("ddd dd MMM yyyy") );
    litProgrammeTV();
}

void MainWindowImpl::on_boutonJourApres_clicked()
{
    m_currentDate = m_currentDate.addDays(1);
    //labelDate->setText( m_currentDate.toString("ddd dd MMM yyyy") );
    litProgrammeTV();
}





void MainWindowImpl::on_aujourdhui_clicked()
{
    m_currentDate = QDate::currentDate();
    //labelDate->setText( m_currentDate.toString("ddd dd MMM yyyy") );
    litProgrammeTV();
}

void MainWindowImpl::slotTimerMinute()
{
    //QD << QTime::currentTime();
    m_handler->posLigneHeureCourante();
    // Liste des programmes de la soiree
    listeSoiree->clear();
    foreach(ProgrammeTV prog, m_handler->programmesSoiree() )
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/images/"+prog.channelName+".png"), prog.title);
        QVariant v;
        v.setValue( prog );
        item->setData(Qt::UserRole, v );
        listeSoiree->addItem( item );
    }
    // Liste des programmes maintenant
    listeMaintenant->clear();
    foreach(ProgrammeTV prog, m_handler->programmesMaintenant() )
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/images/"+prog.channelName+".png"), prog.title);
        listeMaintenant->addItem( item );
        QVariant v;
        v.setValue( prog );
        item->setData(Qt::UserRole, v );
    }
    //QCoreApplication::processEvents();
    m_timerMinute->start(60000);
}


void MainWindowImpl::itemClique(GraphicsRectItem *item)
{
    if ( item != 0 )
    {
        listeMaintenant->setCurrentRow(-1);
        listeSoiree->setCurrentRow(-1);
    }
    foreach(GraphicsRectItem *it, m_handler->listeItemProgrammes())
    {
        if (it==item)
        {
            it->setActif( true );
            ProgrammeTV prog = it->data(0).value<ProgrammeTV>();
            afficheDescription( prog );
        }
        else
            it->setActif( false );
    }
}


void MainWindowImpl::on_action_Quitter_triggered()
{
    sauveINI();
    qApp->quit();
}

void MainWindowImpl::on_action_Configurer_triggered()
{
    QDialog *dialog = new QDialog(this);
    uiConfig.setupUi(dialog);
    uiConfig.command->setText( m_command );
    uiConfig.commandOptions->setText( m_commandOptions );
    uiConfig.demarrerEnIcone->setChecked( m_demarrerEnIcone );
    uiConfig.repertoire->setText( m_repertoire );
    uiConfig.nomFichier->setText( m_formatNomFichier );
    uiConfig.nomFichierXML->setText( m_nomFichierXML );
    connect(uiConfig.choixRepertoire, SIGNAL(clicked()), this, SLOT(slotChoixRepertoire()) );
    connect(uiConfig.choixFichierXML, SIGNAL(clicked()), this, SLOT(slotFichierXML()) );
    if ( dialog->exec() == QDialog::Accepted )
    {
        m_command = uiConfig.command->text();
        m_commandOptions = uiConfig.commandOptions->text();
        m_demarrerEnIcone = uiConfig.demarrerEnIcone->isChecked();
        m_repertoire = uiConfig.repertoire->text();
        m_formatNomFichier = uiConfig.nomFichier->text();
        m_nomFichierXML = uiConfig.nomFichierXML->text();
    }
    delete dialog;
    sauveINI();
}

void MainWindowImpl::on_soiree_clicked()
{
    m_handler->soiree();
}
void MainWindowImpl::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void MainWindowImpl::createTrayIcon()
{
    restoreAction = new QAction(tr("&Restaurer"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    quitAction = new QAction(tr("&Quitter"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(on_action_Quitter_triggered()));
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip("qmagneto");
}
void MainWindowImpl::slotIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        //case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        showNormal();
        break;
        //case QSystemTrayIcon::MiddleClick:
        //break;
        //default:
        ;
    }
}

void MainWindowImpl::on_action_Programmes_triggered()
{
    m_dialogProgrammes->show();
}

void MainWindowImpl::itemClique(QListWidgetItem *item)
{
    itemClique((GraphicsRectItem  *)0);
    if ( item->listWidget() == listeSoiree )
        listeMaintenant->setCurrentRow(-1);
    else
        listeSoiree->setCurrentRow(-1);
    ProgrammeTV prog = item->data(Qt::UserRole).value<ProgrammeTV>();
    afficheDescription( prog );
}


void MainWindowImpl::slotFichierXML()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Fichier XML"),
                m_nomFichierXML,
                tr("Fichiers XML (*.xml *.XML *)"));
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    uiConfig.nomFichierXML->setText( s );
}

void MainWindowImpl::itemDoubleClicked(QListWidgetItem *item)
{
    itemClique(item);
    ProgrammeTV prog = item->data(Qt::UserRole).value<ProgrammeTV>();
    if ( !prog.start.isValid() )
        return;
    QString desc;
    if( prog.desc.count() )
    	desc = prog.desc.first();
    ajouterProgramme(prog.channelName, prog.channel, prog.start, prog.stop, prog.title, desc);
}


QString MainWindowImpl::cheminIni()
{
    static QString path;
    if (!path.isEmpty()) return path;

    // if we havn't yet done so, determine the full db file name and make sure the directory exists
    // determine path for application data dirs
#ifdef Q_OS_WIN32
    wchar_t buf[MAX_PATH];
    if (!SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, buf))
        path = QString::fromUtf16((ushort *)buf)+"/";
    else
        path = QDir::homePath()+"/Application Data/"; // this shouldn't happen
#else
    path = QDir::homePath()+"/";
#endif

    // create subdir
    QDir dir(path);
#ifdef Q_OS_WIN32
    dir.mkdir("QMagneto");
    path += "QMagneto/";
#else
    dir.mkdir(".qmagneto");
    path += ".qmagneto/";
#endif
    return path;
}


void MainWindowImpl::litEnregistrements()
{
    QSettings settings(cheminIni() + "enregistrements.ini", QSettings::IniFormat);
    settings.beginGroup("Nombre");
    int nombre = settings.value("nombre", 0).toInt();
    settings.endGroup();
    for (int i=0; i<nombre; i++)
    {
        settings.beginGroup("Enregistrements"+QString::number(i));
        QString chaine = settings.value("chaine", "").toString();
        QString numChaine = settings.value("numChaine", "").toString();
        QDateTime debut = QDateTime::fromTime_t( settings.value("debut", "").toInt() );
        QDateTime fin = QDateTime::fromTime_t( settings.value("fin", "").toInt() );
        QString nomFichier = settings.value("nomFichier", "").toString();
        ajouterProgramme(chaine, numChaine, debut, fin, nomFichier, "", false);
        settings.endGroup();
    }

}


void MainWindowImpl::sauveEnregistrements()
{
    QSettings settings(cheminIni() + "enregistrements.ini", QSettings::IniFormat);
    settings.beginGroup("Nombre");
    settings.setValue("nombre", m_uiProgrammes.table->rowCount());
    settings.endGroup();
    for (int i=0; i<m_uiProgrammes.table->rowCount(); i++)
    {
        settings.beginGroup("Enregistrements"+QString::number(i));
        QTableWidgetItem *item = m_uiProgrammes.table->item(i, 0);
        Programme prog = item->data(Qt::UserRole).value<Programme>();
        settings.setValue("chaine", prog.chaine);
        settings.setValue("numChaine", prog.numChaine);
        settings.setValue("debut", prog.debut.toTime_t());
        settings.setValue("fin", prog.fin.toTime_t());
        settings.setValue("nomFichier", m_uiProgrammes.table->item(i, 3)->text());
        settings.endGroup();
    }
}


void MainWindowImpl::play(QString nomChaine)
{
    QString id;
    QList<ChaineTV> listeChaines = m_handler->chaines();
    for (int i=0; i<listeChaines.count(); i++)
    {
        ChaineTV ch = listeChaines.at(i);
        if ( ch.name == nomChaine )
        {
            id = ch.id;
            break;
        }
    }
    QD<<id;
    QPixmap logo = QPixmap(":/images/images/"+nomChaine+".png").scaled(labelLogo->size(),
                   Qt::KeepAspectRatio);
    logo = logo.copy(1, 1, logo.width()-1, logo.height()-1);
    labelLogo->setPixmap( logo );
    m_idChaineVisu = id;
    m_visu->stopMPlayer();
    if ( !m_visu->startMPlayer("rtsp://mafreebox.freebox.fr/freeboxtv/stream?id="+id) )
    {
        labelLogo->setPixmap( 0 );
        m_idChaineVisu = "";
    }
}

void MainWindowImpl::on_stop_clicked()
{
    labelLogo->setPixmap( 0 );
    m_idChaineVisu = "";
    m_visu->stopMPlayer();
}

void MainWindowImpl::on_record_clicked()
{
    if ( m_idChaineVisu.isEmpty() )
        return;
    ProgrammeTV p;
    foreach(ProgrammeTV prog, m_handler->programmesMaintenant() )
    {
        if ( prog.channel == m_idChaineVisu )
        {
            p = prog;
            break;
        }
    }
    if ( !p.start.isValid() )
        return;
    QString desc;
    if( p.desc.count() )
    	desc = p.desc.first();
    ajouterProgramme(p.channelName, p.channel, p.start, p.stop, p.title, desc);
}

void MainWindowImpl::afficheDescription(ProgrammeTV prog)
{
    int secs = prog.start.time().secsTo( prog.stop.time() );
    QString description = prog.desc.join("<br>");
    QString critique;
    if( description.contains("Critique") )
    {
    	critique = description.section("Critique :", 1, 1);
    	description = description.section("Critique :", 0, 0);
   	}
    QString d = "<html>";
    d = d + "<table style=\"text-align: left; width: 1%;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">";
    d = d + "<tbody><tr>";
    d = d + "<td><img style=\"vertical-align: top;\" src=\":/images/images/"+prog.channelName+".png\"></td>";
    //d += "<td  style=\"width: 1%; vertical-align: middle;\">";
    d = d +"<td style=\"width: 1%; vertical-align: top;\">"
        +"<span style=\"font-weight: bold;\">"
        +prog.title
        +"</span> " + prog.subTitle
        +"<table><tbody><tr><td>"+prog.start.toString("hh:mm")+"-"+prog.stop.toString("hh:mm")
        +" ("+QTime(0,0).addSecs(secs).toString("hh:mm")+")</td></tr></tbody></table>"
        + "<td  style=\"width: 99%; vertical-align: top; text-align: left;\">";
    for (int i=0; i<prog.star.section("/", 0, 0).toInt(); i++)
        d = d + "<img style=\"vertical-align: middle;\" src=\":/images/images/star.png\">";
    d = d + "</td></tr>";
    d += "</tbody></table><br>";
    d+="</td>";
    d += "<span style=\"font-weight: bold;\">"+QString::fromUtf8("Catégorie : ")+"</span>"+prog.category.join("/");
    d += "<br>";
    d += description;
    if( !critique.isEmpty() )
    	d += "<br><span style=\"font-weight: bold;\">"+QString::fromUtf8("Critique : ")+"</span>"+critique+"</span>";
    d += "</html>";
    desc->setText( d );
//QApplication::clipboard()->setText(d);
}


QString MainWindowImpl::numFreebox(QString s)
{
	s.replace("C1.telepoche.com", "NONE");
	s.replace("C122.telepoche.com", "679");
	s.replace("C167.telepoche.com", "372");
	s.replace("C168.telepoche.com", "374");
	s.replace("C169.telepoche.com", "382");
	s.replace("C170.telepoche.com", "226");
	s.replace("C193.telepoche.com", "678");
	s.replace("C194.telepoche.com", "400");
	s.replace("C195.telepoche.com", "677");
	s.replace("C2.telepoche.com", "201");
	s.replace("C28.telepoche.com", "376");
	s.replace("C3.telepoche.com", "202");
	s.replace("C38.telepoche.com", "NONE");
	s.replace("C4.telepoche.com", "NONE");
	s.replace("C5.telepoche.com", "204");
	s.replace("C6.telepoche.com", "NONE");
	s.replace("C7.telepoche.com", "203");
	s.replace("C9.telepoche.com", "497");
	return s;
}


void MainWindowImpl::on_actionA_propos_triggered()
{
    QDialog about;
    Ui::About ui;
    ui.setupUi( &about );
    ui.version->setText( tr("Version %1").arg(VERSION) );
    about.exec();
}

void MainWindowImpl::on_actionA_propos_de_Qt_triggered()
{
	QMessageBox::aboutQt( this );
}

