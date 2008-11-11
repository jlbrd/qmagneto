#include "listmaintenant.h"
#include "xmldefaulthandler.h"
#include "mainwindowimpl.h"
#include <QPainter>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ListMaintenant::ListMaintenant( QWidget *main )
        : QListWidget(main)
{
    m_main = (MainWindowImpl *)main;
}
