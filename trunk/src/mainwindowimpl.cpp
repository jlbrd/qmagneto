#include "mainwindowimpl.h"
#include "ui_program.h"
#include "listnow.h"
#include "channelsimpl.h"
#include "configimpl.h"
#include "programimpl.h"
#include "ui_about.h"
#include "ui_newversion.h"
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
#include <QNetworkProxy>

#ifdef Q_OS_WIN32
#include <shlobj.h>
#endif
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
#include "releaseversion.h"

//
MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f)
        : QMainWindow(parent, f)
{
    setupUi(this);
    menu_View->addAction(dockDesc->toggleViewAction());
    menu_View->addAction(dockEvening->toggleViewAction());
    menu_View->addAction(dockMaintenant->toggleViewAction());
    connect(graphicsViewProgrammes->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotHorizontalValueChanged(int)) );
    connect(graphicsViewProgrammes->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotVerticalValueChanged(int)) );
    m_handler = new XmlDefaultHandler(this, graphicsViewProgrammes);
    graphicsViewProgrammes->setScene( new QGraphicsScene(this) );
    m_command = "mencoder";
    //m_commandOptions = "$STREAM -oac mp3lame -lameopts abr:br=64 -af volnorm -ovc lavc -lavcopts vcodec=mpeg4:aspect=15/9:vbitrate=512 -vf crop=0:0,scale=352:288 -idx -ffourcc DIVX -ofps 25.0 -o $OUT";
    //m_commandOptions = "\"$STREAM\" -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=300 -vf scale=-2:240 -ffourcc DIVX -fps 25 -ofps 25 -o \"$OUT\"";
    //m_commandOptions = "--intf dummy \"$STREAM\" :sout=#transcode{vcodec=h264,vb=2048,scale=1,acodec=mpga,ab=192,channels=2}:duplicate{dst=std{access=file,mux=ts,dst=\"'$OUT.avi'\"}}";
    m_http = 0;
    m_checkNewVersion = true;
    m_commandOptions = "\"$STREAM\"  -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=1500 -vf scale=-2:400 -ffourcc DIVX -fps 25 -ofps 25 -o  \"$OUT\"";
    m_readingCommand = "vlc";
    m_readingCommandOptions = "\"$STREAM\"";
    m_directory = QDir::homePath();
    m_currentDate = QDate::currentDate();
    m_hourBeginning = 6;
    m_systrayStarts = false;
    m_filenameFormat = "[%n]-%t-%d.%m%y";
    m_fromFile = false;
    m_comboURL = 0;
    m_proxyEnabled = false;
    m_proxyAddress = "";
    m_proxyPort = 0;
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
    m_programsDialog = new QDialog(this);
    m_programsUi.setupUi(m_programsDialog);
    connect(listNow, SIGNAL(itemClicked ( QListWidgetItem * )), this, SLOT(slotItemClicked ( QListWidgetItem * )) );
    connect(listNow, SIGNAL(itemDoubleClicked( QListWidgetItem * )), this, SLOT(itemDoubleClicked( QListWidgetItem * )) );
    connect(listEvening, SIGNAL(itemClicked ( QListWidgetItem * )), this, SLOT(slotItemClicked ( QListWidgetItem * )) );
    connect(listEvening, SIGNAL(itemDoubleClicked( QListWidgetItem * )), this, SLOT(itemDoubleClicked( QListWidgetItem * )) );
    connect(m_programsUi.remove, SIGNAL(clicked()), this, SLOT(slotDelete()) );
    actionToggleFullScreen = new QAction(this);
    actionToggleFullScreen->setShortcut( Qt::Key_F6 );
    actionToggleFullScreen->setShortcutContext( Qt::ApplicationShortcut );
    connect(actionToggleFullScreen, SIGNAL(triggered()), this, SLOT(slotToggleFullScreen()) );
    addAction( actionToggleFullScreen );
    readRecording();
    trayIcon->show();
    if( m_checkNewVersion )
    {
	    m_http = new QHttp( this );
	    connect(m_http, SIGNAL(done(bool)), this, SLOT(slotReleaseVersion(bool)) );
	    QUrl url("http://code.google.com/p/qmagneto/source/browse/trunk/src/releaseversion.h");
	    m_http->setHost(url.host());
	    m_http->get( url.toString());
   	}
}
//
MainWindowImpl::~MainWindowImpl()
{
    if ( m_http )
    {
        m_http->clearPendingRequests();
        m_http->abort();
        m_http->close();
        delete m_http;
    }
    delete m_programsDialog;
}
void MainWindowImpl::slotToggleFullScreen()
{
#ifdef MAEMO
    setWindowState(windowState() ^ Qt::WindowFullScreen);
#else
    setGeometry(80, 60, width()==800 ? 672 : 800, height()==480 ? 396 : 480);
#endif
}

void MainWindowImpl::slotDelete()
{
    QTableWidgetItem *item = m_programsUi.table->item(m_programsUi.table->currentRow(), 0);
    if ( !item )
        return;
    Program program = item->data(Qt::UserRole).value<Program>();
    if ( program.process )
    {
        //kill(program.process->pid(), SIGINT);
#ifdef Q_WS_WIN
        program.process->kill();
#else
        program.process->terminate();
#endif
    }
    if ( program.timer )
    {
        program.timer->stop();
        delete program.timer;
    }
    m_programsUi.table->removeRow( m_programsUi.table->currentRow() );
    saveRecording();
}

void MainWindowImpl::addProgram(TvProgram prog, QString title, bool showDialog, Kind kind)
{
    ProgramImpl *programImpl = new ProgramImpl(this, prog, m_filenameFormat);
    if ( prog.stop < QDateTime::currentDateTime()
            || numBox(prog.channel ).contains("NONE") )
    {
        programImpl->addButton->setDisabled( true );
        programImpl->viewButton->setDisabled( true );
        programImpl->labelInfo->setText(tr("Unable to record or read, the channel is not configured"));
    }
    else
    {
        programImpl->labelInfo->setText("");
    }
    programImpl->setType(kind);
    if ( !showDialog || programImpl->exec() == QDialog::Accepted )
    {
        QDateTime start;
        start.setDate(programImpl->beginDate->date());
        start.setTime(programImpl->beginHour->time());
        if ( showDialog && programImpl->checkBoxAvant->isChecked() )
            start = start.addSecs(programImpl->before->text().toInt()*-60);
        QDateTime end;
        end.setDate(programImpl->endDate->date());
        end.setTime(programImpl->endHour->time());
        if ( showDialog && programImpl->checkBoxAjouter->isChecked() )
            end = end.addSecs(programImpl->add->text().toInt()*60);
        QString nouveauTitre = programImpl->filename->text();
        if ( showDialog )
        {
            nouveauTitre.replace("%n", prog.channelName);
            nouveauTitre.replace("%t", prog.title);
            nouveauTitre.replace("%y", prog.start.date().toString("yyyy"));
            nouveauTitre.replace("%m", prog.start.date().toString("MMM"));
            nouveauTitre.replace("%d", prog.start.date().toString("dd"));
            nouveauTitre.remove('/').remove('\\');
        }
        else
        {
            nouveauTitre = title;
        }
        m_programsUi.table->setRowCount(m_programsUi.table->rowCount()+1);
        QTableWidgetItem *item1 = new QTableWidgetItem(prog.channelName);
        item1->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_programsUi.table->setItem(m_programsUi.table->rowCount()-1, 0, item1);
        //
        QTableWidgetItem *item = new QTableWidgetItem(start.toString(Qt::LocaleDate));
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_programsUi.table->setItem(m_programsUi.table->rowCount()-1, 1, item);
        //
        //
        item = new QTableWidgetItem(end.toString(Qt::LocaleDate));
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_programsUi.table->setItem(m_programsUi.table->rowCount()-1, 2, item);
        //
        QString t;
        if ( programImpl->kind() == Recording )
            t = tr("Recording planned");
        else
            t = tr("Reading planned");
        item = new QTableWidgetItem(t);
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        m_programsUi.table->setItem(m_programsUi.table->rowCount()-1, 4, item);
        //
        int msecs = QDateTime::currentDateTime().secsTo( start ) * 1000;
        msecs = qMax(0, msecs);
        Program program;
        program.id = QDateTime::currentDateTime().toTime_t();
        program.channel = prog.channelName;
        program.channelNum = prog.channel;
        program.start = start;//prog.start;
        program.end = end;//prog.stop;
        program.state = Idle;
        program.timer = new QTimer(this);
        program.process = 0;
        program.kind = programImpl->kind();
        connect(program.timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
        program.timer->start(msecs);
        //
        item = new QTableWidgetItem(nouveauTitre);
        m_programsUi.table->setItem(m_programsUi.table->rowCount()-1, 3, item);
        //
        QD << msecs << program.id << "start :" <<program.start.toString() << "end :" << program.end.toString();
        QVariant v;
        v.setValue( program );
        item1->setData(Qt::UserRole, v );
    }
    delete programImpl;
    saveRecording();
}

void MainWindowImpl::slotTimer()
{
    for (int i=0; i<m_programsUi.table->rowCount(); i++)
    {
        QTableWidgetItem *item = m_programsUi.table->item(i, 0);
        Program program = item->data(Qt::UserRole).value<Program>();
        if ( sender() == program.timer )
        {
            int msecs;
            QString options;
            switch ( program.state )
            {
            case Idle:
                switch ( program.kind )
                {
                case Recording:
                    msecs = QDateTime::currentDateTime().secsTo( program.end ) * 1000;
                    program.timer->start(msecs);
                    m_programsUi.table->item(i, 4)->setText(tr("In Recording"));
                    program.state = Working;
                    m_programsUi.table->item(i, 3)->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

                    program.process = new QProcess( this );
                    options = m_commandOptions;
                    options.replace("$STREAM", numBox(program.channelNum));
                    options.replace("$OUT", m_directory+m_programsUi.table->item(i, 3)->text().replace("\"", " " ).replace("'"," ") );
                    connect(program.process, SIGNAL(readyReadStandardError()), this, SLOT(slotReadyReadStandardError()));
                    connect(program.process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadyReadStandardOutput()));
                    connect(program.process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotFinished(int, QProcess::ExitStatus)));
                    program.process->start(m_command+" "+options);
                    QD << "start" << m_command+" "+options;
                    QD << "end planned :" << QDateTime::currentDateTime().addMSecs(msecs);
                    break;
                case Reading:
                    m_programsUi.table->item(i, 4)->setText(tr("On reading"));
                    program.state = Completed;
                    m_programsUi.table->item(i, 3)->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
                    options = m_readingCommandOptions;
                    options.replace("$STREAM", numBox(program.channelNum));
                    QD << m_readingCommand << options;
                    QProcess::startDetached(m_readingCommand+" "+options);
                    break;
                }
                break;
            case Working:
                program.timer->stop();
                QD << "end" << program.id << program.end.toString(Qt::LocaleDate);
                program.state = Completed;
                //kill(program.process->pid(), SIGINT);
#ifdef Q_WS_WIN
                program.process->kill();
#else
                program.process->terminate();
#endif
                break;
            case Completed:
                break;
            }
            QVariant v;
            v.setValue( program );
            item->setData(Qt::UserRole, v );
        }
    }
}

void MainWindowImpl::readIni()
{
    QSettings settings(iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Options");
    m_command = settings.value("m_command", m_command).toString();
    m_commandOptions = settings.value("m_commandOptions", m_commandOptions).toString();
    m_readingCommand = settings.value("m_readingCommand", m_readingCommand).toString();
    m_readingCommandOptions = settings.value("m_readingCommandOptions", m_readingCommandOptions).toString();
    m_directory = settings.value("m_directory", m_directory).toString();
    m_hourBeginning = settings.value("m_hourBeginning", m_hourBeginning).toInt();
    m_systrayStarts = settings.value("m_systrayStarts", m_systrayStarts).toBool();
    m_filenameFormat = settings.value("m_filenameFormat", m_filenameFormat).toString();
    m_xmlFilename = settings.value("m_xmlFilename", m_xmlFilename).toString();
    m_comboURL = settings.value("m_comboURL", m_comboURL).toInt();
    m_fromFile = settings.value("m_fromFile", m_fromFile).toBool();
    m_handler->setProgWidth( settings.value("m_progWidth", 180.0).toDouble() );
    m_handler->setProgHeight( settings.value("m_progHeight", 60.0).toDouble() );
    m_handler->setHourHeight( settings.value("m_hourHeight", 25.0).toDouble() );
    m_checkNewVersion = settings.value("m_checkNewVersion", m_checkNewVersion).toBool();
    m_proxyEnabled = settings.value("m_proxyEnabled", m_proxyEnabled).toBool();
    m_proxyAddress = settings.value("m_proxyAddress", m_proxyAddress).toString();
	//
    m_proxyPort = settings.value("m_proxyPort", m_proxyPort).toInt();
	QNetworkProxy proxy;
	proxy.setType(m_proxyEnabled ? QNetworkProxy::HttpCachingProxy : QNetworkProxy::NoProxy);
	proxy.setHostName(m_proxyAddress);
	proxy.setPort(m_proxyPort);
	QNetworkProxy::setApplicationProxy( proxy );
	//
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


void MainWindowImpl::saveIni()
{
    QSettings settings(iniPath() + "qmagneto.ini", QSettings::IniFormat);

    settings.beginGroup("Options");
    settings.setValue("m_command", m_command);
    settings.setValue("m_commandOptions", m_commandOptions);
    settings.setValue("m_readingCommand", m_readingCommand);
    settings.setValue("m_readingCommandOptions", m_readingCommandOptions);
    settings.setValue("m_directory", m_directory);
    settings.setValue("m_hourBeginning", m_hourBeginning);
    settings.setValue("m_systrayStarts", m_systrayStarts);
    settings.setValue("m_filenameFormat", m_filenameFormat);
    settings.setValue("m_xmlFilename", m_xmlFilename);
    settings.setValue("m_comboURL", m_comboURL);
    settings.setValue("m_fromFile", m_fromFile);
    settings.setValue("m_progWidth", m_handler->progWidth());
    settings.setValue("m_progHeight", m_handler->progHeight());
    settings.setValue("m_hourHeight", m_handler->hourHeight());
    settings.setValue("m_programFont", GraphicsRectItem::programFont());
    settings.setValue("m_checkNewVersion", m_checkNewVersion);
    settings.setValue("m_proxyEnabled", m_proxyEnabled);
    settings.setValue("m_proxyAddress", m_proxyAddress);
    settings.setValue("m_proxyPort", m_proxyPort);
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
    for (int i=0; i<m_programsUi.table->rowCount(); i++)
    {
        QTableWidgetItem *item = m_programsUi.table->item(i, 0);
        Program program = item->data(Qt::UserRole).value<Program>();
        if ( process == program.process )
        {
            program.process = 0;
            program.timer->stop();
            delete program.timer;
            program.timer = 0;
            m_programsUi.table->item(i, 4)->setText(tr("Ended") );
            program.state = Completed;
            QVariant v;
            v.setValue( program );
            item->setData(Qt::UserRole, v );
        }
    }
    process->deleteLater();
    QD << "Finished";
}


#include <QTime>
void MainWindowImpl::readTvGuide()
{
    QTime t;
    t.start();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_timerMinute->stop();
    m_handler->setDate(m_currentDate);
    m_handler->setHeureDebutJournee( m_hourBeginning );
    m_handler->init();
    if ( !m_handler->readFromDB() )
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("XML File"), 
        	tr("The XML file is too old or missing.")+"\n"+tr("Please update."));
        on_action_Options_triggered();
        return;
    }
    QDate minimumDate = m_handler->minimumDate();
    dateEdit->setMinimumDate(minimumDate);
    if ( m_currentDate == m_handler->minimumDate() )
        dayBeforeButton->setDisabled(true);
    else
        dayBeforeButton->setDisabled(false);
    QDate maximumDate = m_handler->maximumDate();
    dateEdit->setMaximumDate(maximumDate);
    if ( m_currentDate == m_handler->maximumDate() )
        dayAfterButton->setDisabled(true);
    else
        dayAfterButton->setDisabled(false);

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
    QHeaderView *header = m_programsUi.table->horizontalHeader();
    header->resizeSection( 0, 90 );
    header->resizeSection( 1, 110 );
    header->resizeSection( 2, 110 );
    header->resizeSection( 3, 270 );
    header->resizeSection( 4, 90 );
    m_programsUi.table->verticalHeader()->hide();
    readIni();
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

void MainWindowImpl::on_dayBeforeButton_clicked()
{
    m_currentDate = m_currentDate.addDays(-1);
    dateEdit->setDate( m_currentDate );
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
}

void MainWindowImpl::on_dayAfterButton_clicked()
{
    m_currentDate = m_currentDate.addDays(1);
    dateEdit->setDate( m_currentDate );
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
}

void MainWindowImpl::on_now_clicked()
{
    m_currentDate = QDate::currentDate();
    dateEdit->setDate( m_currentDate );
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
    m_handler->nowCenter();
}

void MainWindowImpl::slotTimerMinute()
{
    m_handler->currentTimeLinePosition();
    // List of evening programs
    listEvening->clear();
    foreach(TvProgram prog, m_handler->eveningPrograms() )
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/images/"+prog.channelName+".png"), prog.title);
        QVariant v;
        v.setValue( prog );
        item->setData(Qt::UserRole, v );
        listEvening->addItem( item );
    }
    dockEvening->setWindowTitle(tr("Evening of %1").arg(m_currentDate.toString(tr("dddd dd MMM yyyy"))));
    // Liste des programs now
    listNow->clear();
    foreach(TvProgram prog, m_handler->programsMaintenant() )
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/images/"+prog.channelName+".png"), prog.title);
        listNow->addItem( item );
        QVariant v;
        v.setValue( prog );
        item->setData(Qt::UserRole, v );

        QPixmap pix(listNow->visualItemRect(item).width(), listNow->visualItemRect(item).height());
        pix.fill(Qt::white);
        QPainter painter(&pix);
        painter.drawRect(pix.rect());
        painter.setBrush( QColor(Qt::blue).light(180) );
        float f = (float)prog.start.secsTo( prog.stop ) / (float)prog.start.secsTo( QDateTime::currentDateTime() );
        painter.drawRect(0,0, (float)listNow->visualItemRect(item).width()/f, pix.height());
        painter.end();
        item->setBackground( QBrush(pix) );
    }
    m_timerMinute->start(60000);
}

void MainWindowImpl::slotTimer3Seconde()
{
    int num = 0;
    for (int i=0; i<m_programsUi.table->rowCount(); i++)
    {
        QTableWidgetItem *item = m_programsUi.table->item(i, 0);
        Program program = item->data(Qt::UserRole).value<Program>();
        switch ( program.state )
        {
        case Working:
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

void MainWindowImpl::slotItemClicked(GraphicsRectItem *item)
{
    if ( item != 0 )
    {
        listNow->setCurrentRow(-1);
        listEvening->setCurrentRow(-1);
    }
    foreach(GraphicsRectItem *it, m_handler->listeItemProgrammes())
    {
        if (it==item)
        {
            it->setEnabled( true );
            TvProgram prog = it->data(0).value<TvProgram>();
            desc->clear();
            desc->setText( showDescription( prog ) );
        }
        else
            it->setEnabled( false );
    }
}


void MainWindowImpl::on_action_Quit_triggered()
{
    saveIni();
    if( m_handler )
    	delete m_handler;
    qApp->quit();
}

void MainWindowImpl::on_action_Options_triggered()
{
    ConfigImpl *dialog = new ConfigImpl(this);
    dialog->command->setText( m_command );
    dialog->commandOptions->setText( m_commandOptions );
    dialog->commandLecture->setText( m_readingCommand );
    dialog->commandLectureOptions->setText( m_readingCommandOptions );
    dialog->systrayStarts->setChecked( m_systrayStarts );
    dialog->directory->setText( m_directory );
    dialog->filename->setText( m_filenameFormat );
    dialog->XmlFilename->setText( m_xmlFilename );
    dialog->fromFile->setChecked( m_fromFile );
    dialog->comboURL->setCurrentIndex( m_comboURL );
    dialog->startHour->setValue( m_hourBeginning );
    dialog->programWidth->setValue( m_handler->progWidth() );
    dialog->programHeight->setValue( m_handler->progHeight() );
    dialog->findUpdate->setChecked( m_checkNewVersion );
    dialog->proxyEnabled->setChecked( m_proxyEnabled );
    dialog->proxyAddress->setText( m_proxyAddress );
    dialog->proxyPort->setValue( m_proxyPort );
	QNetworkProxy proxy;
	proxy.setType(m_proxyEnabled ? QNetworkProxy::HttpCachingProxy : QNetworkProxy::NoProxy);
	proxy.setHostName(m_proxyAddress);
	proxy.setPort(m_proxyPort);
	QNetworkProxy::setApplicationProxy( proxy );
	//
	QFontDatabase db;
	dialog->comboFont->addItems( db.families() );
	dialog->comboFont->setCurrentIndex( dialog->comboFont->findText( GraphicsRectItem::programFont().family() ) );
	dialog->fontSize->setValue( GraphicsRectItem::programFont().pointSize() );
    if ( dialog->exec() == QDialog::Accepted )
    {
        m_command = dialog->command->text();
        m_commandOptions = dialog->commandOptions->text();
        m_readingCommand = dialog->commandLecture->text();
        m_readingCommandOptions = dialog->commandLectureOptions->text();
        m_systrayStarts = dialog->systrayStarts->isChecked();
        m_directory = dialog->directory->text();
        if ( !m_directory.endsWith("/") )
            m_directory += "/";
        m_filenameFormat = dialog->filename->text();
        m_xmlFilename = dialog->XmlFilename->text();
        m_fromFile = dialog->fromFile->isChecked();
        m_comboURL = dialog->comboURL->currentIndex();
        m_hourBeginning = dialog->startHour->value();
	    m_handler->setProgWidth( dialog->programWidth->value() );
	    m_handler->setProgHeight( dialog->programHeight->value() );
	    m_checkNewVersion = dialog->findUpdate->isChecked();
	    m_proxyEnabled = dialog->proxyEnabled->isChecked();
	    m_proxyAddress = dialog->proxyAddress->text();
	    m_proxyPort = dialog->proxyPort->value();
	    GraphicsRectItem::setProgramFont( 
	    	QFont(dialog->comboFont->currentText(), 
	    	dialog->fontSize->value() ) 
	    );
        saveIni();
        readTvGuide();
    }
    delete dialog;
}
void MainWindowImpl::populateDB(bool fromFile, QString XmlFilename)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if ( fromFile )
    {
        m_xmlFilename = XmlFilename;
    }
    else
    {
        QProcess process;
        QD << QString(tr("Download of %1")).arg(XmlFilename);
        process.start("wget", QStringList() << "-O" << QDir::tempPath()+"/fichier.zip" << XmlFilename);
        process.waitForFinished(-1);
        if ( process.exitCode() )
        {
            QMessageBox::warning(this, tr("XML File"), tr("Unable to download the file. You must be connected to Internet and have the command wget."));
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
            QMessageBox::warning(this, tr("XML File"), tr("Unable to download the file. You must have the command unzip."));
            QApplication::restoreOverrideCursor();
            return;
        }
        XmlFilename = QDir::tempPath()+"/"+XmlFilename.section("/", -1, -1).section(".", 0, 0)+".xml";
        QD << "analyse de " + XmlFilename;
        process.terminate();
        if ( !QFile::exists(XmlFilename) )
        {
            QMessageBox::warning(this, tr("XML File"), tr("A problem was occurs during the download."));
            QApplication::restoreOverrideCursor();
            return;
        }
    }
    QXmlSimpleReader xmlReader;
    QFile file(XmlFilename);
    QXmlInputSource *source = new QXmlInputSource(&file);
    xmlReader.setContentHandler(m_handler);
    xmlReader.setErrorHandler(m_handler);
    xmlReader.parse(source);
    delete source;
    readTvGuide();
    QApplication::restoreOverrideCursor();
}
//
void MainWindowImpl::on_evening_clicked()
{
    m_handler->evening();
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
    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(on_action_Quit_triggered()));
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

void MainWindowImpl::on_action_Programs_triggered()
{
    m_programsDialog->show();
}

void MainWindowImpl::slotItemClicked(QListWidgetItem *item)
{
    slotItemClicked((GraphicsRectItem  *)0);
    if ( item->listWidget() == listEvening )
        listNow->setCurrentRow(-1);
    else
        listEvening->setCurrentRow(-1);
    TvProgram prog = item->data(Qt::UserRole).value<TvProgram>();
    desc->clear();
    desc->setText( showDescription( prog ) );
}



void MainWindowImpl::itemDoubleClicked(QListWidgetItem *item)
{
    slotItemClicked(item);
    TvProgram prog = item->data(Qt::UserRole).value<TvProgram>();
    if ( !prog.start.isValid() )
        return;
    QString resume;
    if ( prog.resume.count() )
        resume = prog.resume.first();
    addProgram(prog);
}


QString MainWindowImpl::iniPath()
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


void MainWindowImpl::readRecording()
{
    QSettings settings(iniPath() + "enregistrements.ini", QSettings::IniFormat);
    settings.beginGroup("Nombre");
    int nombre = settings.value("nombre", 0).toInt();
    settings.endGroup();
    for (int i=0; i<nombre; i++)
    {
        settings.beginGroup("Enregistrements"+QString::number(i));
        QString filename = settings.value("filename", "").toString();
        TvProgram prog;
        prog.start = QDateTime::fromTime_t( settings.value("start", "").toInt() );
        prog.stop = QDateTime::fromTime_t( settings.value("end", "").toInt() );
        prog.channelName = settings.value("channel", "").toString();
        prog.channel = settings.value("channelNum", "").toString();
        Kind kind = (Kind)settings.value("kind", Recording).toInt();
        addProgram(prog, filename, false, kind);
        settings.endGroup();
    }

}


void MainWindowImpl::saveRecording()
{
    QSettings settings(iniPath() + "enregistrements.ini", QSettings::IniFormat);
    settings.beginGroup("Nombre");
    settings.setValue("nombre", m_programsUi.table->rowCount());
    settings.endGroup();
    for (int i=0; i<m_programsUi.table->rowCount(); i++)
    {
        QTableWidgetItem *item = m_programsUi.table->item(i, 0);
        Program prog = item->data(Qt::UserRole).value<Program>();
        if ( prog.state != Completed )
        {
            settings.beginGroup("Enregistrements"+QString::number(i));
            settings.setValue("channel", prog.channel);
            settings.setValue("channelNum", prog.channelNum);
            settings.setValue("start", prog.start.toTime_t());
            settings.setValue("end", prog.end.toTime_t());
            settings.setValue("kind", prog.kind);
            settings.setValue("filename", m_programsUi.table->item(i, 3)->text());
            settings.endGroup();
        }
    }
}


QString MainWindowImpl::showDescription(TvProgram prog)
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
    //d = d + "<td width=20%><img style=\"vertical-align: top;\" src=\""+QDir::tempPath()+"/qmagnetochannel.jpg\"></td>";
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
    d += "<span style=\"font-weight: bold;\">"+tr("CATEGORY : ")+"</span>"+prog.category.join("/");
    d += "<br>";
    if ( !prog.story.isEmpty() )
        d += "<span style=\"font-weight: bold;\">"+tr("STORY : ")+"</span>"+prog.story+"</span><br>";
    //d += resume;
    if ( !resume.isEmpty() )
        d += "<span style=\"font-weight: bold;\">"+tr("SUMMARY : ")+"</span>"+resume+"</span><br>";
    if ( !critique.isEmpty() )
        d += "<span style=\"font-weight: bold;\">"+tr("OPINION : ")+"</span>"+critique+"</span>";
    d += "</html>";
//QD << d.toAscii();
    QApplication::clipboard()->setText( d.toAscii() );
    return d;
}


QString MainWindowImpl::numBox(QString s)
{
    QSettings settings(iniPath() + "qmagneto.ini", QSettings::IniFormat);
    settings.beginGroup("Channels");
    QString ret = settings.value(s, "NONE").toString();
    settings.endGroup();
    return ret;
}


void MainWindowImpl::on_action_About_triggered()
{
    QDialog about;
    Ui::About ui;
    ui.setupUi( &about );
    ui.version->setText( tr("Version %1").arg(VERSION) );
    about.exec();
}

void MainWindowImpl::on_action_AboutQt_triggered()
{
    QMessageBox::aboutQt( this );
}


void MainWindowImpl::on_action_Channels_triggered()
{
    ChannelsImpl *dialog = new ChannelsImpl(this, m_handler->channels());
    if ( dialog->exec() == QDialog::Accepted )
        readTvGuide();
    delete dialog;
}

void MainWindowImpl::on_dateEdit_dateChanged(QDate date)
{
    m_currentDate = date;
    readTvGuide();
    m_handler->deplaceChaines( 0 );
    m_handler->deplaceHeures( 0 );
}

void MainWindowImpl::slotReleaseVersion(bool error)
{
    if ( error )
    {
        QD << m_http->errorString();
	    m_http->deleteLater();
	    m_http = 0;
        return;
    }
    QString data;
    data = m_http->readAll();
    if ( data.isEmpty() )
    {
	    m_http->deleteLater();
	    m_http = 0;
        return;
   	}
    QString releaseVersion = data.section("define VERSION &quot;", 1, 1).section("&quot;", 0, 0);
    if (!releaseVersion.isEmpty() && releaseVersion!=QString(VERSION))
    {
	    QDialog nv;
	    Ui::NewVersion ui;
	    ui.setupUi( &nv );
	    ui.labelVersion->setText( ui.labelVersion->text().replace("VERSION", releaseVersion) );
	    nv.exec();
   	}
    m_http->deleteLater();
    m_http = 0;
}

