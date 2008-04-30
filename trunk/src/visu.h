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

class Visu: public QObject
{
    Q_OBJECT

public:
    Visu(QWidget *parent, QWidget *rendererTarget);
    bool startMPlayer(QString stream="");
    bool stopMPlayer();
protected:
    virtual void closeEvent(QCloseEvent *e);
private:
private slots:
    void catchOutput();
    void switchPlayState();
    void mplayerEnded(int exitCode, QProcess::ExitStatus exitStatus);
private:
    QWidget *m_renderTarget;
    QProcess *mplayerProcess;
    bool isPlaying;
    QString m_stream;
};
