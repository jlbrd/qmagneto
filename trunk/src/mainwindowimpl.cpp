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
    progressBar->setVisible(false);
    menu_View->addAction(dockDesc->toggleViewAction());
    menu_View->addAction(dockEvening->toggleViewAction());
    menu_View->addAction(dockMaintenant->toggleViewAction());
    menu_View->addAction(dockCustomCommandLog->toggleViewAction());
    menu_View->addAction(dockPrograms->toggleViewAction());
    connect(graphicsViewProgrammes->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotHorizontalValueChanged(int)) );
    connect(graphicsViewProgrammes->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotVerticalValueChanged(int)) );
    m_handler = new XmlDefaultHandler(this, graphicsViewProgrammes);
    graphicsViewProgrammes->setScene( new QGraphicsScene(this) );
    m_command = "mencoder";
    m_customCommand = "tv_grab_fr --days 8 --slow --output /tmp/tv.xml";
    m_customCommandFile = "/tmp/tv.xml";
    //m_commandOptions = "$STREAM -oac mp3lame -lameopts abr:br=64 -af volnorm -ovc lavc -lavcopts vcodec=mpeg4:aspect=15/9:vbitrate=512 -vf crop=0:0,scale=352:288 -idx -ffourcc DIVX -ofps 25.0 -o $OUT";
    //m_commandOptions = "\"$STREAM\" -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=300 -vf scale=-2:240 -ffourcc DIVX -fps 25 -ofps 25 -o \"$OUT\"";
    //m_commandOptions = "--intf dummy \"$STREAM\" :sout=#transcode{vcodec=h264,vb=2048,scale=1,acodec=mpga,ab=192,channels=2}:duplicate{dst=std{access=file,mux=ts,dst=\"'$OUT.avi'\"}}";
    m_commandOptions = "\"$STREAM\"  -oac mp3lame -ovc lavc -lavcopts vcodec=mpeg4:mbd=1:vbitrate=1500 -vf scale=-2:400 -ffourcc DIVX -fps 25 -ofps 25 -o  \"$OUT\"";
    m_readingCommand = "vlc";
    m_readingCommandOptions = "\"$STREAM\"";
    m_directory = QDir::homePath();
    m_currentDate = QDate::currentDate();
    m_hourBeginning = 6;
    m_systrayStarts = false;
    m_filenameFormat = "[%n]-%t-%d.%m%y";
    m_sourceUpdate = 1;
    m_comboURL = 0;
    m_proxyEnabled = false;
    m_proxyAddress = "";
    m_proxyPort = 0;
    m_http = 0;
    m_scheduledUpdate = true;
    m_atStartup = true;
    m_everyDay = false;
    m_everyDayAt = 0;
    m_onlyIfOutOfDate = true;
    m_onlyIfOutOfDateDay = 3;
    connect(action_ReadProgramGuide, SIGNAL(triggered()), this, SLOT(slotPopulateDB()));
    m_timerMinute = new QTimer(this);
    connect(m_timerMinute, SIGNAL(timeout()), this, SLOT(slotTimerMinute()));
    m_timer3Seconde = new QTimer(this);
    connect(m_timer3Seconde, SIGNAL(timeout()), this, SLOT(slotTimer3Seconde()));
    m_timer3Seconde->start( 3000 );
    m_scheduledUpdateTimer = new QTimer(this);
    m_scheduledUpdateTimer->setSingleShot( true );
    connect(m_scheduledUpdateTimer, SIGNAL(timeout()), this, SLOT(slotScheduledUpdate()));
    createTrayIcon();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotIconActivated(QSystemTrayIcon::ActivationReason)));
    QIcon icon = QIcon(":/images/images/tv.png");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    m_programsDialog = new QDialog(this);
    connect(listNow, SIGNAL(itemClicked ( QListWidgetItem * )), this, SLOT(slotItemClicked ( QListWidgetItem * )) );
    connect(listNow, SIGNAL(itemDoubleClicked( QListWidgetItem * )), this, SLOT(itemDoubleClicked( QListWidgetItem * )) );
    connect(listEvening, SIGNAL(itemClicked ( QListWidgetItem * )), this, SLOT(slotItemClicked ( QListWidgetItem * )) );
    connect(listEvening, SIGNAL(itemDoubleClicked( QListWidgetItem * )), this, SLOT(itemDoubleClicked( QListWidgetItem * )) );
    connect(programsRemove, SIGNAL(clicked()), this, SLOT(slotDelete()) );
    actionToggleFullScreen = new QAction(this);
    actionToggleFullScreen->setShortcut( Qt::Key_F6 );
    actionToggleFullScreen->setShortcutContext( Qt::ApplicationShortcut );
    connect(actionToggleFullScreen, SIGNAL(triggered()), this, SLOT(slotToggleFullScreen()) );
    addAction( actionToggleFullScreen );
    readRecording();
    trayIcon->show();
    //
    gridLayout->setSpacing(-1);
    gridLayout->setMargin(9);
    m_findWidget = new QWidget;
    uiFind.setupUi(m_findWidget);
    connect(uiFind.toolClose, SIGNAL(clicked()), m_findWidget, SLOT(hide()) );
    connect(uiFind.editFind, SIGNAL(textChanged(QString)), this, SLOT(slotFindWidget_textChanged(QString)) );
    connect(uiFind.editFind, SIGNAL(returnPressed()), this, SLOT(slotFindNext()) );
    connect(uiFind.toolPrevious, SIGNAL(clicked()), this, SLOT(slotFindPrevious()) );
    connect(uiFind.toolNext, SIGNAL(clicked()), this, SLOT(slotFindNext()) );
    uiFind.labelWrapped->setVisible(false);
    //
    m_autoHideTimer = new QTimer(this);
    m_autoHideTimer->setInterval(10000);
    m_autoHideTimer->setSingleShot(true);
    connect(m_autoHideTimer, SIGNAL(timeout()), m_findWidget, SLOT(hide()));
    gridLayout->addWidget(m_findWidget, 100, 0, 1, 1);
    m_findWidget->hide();
}
MainWindowImpl::~MainWindowImpl()
{
    if ( m_http )
    {
        m_file->close();
        QIODevice *device = (QFile *)m_http->currentDestinationDevice();
        delete device;
        m_http->deleteLater();
    }
    delete m_programsDialog;
}
//
void MainWindowImpl::slotFindPrevious()
{
    slotFindWidget_textChanged(uiFind.editFind->text(), true, false);
}
void MainWindowImpl::slotFindNext()
{
    slotFindWidget_textChanged(uiFind.editFind->text(), false, false);
}
//
void MainWindowImpl::on_action_Find_triggered()
{
    m_autoHideTimer->stop();
    m_findWidget->show();
    uiFind.editFind->setFocus(Qt::ShortcutFocusReason);
    m_autoHideTimer->start();
}
void MainWindowImpl::slotFindWidget_textChanged(QString text, bool backward, bool fromBegin)
{
    GraphicsRectItem *it = m_handler->findProgramme(text, backward, fromBegin, uiFind.sensitiveCase->isChecked(), uiFind.wholeWords->isChecked());
    if ( it )
    {
        slotItemClicked( it );
    }
    m_autoHideTimer->start();
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
    QTableWidgetItem *item = programsTable->item(programsTable->currentRow(), 0);
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
    programsTable->removeRow( programsTable->currentRow() );
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
        programsTable->setRowCount(programsTable->rowCount()+1);
        QTableWidgetItem *item1 = new QTableWidgetItem(prog.channelName);
        item1->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        programsTable->setItem(programsTable->rowCount()-1, 0, item1);
        //
        QTableWidgetItem *item = new QTableWidgetItem(start.toString(Qt::LocaleDate));
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        programsTable->setItem(programsTable->rowCount()-1, 1, item);
        //
        //
        item = new QTableWidgetItem(end.toString(Qt::LocaleDate));
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        programsTable->setItem(programsTable->rowCount()-1, 2, item);
        //
        QString t;
        if ( programImpl->kind() == Recording )
            t = tr("Recording planned");
        else
            t = tr("Reading planned");
        item = new QTableWidgetItem(t);
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        programsTable->setItem(programsTable->rowCount()-1, 4, item);
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
        programsTable->setItem(programsTable->rowCount()-1, 3, item);
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
    for (int i=0; i<programsTable->rowCount(); i++)
    {
        QTableWidgetItem *item = programsTable->item(i, 0);
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
                    programsTable->item(i, 4)->setText(tr("In Recording"));
                    program.state = Working;
                    programsTable->item(i, 3)->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

                    program.process = new QProcess( this );
                    options = m_commandOptions;
                    options.replace("$STREAM", numBox(program.channelNum));
                    options.replace("$OUT", m_directory+programsTable->item(i, 3)->text().replace("\"", " " ).replace("'"," ") );
                    connect(program.process, SIGNAL(readyReadStandardError()), this, SLOT(slotReadyReadStandardError()));
                    connect(program.process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadyReadStandardOutput()));
                    connect(program.process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotFinished(int, QProcess::ExitStatus)));
                    program.process->start(m_command+" "+options);
                    QD << "start" << m_command+" "+options;
                    QD << "end planned :" << QDateTime::currentDateTime().addMSecs(msecs);
                    break;
                case Reading:
                    programsTable->item(i, 4)->setText(tr("On reading"));
                    program.state = Completed;
                    programsTable->item(i, 3)->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
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
    m_customCommand = settings.value("m_customCommand", m_customCommand).toString();
    m_customCommandFile = settings.value("m_customCommandFile", m_customCommandFile).toString();
    m_comboURL = settings.value("m_comboURL", m_comboURL).toInt();
    m_sourceUpdate = settings.value("m_sourceUpdate", m_sourceUpdate).toInt();
    m_handler->setProgWidth( settings.value("m_progWidth", 180.0).toDouble() );
    m_handler->setProgHeight( settings.value("m_progHeight", 60.0).toDouble() );
    m_handler->setHourHeight( settings.value("m_hourHeight", 25.0).toDouble() );
    m_proxyEnabled = settings.value("m_proxyEnabled", m_proxyEnabled).toBool();
    m_proxyAddress = settings.value("m_proxyAddress", m_proxyAddress).toString();
    m_proxyPort = settings.value("m_proxyPort", m_proxyPort).toInt();
    m_proxyUsername = settings.value("m_proxyUsername", m_proxyUsername).toString();
    m_proxyPassword = settings.value("m_proxyPassword", m_proxyPassword).toInt();
    m_scheduledUpdate = settings.value("m_scheduledUpdate", m_scheduledUpdate).toBool();
    m_atStartup = settings.value("m_atStartup", m_atStartup).toBool();
    m_everyDay = settings.value("m_everyDay", m_everyDay).toBool();
    m_everyDayAt = settings.value("m_everyDayAt", m_everyDayAt).toInt();
    m_onlyIfOutOfDate = settings.value("m_onlyIfOutOfDate", m_onlyIfOutOfDate).toBool();
    m_onlyIfOutOfDateDay = settings.value("m_onlyIfOutOfDateDay", m_onlyIfOutOfDateDay).toInt();
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
    settings.setValue("m_customCommand", m_customCommand);
    settings.setValue("m_customCommandFile", m_customCommandFile);
    settings.setValue("m_comboURL", m_comboURL);
    settings.setValue("m_sourceUpdate", m_sourceUpdate);
    settings.setValue("m_progWidth", m_handler->progWidth());
    settings.setValue("m_progHeight", m_handler->progHeight());
    settings.setValue("m_hourHeight", m_handler->hourHeight());
    settings.setValue("m_programFont", GraphicsRectItem::programFont());
    settings.setValue("m_proxyEnabled", m_proxyEnabled);
    settings.setValue("m_proxyAddress", m_proxyAddress);
    settings.setValue("m_proxyPort", m_proxyPort);
    settings.setValue("m_proxyUsername", m_proxyUsername);
    settings.setValue("m_proxyPassword", m_proxyPassword);
    settings.setValue("m_scheduledUpdate", m_scheduledUpdate);
    settings.setValue("m_atStartup", m_atStartup);
    settings.setValue("m_everyDay", m_everyDay);
    settings.setValue("m_everyDayAt", m_everyDayAt);
    settings.setValue("m_onlyIfOutOfDate", m_onlyIfOutOfDate);
    settings.setValue("m_onlyIfOutOfDateDay", m_onlyIfOutOfDateDay);
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
    for (int i=0; i<programsTable->rowCount(); i++)
    {
        QTableWidgetItem *item = programsTable->item(i, 0);
        Program program = item->data(Qt::UserRole).value<Program>();
        if ( process == program.process )
        {
            program.process = 0;
            program.timer->stop();
            delete program.timer;
            program.timer = 0;
            programsTable->item(i, 4)->setText(tr("Ended") );
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
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        //QMessageBox::warning(this, tr("XML File"),
                             //tr("The XML file is too old or missing.")+"\n"+tr("Please update."));
        //on_action_Options_triggered();
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
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}


void MainWindowImpl::init()
{
    QHeaderView *header = programsTable->horizontalHeader();
    header->resizeSection( 0, 90 );
    header->resizeSection( 1, 110 );
    header->resizeSection( 2, 110 );
    header->resizeSection( 3, 270 );
    header->resizeSection( 4, 90 );
    programsTable->verticalHeader()->hide();
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
    for (int i=0; i<programsTable->rowCount(); i++)
    {
        QTableWidgetItem *item = programsTable->item(i, 0);
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
    foreach(GraphicsRectItem *it, m_handler->listItemProgrammes())
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
    if ( m_handler )
        delete m_handler;
    qApp->quit();
}

void MainWindowImpl::on_action_Options_triggered()
{
    ConfigImpl *dialog = new ConfigImpl(this);
    connect(dialog, SIGNAL(populateDB(int, QString)), this, SLOT(slotPopulateDB(int, QString)) );
    dialog->command->setText( m_command );
    dialog->commandOptions->setText( m_commandOptions );
    dialog->commandLecture->setText( m_readingCommand );
    dialog->commandLectureOptions->setText( m_readingCommandOptions );
    dialog->systrayStarts->setChecked( m_systrayStarts );
    dialog->directory->setText( m_directory );
    dialog->filename->setText( m_filenameFormat );
    dialog->XmlFilename->setText( m_xmlFilename );
    dialog->customCommand->setText( m_customCommand );
    dialog->customCommandFile->setText( m_customCommandFile );
    dialog->customCommand->setText( m_customCommand );
    dialog->comboURL->setCurrentIndex( m_comboURL );
    dialog->startHour->setValue( m_hourBeginning );
    dialog->programWidth->setValue( m_handler->progWidth() );
    dialog->programHeight->setValue( m_handler->progHeight() );
    dialog->proxyAddress->setText( m_proxyAddress );
    dialog->proxyPort->setValue( m_proxyPort );
    dialog->proxyUsername->setText( m_proxyUsername );
    dialog->proxyPassword->setText( m_proxyPassword );
    dialog->proxyEnabled->setChecked( m_proxyEnabled );
    dialog->scheduledUpdate->setChecked( m_scheduledUpdate );
    dialog->atStartup->setChecked( m_atStartup );
    dialog->onlyIfOutOfDate->setChecked( m_onlyIfOutOfDate );
    dialog->everyDay->setChecked( m_everyDay );
    dialog->everyDayAt->setValue( m_everyDayAt );
    dialog->onlyIfOutOfDateDay->setValue( m_onlyIfOutOfDateDay );
    //
    QFontDatabase db;
    dialog->comboFont->addItems( db.families() );
    dialog->comboFont->setCurrentIndex( dialog->comboFont->findText( GraphicsRectItem::programFont().family() ) );
    dialog->fontSize->setValue( GraphicsRectItem::programFont().pointSize() );
    if ( m_sourceUpdate == 0 )
        dialog->fromFile->setChecked( true );
    else if ( m_sourceUpdate == 1 )
        dialog->fromUrl->setChecked( true );
    else
        dialog->fromCustomCommand->setChecked( true );
    connect(dialog->XmlFilename, SIGNAL(textChanged(QString)), dialog->fromFile, SLOT(click()));
    connect(dialog->comboURL, SIGNAL(currentIndexChanged(int)), dialog->fromUrl, SLOT(click()));
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
        m_customCommand = dialog->customCommand->text();
        m_customCommandFile = dialog->customCommandFile->text();
        if ( dialog->fromFile->isChecked() )
            m_sourceUpdate = 0;
        else if ( dialog->fromUrl->isChecked() )
            m_sourceUpdate = 1;
        else
            m_sourceUpdate = 2;
        m_comboURL = dialog->comboURL->currentIndex();
        m_hourBeginning = dialog->startHour->value();
        m_handler->setProgWidth( dialog->programWidth->value() );
        m_handler->setProgHeight( dialog->programHeight->value() );
        m_proxyEnabled = dialog->proxyEnabled->isChecked();
        m_proxyAddress = dialog->proxyAddress->text();
        m_proxyPort = dialog->proxyPort->value();
        m_proxyUsername = dialog->proxyUsername->text();
        m_proxyPassword = dialog->proxyPassword->text();
        m_scheduledUpdate = dialog->scheduledUpdate->isChecked();
        m_atStartup = dialog->atStartup->isChecked();
        m_everyDay = dialog->everyDay->isChecked();
        m_everyDayAt = dialog->everyDayAt->value();
        m_onlyIfOutOfDate = dialog->onlyIfOutOfDate->isChecked();
        m_onlyIfOutOfDateDay = dialog->onlyIfOutOfDateDay->value();
        GraphicsRectItem::setProgramFont(
            QFont(dialog->comboFont->currentText(),
                  dialog->fontSize->value() )
        );
        saveIni();
        slotScheduledUpdate( true );
        //readTvGuide();
    }
    delete dialog;
}
void MainWindowImpl::slotPopulateDB(int source, QString XmlFilename)
{
    if ( m_http )
    {
        m_file->close();
        QIODevice *device = (QFile *)m_http->currentDestinationDevice();
        delete device;
        m_http->deleteLater();
    }
    m_handler->getImages()->setList(QStringList(), QSqlQuery());
    if ( source == -1 )
    {
        if ( m_sourceUpdate == 0 )
        {
            XmlFilename = m_xmlFilename;
        }
        else if ( m_sourceUpdate == 1 )
        {
            if ( m_comboURL == 0 )
                XmlFilename = "http://xmltv.myftp.org/download/tnt.zip";
            else
                XmlFilename = "http://xmltv.myftp.org/download/complet.zip";

        }
        else // Custom command
        {
            XmlFilename = m_customCommand;
        }
        source = m_sourceUpdate;
    }
    if ( source == 0 ) // From file
    {
        m_xmlFilename = XmlFilename;
        if ( m_xmlFilename.toLower().endsWith(".zip") )
            slotPopulateUnzip();
        else
            slotPopulateParse();
    }
    else if ( source == 1 ) // From URL
    {
        progressBar->setFormat(tr("Downloading %p%"));
        progressBar->setVisible(true);
        m_xmlFilename = QDir::tempPath()+"/"+XmlFilename.section("/", -1, -1).section(".", 0, 0)+".xml";
        QD << QString(tr("Download of %1")).arg(XmlFilename);
        m_file = new QFile( QDir::tempPath()+"/fichier.zip" );
        m_file->open(QIODevice::WriteOnly);
        m_http = new QHttp(this);
        connect(m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(slotDataReadProgress(int,int)));
        if ( m_proxyEnabled )
        {
            m_http->setProxy(m_proxyAddress, m_proxyPort, m_proxyUsername, m_proxyPassword);
        }
        connect(m_http, SIGNAL(requestFinished(int, bool)), this, SLOT(slotPopulateUnzip(int, bool)) );
        QUrl url(XmlFilename);
        m_http->setHost(url.host());
        m_httpId = m_http->get( url.toString(), m_file);
        QD << "get" << XmlFilename;
    }
    else // Custom command
    {
        progressBar->setFormat(tr("Custom command:")+" \""+XmlFilename+"\" "+tr("in progress"));
        progressBar->setVisible(true);
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        QProcess *process = new QProcess;
        connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotPopulateParse()) );
        connect(process, SIGNAL(readyReadStandardError()), this, SLOT(slotCustomCommandError()) );
        connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotCustomCommandError()) );
        customCommandLog->clear();
        process->start( XmlFilename );
    }
}
void MainWindowImpl::slotCustomCommandError()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    customCommandLog->append( process->readAllStandardError() );
    customCommandLog->append( process->readAllStandardOutput() );
}
void MainWindowImpl::slotDataReadProgress(int done, int total)
{
    progressBar->setRange(0, total);
    progressBar->setValue(done);
}
//
void MainWindowImpl::slotPopulateUnzip(int id, bool error)
{
    if ( id != m_httpId )
        return;
    QIODevice *device = (QFile *)m_http->currentDestinationDevice();
    if ( error )
    {
        QD << m_http->errorString();
        delete device;
        m_http->deleteLater();
        m_http = 0;
        QMessageBox::warning(this, tr("XML File"), tr("Unable to download the file."));
        //QApplication::setOverrideCursor(Qt::ArrowCursor);
        return;
    }
    m_file->close();
    delete device;
    m_http->deleteLater();
    m_http = 0;
    QD << "decompression de " + QDir::tempPath()+"/fichier.zip";
    QProcess process;
    QString command;
#ifdef WIN32
    command = QCoreApplication::applicationDirPath() + "/unzip.exe";
#else
    command = "unzip";
#endif
    process.start(command, QStringList() << "-o" << QDir::tempPath()+"/fichier.zip" << "-d" << QDir::tempPath());
    process.waitForFinished(-1);
    if ( process.exitCode() )
    {
        QMessageBox::warning(this, tr("XML File"), tr("Unable to unzip the file. You must have the command unzip."));
        //QApplication::setOverrideCursor(Qt::ArrowCursor);
        return;
    }
    QD << "analyse de " + m_xmlFilename;
    process.terminate();
    if ( !QFile::exists(m_xmlFilename) )
    {
        QMessageBox::warning(this, tr("XML File"), tr("A problem was occurs during the download."));
        //QApplication::setOverrideCursor(Qt::ArrowCursor);
        return;
    }
    slotPopulateParse();
    progressBar->setVisible(false);
}
//
void MainWindowImpl::slotPopulateParse()
{
    progressBar->setVisible(false);
    QProcess *process = qobject_cast<QProcess *>(sender());
    QXmlSimpleReader xmlReader;
    QString s;
    if ( process )
    {
        s = m_customCommandFile;
        process->deleteLater();
    }
    else
        s = m_xmlFilename;
    QFile file(s);
    QXmlInputSource *source = new QXmlInputSource(&file);
    xmlReader.setContentHandler(m_handler);
    xmlReader.setErrorHandler(m_handler);
    xmlReader.parse(source);
    delete source;
    readTvGuide();
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
    settings.setValue("nombre", programsTable->rowCount());
    settings.endGroup();
    for (int i=0; i<programsTable->rowCount(); i++)
    {
        QTableWidgetItem *item = programsTable->item(i, 0);
        Program prog = item->data(Qt::UserRole).value<Program>();
        if ( prog.state != Completed )
        {
            settings.beginGroup("Enregistrements"+QString::number(i));
            settings.setValue("channel", prog.channel);
            settings.setValue("channelNum", prog.channelNum);
            settings.setValue("start", prog.start.toTime_t());
            settings.setValue("end", prog.end.toTime_t());
            settings.setValue("kind", prog.kind);
            settings.setValue("filename", programsTable->item(i, 3)->text());
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

void MainWindowImpl::slotScheduledUpdate(bool fromOptionDialog)
{
    if( !m_scheduledUpdate )
        return;
    QTimer *timer = qobject_cast<QTimer *>(sender());
    bool programOutdated= m_handler->programOutdated(m_onlyIfOutOfDateDay);
    if( m_atStartup && timer==0 && !fromOptionDialog)
    {
        if( !m_onlyIfOutOfDate || (m_onlyIfOutOfDate && programOutdated) )
        {
            // Update
            slotPopulateDB();
        }
    }
    else if ( timer )
    {
        // Update
        slotPopulateDB();
    }
    if( m_everyDay )
    {
        // Re-starts the timer
        QDateTime next;
        if( QDateTime::currentDateTime().time().hour() > m_everyDayAt )
            next = QDateTime(QDate::currentDate().addDays(1), QTime(m_everyDayAt, 0) );
        else
            next = QDateTime(QDate::currentDate(), QTime(m_everyDayAt, 0) );
        int msecs = QDateTime::currentDateTime().secsTo( next ) * 1000;
        m_scheduledUpdateTimer->start(msecs);
    }
}

