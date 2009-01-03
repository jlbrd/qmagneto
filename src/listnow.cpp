#include "listnow.h"
#include "xmldefaulthandler.h"
#include "mainwindowimpl.h"
#include <QPainter>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ListNow::ListNow( QWidget *main )
        : QListWidget(main)
{
    m_main = (MainWindowImpl *)main;
}
