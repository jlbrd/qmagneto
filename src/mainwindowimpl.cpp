#include "mainwindowimpl.h"
#include "ui_programme.h"
#include "listmaintenant.h"
#include "canauximpl.h"
#include "configimpl.h"
#include "programmeimpl.h"
#include "ui_about.h"
#include <QHeaderView>
#include <QTimer>
#include <QFileDialog>
#include <QProcess>
#include <QSettings>
#include <QScrollBar>
#include <QListWidget>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QClipboard>
#include <QHttp>

#ifdef Q_OS_WIN32
#include <shlobj.h>
#endif
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
#define VERSION "0.5-8"
//
MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f)
        : QMainWindow(parent, f)
{
    setupUi(this);
    menu_Affichage->addAction(dockDesc->toggleViewAction());
    menu_Affichage->addAction(dockSoiree->toggleViewAction());
    menu_Affichage->addAction(dockMaintenant->toggleViewAction());
    connect(graphicsViewProgrammes->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotHorizontalValueChanged(int)) );
    connect(graphicsViewProgrammes->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotVerticalValueChanged(int)) );
    m_handler = new XmlDefaultHandler(this, graphicsViewProgrammes);
    graphicsViewProgrammes->setScene( new QGraphicsScene(this) );
    m_command = "mencoder";
    //m_commandOptions = "$STREAM -oac mp3lame -lameopts abr:br=64 -af volnorm -ovc lavc -lavcopts vcodec=mpeg4:aspect=15/9:vbitrate=512 -vf crop=0:0,scale=352:288 -idx -ffourcc DIVX -ofps 25.0 -o $OUT";
    //m_commandOptions = "\"$STREAM\" -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=300 -vf scale=-2:240 -ffourcc DIVX -fps 25 -ofps 25 -o \"$OUT\"";
    //m_commandOptions = "--intf dummy \"$STREAM\" :sout=#transcode{vcodec=h264,vb=2048,scale=1,acodec=mpga,ab=192,channels=2}:duplicate{dst=std{access=file,mux=ts,dst=\"'$OUT.avi'\"}}";
    m_commandOptions = "\"$STREAM\"  -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=1500 -vf scale=-2:400 -ffourcc DIVX -fps 25 -ofps 25 -o  \"$OUT\"";
    m_commandLecture = "vlc";
    m_commandLectureOptions = "\"$STREAM\"";
    m_repertoire = QDir::homePath();
    m_currentDate = QDate::currentDate();
    m_heureDebutJournee = 6;
    m_demarrerEnIcone = false;
    m_formatNomFichier = "[%n]-%t-%j.%m%a";
    m_depuisFichier = false;
    m_comboURL = 0;
    m_timerMinute = new QTimer(this);
    connect(m_timerMinute, SIGNAL(timeout()), this, SLOT(slotTimerMinute()));
    m_timer3Seconde = new QTimer(this);
    connect(m_timer3Seconde, SIGNAL(timeout()), this, SLOT(slotTimer3Seconde()));
    m_timer3Seconde->start( 3000 );
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
#ifdef Q_WS_WIN
        programme.process->kill();
#else
        programme.process->terminate();
#endif
    }
    if ( programme.timer )
    {
        programme.timer->stop();
        delete programme.timer;
    }
    m_uiProgrammes.table->removeRow( m_uiProgrammes.table->currentRow() );
    sauveEnregistrements();
}

void MainWindowImpl::ajouterProgramme(ProgrammeTV prog, QString titre, bool afficherDialogue, Type type)
{
    ProgrammeImpl *programmeImpl = new ProgrammeImpl(this, prog, m_formatNomFichier);
    if ( prog.stop < QDateTime::currentDateTime()
            || numBox(prog.channel ).contains("NONE") )
    {
        programmeImpl->boutonAjouter->setDisabled( true );
        programmeImpl->boutonRegarder->setDisabled( true );
        programmeImpl->labelInfo->setText(QString::fromUtf8("Impossible d'enregistrer ou regarder, le canal n'est pas configuré"));
    }
    else
    {
        programmeImpl->labelInfo->setText("");
    }
    programmeImpl->setType(type);
    if ( !afficherDialogue || programmeImpl->exec() == QDialog::Accepted )
    {
        QDateTime debut;
        debut.setDate(programmeImpl->dateDebut->date());
        debut.setTime(programmeImpl->heureDebut->time());
        if ( afficherDialogue && programmeImpl->checkBoxAvant->isChecked() )
            debut = debut.addSecs(programmeImpl->avant->text().toInt()*-60);
        QDateTime fin;
        fin.setDate(programmeImpl->dateFin->date());
        fin.setTime(programmeImpl->heureFin->time());
        if ( afficherDialogue && programmeImpl->checkBoxAjouter->isChecked() )
            fin = fin.addSecs(programmeImpl->ajouter->text().toInt()*60);
        QString nouveauTitre = programmeImpl->nomFichier->text();
        if ( afficherDialogue )
        {
            nouveauTitre.replace("%n", prog.channelName);
            nouveauTitre.replace("%t", prog.title);
            nouveauTitre.replace("%a", prog.start.date().toString("yyyy"));
            nouveauTitre.replace("%m", prog.start.date().toString("MMM"));
            nouveauTitre.replace("%j", prog.start.date().toString("dd"));
            nouveauTitre.remove('/').remove('\\');
        }
        else
        {
            nouveauTitre = titre;
        }
        m_uiProgrammes.table->setRowCount(m_uiProgrammes.table->rowCount()+1);
        QTableWidgetItem *item1 = new QTableWidgetItem(prog.channelName);
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
        QString t;
        if ( programmeImpl->type() == Enregistrement )
            t = "Enregistrement programmÃ©";
        else
            t = "Lecture programmÃ©e";
        item = new QTableWidgetItem(t);
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_uiProgrammes.table->setItem(m_uiProgrammes.table->rowCount()-1, 4, item);
        //
        int msecs = QDateTime::currentDateTime().secsTo( debut ) * 1000;
        msecs = qMax(0, msecs);
        Programme programme;
        programme.id = QDateTime::currentDateTime().toTime_t();
        programme.chaine = prog.channelName;
        programme.numChaine = prog.channel;
        programme.debut = debut;//prog.start;
        programme.fin = fin;//prog.stop;
        programme.etat = Attente;
        programme.timer = new QTimer(this);
        programme.process = 0;
        programme.type = programmeImpl->type();
        connect(programme.timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
        programme.timer->start(msecs);
        //
        item = new QTableWidgetItem(nouveauTitre);
        m_uiProgrammes.table->setItem(m_uiProgrammes.table->rowCount()-1, 3, item);
        //
        QD << msecs << programme.id << "debut :" <<programme.debut.toString() << "fin :" << programme.fin.toString();
        QVariant v;
        v.setValue( programme );
        item1->setData(Qt::UserRole, v );
    }
    delete programmeImpl;
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
                switch ( programme.type )
                {
                case Enregistrement:
                    msecs = QDateTime::currentDateTime().secsTo( programme.fin ) * 1000;
                    programme.timer->start(msecs);
                    m_uiProgrammes.table->item(i, 4)->setText("Enregistrement en cours");
                    programme.etat = EnCours;
                    m_uiProgrammes.table->item(i, 3)->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

                    programme.process = new QProcess( this );
                    options = m_commandOptions;
                    options.replace("$STREAM", numBox(programme.numChaine));
                    options.replace("$OUT", m_repertoire+m_uiProgrammes.table->item(i, 3)->text().replace("\"", " " ).replace("'"," ") );
                    connect(programme.process, SIGNAL(readyReadStandardError()), this, SLOT(slotReadyReadStandardError()));
                    connect(programme.process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadyReadStandardOutput()));
                    connect(programme.process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotFinished(int, QProcess::ExitStatus)));
                    programme.process->start(m_command+" "+options);
                    QD << "debut" << m_command+" "+options;
                    QD << "fin prevue :" << QDateTime::currentDateTime().addMSecs(msecs);
                    break;
                case Lecture:
                    m_uiProgrammes.table->item(i, 4)->setText("En lecture");
                    programme.etat = Termine;
                    m_uiProgrammes.table->item(i, 3)->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
                    options = m_commandLectureOptions;
                    options.replace("$STREAM", numBox(programme.numChaine));
                    QD << m_commandLecture << options;
                    QProcess::startDetached(m_commandLecture+" "+options);
                    break;
                }
                break;
            case EnCours:
                programme.timer->stop();
                QD << "fin" << programme.id << programme.fin.toString(Qt::LocaleDate);
                programme.etat = Termine;
                //kill(programme.process->pid(), SIGINT);
#ifdef Q_WS_WIN
                programme.process->kill();
#else
                programme.process->terminate();
#endif
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

void MainWindowImpl::litINI()
{
    QSettings settings(cheminIni() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Options");
    m_command = settings.value("m_command", m_command).toString();
    m_commandOptions = settings.value("m_commandOptions", m_commandOptions).toString();
    m_commandLecture = settings.value("m_commandLecture", m_commandLecture).toString();
    m_commandLectureOptions = settings.value("m_commandLectureOptions", m_commandLectureOptions).toString();
    m_repertoire = settings.value("m_repertoire", m_repertoire).toString();
    m_heureDebutJournee = settings.value("m_heureDebutJournee", m_heureDebutJournee).toInt();
    m_demarrerEnIcone = settings.value("m_demarrerEnIcone", m_demarrerEnIcone).toBool();
    m_formatNomFichier = settings.value("m_formatNomFichier", m_formatNomFichier).toString();
    m_nomFichierXML = settings.value("m_nomFichierXML", m_nomFichierXML).toString();
    m_comboURL = settings.value("m_comboURL", m_comboURL).toInt();
    m_depuisFichier = settings.value("m_depuisFichier", m_depuisFichier).toBool();
    m_handler->setProgWidth( settings.value("m_progWidth", 180.0).toDouble() );
    m_handler->setProgHeight( settings.value("m_progHeight", 60.0).toDouble() );
    m_handler->setHourHeight( settings.value("m_hourHeight", 25.0).toDouble() );
	QFont font;  
	font.fromString( 
			settings.value("m_programFont", QPainter().font().toString()).toString()
	);
	GraphicsRectItem::setProgramFont( font );
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
    QSettings settings(cheminIni() + "qmagneto.ini", QSettings::IniFormat);

    settings.beginGroup("Options");
    settings.setValue("m_command", m_command);
    settings.setValue("m_commandOptions", m_commandOptions);
    settings.setValue("m_commandLecture", m_commandLecture);
    settings.setValue("m_commandLectureOptions", m_commandLectureOptions);
    settings.setValue("m_repertoire", m_repertoire);
    settings.setValue("m_heureDebutJournee", m_heureDebutJournee);
    settings.setValue("m_demarrerEnIcone", m_demarrerEnIcone);
    settings.setValue("m_formatNomFichier", m_formatNomFichier);
    settings.setValue("m_nomFichierXML", m_nomFichierXML);
    settings.setValue("m_comboURL", m_comboURL);
    settings.setValue("m_depuisFichier", m_depuisFichier);
    settings.setValue("m_progWidth", m_handler->progWidth());
    settings.setValue("m_progHeight", m_handler->progHeight());
    settings.setValue("m_hourHeight", m_handler->hourHeight());
    settings.setValue("m_programFont", GraphicsRectItem::programFont());
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


void MainWindowImpl::slotFinished(int, QProcess::ExitStatus)
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


#include <QTime>
void MainWindowImpl::litProgrammeTV()
{
    QTime t;
    t.start();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_timerMinute->stop();
    m_handler->setDate(m_currentDate);
    m_handler->setHeureDebutJournee( m_heureDebutJournee );
    m_handler->init();
    if ( !m_handler->readFromDB() )
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, QString::fromUtf8("Fichier XML"), QString::fromUtf8("Le fichier XML du guide TV est trop ancien ou absent.\nVeuillez le mettre a jour."));
        on_action_Configurer_triggered();
        return;
    }
    QDate minimumDate = m_handler->minimumDate();
    dateEdit->setMinimumDate(minimumDate);
    if ( m_currentDate == m_handler->minimumDate() )
        boutonJourAvant->setDisabled(true);
    else
        boutonJourAvant->setDisabled(false);
    QDate maximumDate = m_handler->maximumDate();
    dateEdit->setMaximumDate(maximumDate);
    if ( m_currentDate == m_handler->maximumDate() )
        boutonJourApres->setDisabled(true);
    else
        boutonJourApres->setDisabled(false);

    //m_handler->draw();
    QD << "elapsed" << t.elapsed();
    slotTimerMinute();
    if ( QDate::currentDate() == m_currentDate )
    {
        int msecs = 60000;
        msecs = qMax(0, msecs);

        if ( QTime::currentTime().minute() != 0 )
            msecs = QTime::currentTime().msecsTo( QTime(QTime::currentTime().hour(), QTime::currentTime().minute()+1) );
        m_timerMinute->start(msecs);
    }
    QApplication::restoreOverrideCursor();
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
    litINI();
    dateEdit->setDate( m_currentDate );
}


void MainWindowImpl::resizeEvent(QResizeEvent * event)
{
    QMainWindow::resizeEvent( event );
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
    dateEdit->setDate( m_currentDate );
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
}

void MainWindowImpl::on_boutonJourApres_clicked()
{
    m_currentDate = m_currentDate.addDays(1);
    dateEdit->setDate( m_currentDate );
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
}

void MainWindowImpl::on_maintenant_clicked()
{
    m_currentDate = QDate::currentDate();
    dateEdit->setDate( m_currentDate );
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
    m_handler->centreMaintenant();
}

void MainWindowImpl::slotTimerMinute()
{
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
    dockSoiree->setWindowTitle(QString::fromUtf8("Soirée du ")+m_currentDate.toString("dddd dd MMM yyyy"));
    // Liste des programmes maintenant
    listeMaintenant->clear();
    foreach(ProgrammeTV prog, m_handler->programmesMaintenant() )
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/images/"+prog.channelName+".png"), prog.title);
        listeMaintenant->addItem( item );
        QVariant v;
        v.setValue( prog );
        item->setData(Qt::UserRole, v );

        QPixmap pix(listeMaintenant->visualItemRect(item).width(), listeMaintenant->visualItemRect(item).height());
        pix.fill(Qt::white);
        QPainter painter(&pix);
        painter.drawRect(pix.rect());
        painter.setBrush( QColor(Qt::blue).light(180) );
        float f = (float)prog.start.secsTo( prog.stop ) / (float)prog.start.secsTo( QDateTime::currentDateTime() );
        painter.drawRect(0,0, (float)listeMaintenant->visualItemRect(item).width()/f, pix.height());
        painter.end();
        item->setBackground( QBrush(pix) );
    }
    m_timerMinute->start(60000);
}

void MainWindowImpl::slotTimer3Seconde()
{
    int num = 0;
    for (int i=0; i<m_uiProgrammes.table->rowCount(); i++)
    {
        QTableWidgetItem *item = m_uiProgrammes.table->item(i, 0);
        Programme programme = item->data(Qt::UserRole).value<Programme>();
        switch ( programme.etat )
        {
        case EnCours:
            num++;
            break;
        }
    }
    static bool flags = true;
    QIcon icon;
    if ( flags || num == 0 || num > 6 )
    {
        icon = QIcon(":/images/images/tv.png");
    }
    else
    {
        icon = QIcon(":/images/images/chiffre" + QString::number(num) + ".png");
    }
    trayIcon->setIcon(icon);
    flags = !flags;
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
            desc->clear();
            desc->setText( afficheDescription( prog ) );
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
    ConfigImpl *dialog = new ConfigImpl(this);
    dialog->command->setText( m_command );
    dialog->commandOptions->setText( m_commandOptions );
    dialog->commandLecture->setText( m_commandLecture );
    dialog->commandLectureOptions->setText( m_commandLectureOptions );
    dialog->demarrerEnIcone->setChecked( m_demarrerEnIcone );
    dialog->repertoire->setText( m_repertoire );
    dialog->nomFichier->setText( m_formatNomFichier );
    dialog->nomFichierXML->setText( m_nomFichierXML );
    dialog->depuisFichier->setChecked( m_depuisFichier );
    dialog->comboURL->setCurrentIndex( m_comboURL );
    dialog->heureDebut->setValue( m_heureDebutJournee );
    dialog->programWidth->setValue( m_handler->progWidth() );
    dialog->programHeight->setValue( m_handler->progHeight() );
	QFontDatabase db;
	dialog->comboFont->addItems( db.families() );
	dialog->comboFont->setCurrentIndex( dialog->comboFont->findText( GraphicsRectItem::programFont().family() ) );
	dialog->fontSize->setValue( GraphicsRectItem::programFont().pointSize() );
    if ( dialog->exec() == QDialog::Accepted )
    {
        m_command = dialog->command->text();
        m_commandOptions = dialog->commandOptions->text();
        m_commandLecture = dialog->commandLecture->text();
        m_commandLectureOptions = dialog->commandLectureOptions->text();
        m_demarrerEnIcone = dialog->demarrerEnIcone->isChecked();
        m_repertoire = dialog->repertoire->text();
        if ( !m_repertoire.endsWith("/") )
            m_repertoire += "/";
        m_formatNomFichier = dialog->nomFichier->text();
        m_nomFichierXML = dialog->nomFichierXML->text();
        m_depuisFichier = dialog->depuisFichier->isChecked();
        m_comboURL = dialog->comboURL->currentIndex();
        m_heureDebutJournee = dialog->heureDebut->value();
	    m_handler->setProgWidth( dialog->programWidth->value() );
	    m_handler->setProgHeight( dialog->programHeight->value() );
	    GraphicsRectItem::setProgramFont( 
	    	QFont(dialog->comboFont->currentText(), 
	    	dialog->fontSize->value() ) 
	    );
        sauveINI();
        litProgrammeTV();
    }
    delete dialog;
}
void MainWindowImpl::populateDB(bool depuisFichier, QString nomFichierXML)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if ( depuisFichier )
    {
        m_nomFichierXML = nomFichierXML;
    }
    else
    {
        QProcess process;
        QD << "recuperation du fichier " + nomFichierXML;
        process.start("wget", QStringList() << "-O" << QDir::tempPath()+"/fichier.zip" << nomFichierXML);
        process.waitForFinished(-1);
        if ( process.exitCode() )
        {
            QMessageBox::warning(this, QString::fromUtf8("Fichier XML"), QString::fromUtf8("Impossible de tÃ©lÃ©charger le fichier. Vous devez Ãªtre connectÃ© Ã  Internet et disposer de la commande wget."));
            process.terminate();
            QApplication::restoreOverrideCursor();
            return;
        }
        process.terminate();
        QD << "decompression de " + QDir::tempPath()+"/fichier.zip";
        process.start("unzip", QStringList() << "-o" << QDir::tempPath()+"/fichier.zip" << "-d" << QDir::tempPath());
        process.waitForFinished(-1);
        if ( process.exitCode() )
        {
            QMessageBox::warning(this, QString::fromUtf8("Fichier XML"), QString::fromUtf8("Impossible de tÃ©lÃ©charger le fichier. Vous devez disposer de la commande unzip."));
            QApplication::restoreOverrideCursor();
            return;
        }
        nomFichierXML = QDir::tempPath()+"/"+nomFichierXML.section("/", -1, -1).section(".", 0, 0)+".xml";
        QD << "analyse de " + nomFichierXML;
        process.terminate();
        if ( !QFile::exists(nomFichierXML) )
        {
            QMessageBox::warning(this, QString::fromUtf8("Fichier XML"), QString::fromUtf8("Un problÃ¨me est survenu lors de la rÃ©cupÃ©ration du fichier."));
            QApplication::restoreOverrideCursor();
            return;
        }
    }
    QXmlSimpleReader xmlReader;
    QFile file(nomFichierXML);
    QXmlInputSource *source = new QXmlInputSource(&file);
    xmlReader.setContentHandler(m_handler);
    xmlReader.setErrorHandler(m_handler);
    xmlReader.parse(source);
    delete source;
    litProgrammeTV();
    QApplication::restoreOverrideCursor();
}
//
void MainWindowImpl::on_soiree_clicked()
{
    m_handler->soiree();
}
//
void MainWindowImpl::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
//
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
    trayIcon->setToolTip("QMagneto");
}
//
void MainWindowImpl::slotIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
        showNormal();
        break;
    case QSystemTrayIcon::MiddleClick:
    case QSystemTrayIcon::Unknown:
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::Context:
        break;
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
    desc->clear();
    desc->setText( afficheDescription( prog ) );
}



void MainWindowImpl::itemDoubleClicked(QListWidgetItem *item)
{
    itemClique(item);
    ProgrammeTV prog = item->data(Qt::UserRole).value<ProgrammeTV>();
    if ( !prog.start.isValid() )
        return;
    QString resume;
    if ( prog.resume.count() )
        resume = prog.resume.first();
    ajouterProgramme(prog);
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
        QString nomFichier = settings.value("nomFichier", "").toString();
        ProgrammeTV prog;
        prog.start = QDateTime::fromTime_t( settings.value("debut", "").toInt() );
        prog.stop = QDateTime::fromTime_t( settings.value("fin", "").toInt() );
        prog.channelName = settings.value("chaine", "").toString();
        prog.channel = settings.value("numChaine", "").toString();
        Type type = (Type)settings.value("type", Enregistrement).toInt();
        ajouterProgramme(prog, nomFichier, false, type);
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
        QTableWidgetItem *item = m_uiProgrammes.table->item(i, 0);
        Programme prog = item->data(Qt::UserRole).value<Programme>();
        if ( prog.etat != Termine )
        {
            settings.beginGroup("Enregistrements"+QString::number(i));
            settings.setValue("chaine", prog.chaine);
            settings.setValue("numChaine", prog.numChaine);
            settings.setValue("debut", prog.debut.toTime_t());
            settings.setValue("fin", prog.fin.toTime_t());
            settings.setValue("type", prog.type);
            settings.setValue("nomFichier", m_uiProgrammes.table->item(i, 3)->text());
            settings.endGroup();
        }
    }
}


QString MainWindowImpl::afficheDescription(ProgrammeTV prog)
{
    int secs = prog.start.time().secsTo( prog.stop.time() );
    QString resume = prog.resume.join("<br>");
    QString critique;
    if ( resume.contains("Critique") )
    {
        critique = resume.section("Critique :", 1, 1);
        resume = resume.section("Critique :", 0, 0);
    }
    QString d = "<html>";
    d = d + "<table style=\"text-align: left; width: 100%;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">";
    d = d + "<tbody><tr>";
    d = d + "<td width=20%><img style=\"vertical-align: top;\" src=\":/images/images/"+prog.channelName+".png\"></td>";
    //d = d + "<td width=20%><img style=\"vertical-align: top;\" src=\""+QDir::tempPath()+"/qmagnetochaine.jpg\"></td>";
    d = d +"<td width=40% align=left valign=top>"
        +"<span style=\"font-weight: bold;\">"
        +prog.title
        +"</span> " + prog.subTitle
        +"<br>"+prog.start.toString("hh:mm")+"-"+prog.stop.toString("hh:mm")
        +" ("+QTime(0,0).addSecs(secs).toString("hh:mm")+")</td>";

    d = d +"<td width=15% align=left valign=top>";
    for (int i=0; i<prog.star.section("/", 0, 0).toInt(); i++)
        d = d + "<img style=\"vertical-align: middle;\" src=\":/images/images/star.png\">";
    d = d + "</td>";
    QFile::remove(QDir::tempPath()+"/qmagnetoprog.jpg") ;
    if ( !prog.icon.isEmpty() )
        m_handler->imageToTmp(prog.icon, false);
    if ( QFile::exists( QDir::tempPath()+"/qmagnetoprog.jpg" ) )
    {
        //QD;
        d = d + "<td width=25% align=right><img style=\"vertical-align: middle; text-align: right;\" src=\""+QDir::tempPath()+"/qmagnetoprog.jpg\"></td>";
    }

    d += "</tbody></table><br>";
    d+="</td>";
    d += "<span style=\"font-weight: bold;\">"+QString::fromUtf8("CATEGORIE : ")+"</span>"+prog.category.join("/");
    d += "<br>";
    if ( !prog.histoire.isEmpty() )
        d += "<span style=\"font-weight: bold;\">"+QString::fromUtf8("HISTOIRE : ")+"</span>"+prog.histoire+"</span><br>";
    //d += resume;
    if ( !resume.isEmpty() )
        d += "<span style=\"font-weight: bold;\">"+QString::fromUtf8("RESUME : ")+"</span>"+resume+"</span><br>";
    if ( !critique.isEmpty() )
        d += "<span style=\"font-weight: bold;\">"+QString::fromUtf8("CRITIQUE : ")+"</span>"+critique+"</span>";
    d += "</html>";
//QD << d.toAscii();
    QApplication::clipboard()->setText( d.toAscii() );
    return d;
}


QString MainWindowImpl::numBox(QString s)
{
    QSettings settings(cheminIni() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    QString ret = settings.value(s, "NONE").toString();
    settings.endGroup();
    return ret;
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


void MainWindowImpl::on_action_Canaux_triggered()
{
    CanauxImpl *dialog = new CanauxImpl(this, m_handler->chaines());
    if ( dialog->exec() == QDialog::Accepted )
        litProgrammeTV();
    delete dialog;
}

void MainWindowImpl::on_dateEdit_dateChanged(QDate date)
{
    m_currentDate = date;
    litProgrammeTV();
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
}
