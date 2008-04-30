#include <QApplication>
#include <QProcess>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QColor>
#include <QRect>
#include <QLinearGradient>
#include <QSizePolicy>
#include <QPushButton>
#include <QTextEdit>
#include <QSlider>
#include <QCloseEvent>
#include <QTimer>
#include "visu.h"

Visu::Visu(QWidget *parent, QWidget *rendererTarget)
        : isPlaying(false), m_renderTarget(rendererTarget)
{
    //m_renderTarget->setAttribute(Qt::WA_PaintOnScreen);

    mplayerProcess = new QProcess(this);

    connect(mplayerProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(catchOutput()));
    connect(mplayerProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(mplayerEnded(int, QProcess::ExitStatus)));
}
void Visu::closeEvent(QCloseEvent *e)
{
    stopMPlayer();
    e->accept();
}

bool Visu::startMPlayer(QString stream)
{
    if (isPlaying)
        return true;
	if( !stream.isEmpty() )
		m_stream = stream;
    QStringList args;

    // On demande ï¿½ utiliser mplayer comme backend
    args << "-slave";
    // Et on veut ne pas avoir trop de chose ï¿½ parser :)
    args << "-quiet";
#ifdef Q_WS_WIN
    // reinterpret_cast<qlonglong> obligatoire, winId() ne se laissant pas convertir gentiment ;)
    args << "-wid" << QString::number(reinterpret_cast<qlonglong>(m_renderTarget->winId()));
    args << "-vo" << "directx:noaccel";
#else
    // Sur linux, aucun driver n'a ï¿½tï¿½ nï¿½cessaire et pas de manip pour Wid :)
    args << "-wid" << QString::number(m_renderTarget->winId());
#endif

    args << m_stream;

    // On parse la stdout et stderr au mï¿½me endroit, donc on demande ï¿½ "fusionnner" les 2 flux
    mplayerProcess->setProcessChannelMode(QProcess::MergedChannels);
    mplayerProcess->start("mplayer", args);
    if (!mplayerProcess->waitForStarted(3000))
    {
        qDebug("allez, cherche le bug :o");
        return false;
    }

    // On rï¿½cupï¿½re les infos de base
    mplayerProcess->write("get_video_resolution\n");
    mplayerProcess->write("get_time_length\n");

    isPlaying = true;

    return true;
}

bool Visu::stopMPlayer()
{
    if (!isPlaying)
        return true;

    mplayerProcess->write("quit\n");
    if (!mplayerProcess->waitForFinished(3000))
    {
        qDebug("ZOMG, ï¿½a plante :(");
        return false;
    }

    return true;
}

void Visu::catchOutput()
{
    while (mplayerProcess->canReadLine())
    {
        QByteArray buffer(mplayerProcess->readLine());

        // On vï¿½rifie si on a eu des rï¿½ponses
        // rï¿½ponse ï¿½ get_video_resolution : ANS_VIDEO_RESOLUTION='<width> x <height>'
        if (buffer.startsWith("ANS_VIDEO_RESOLUTION"))
        {
            buffer.remove(0, 21); // vire ANS_VIDEO_RESOLUTION=
            buffer.replace(QByteArray("'"), QByteArray(""));
            buffer.replace(QByteArray(" "), QByteArray(""));
            buffer.replace(QByteArray("\n"), QByteArray(""));
            buffer.replace(QByteArray("\r"), QByteArray(""));
            int sepIndex = buffer.indexOf('x');
            int resX = buffer.left(sepIndex).toInt();
            int resY = buffer.mid(sepIndex+1).toInt();
            //m_renderTarget->setMinimumSize(resX, resY);
        }
        // rï¿½ponse ï¿½ get_time_length : ANS_LENGTH=xx.yy
        else if (buffer.startsWith("ANS_LENGTH"))
        {
            buffer.remove(0, 11); // vire ANS_LENGTH=
            buffer.replace(QByteArray("'"), QByteArray(""));
            buffer.replace(QByteArray(" "), QByteArray(""));
            buffer.replace(QByteArray("\n"), QByteArray(""));
            buffer.replace(QByteArray("\r"), QByteArray(""));
            float maxTime = buffer.toFloat();
        }
        // rï¿½ponse ï¿½ get_time_pos : ANS_TIME_POSITION=xx.y
        else if (buffer.startsWith("ANS_TIME_POSITION"))
        {
            buffer.remove(0, 18); // vire ANS_TIME_POSITION=
            buffer.replace(QByteArray("'"), QByteArray(""));
            buffer.replace(QByteArray(" "), QByteArray(""));
            buffer.replace(QByteArray("\n"), QByteArray(""));
            buffer.replace(QByteArray("\r"), QByteArray(""));
            float currTime = buffer.toFloat();
        }
    }
}

// Play/stop
void Visu::switchPlayState()
{
    if (!isPlaying)
    {
        if (!startMPlayer())
            return;
        isPlaying = true;
    }
    else
    {
        if (!stopMPlayer())
            return;

        isPlaying = false;
    }
}

void Visu::mplayerEnded(int exitCode, QProcess::ExitStatus exitStatus)
{
    isPlaying = false;
}
