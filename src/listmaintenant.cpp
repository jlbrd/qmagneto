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
//

void ListMaintenant::paintEvent( QPaintEvent * event )
{
    QListWidget::paintEvent( event );
    for (int i= 0; i<count(); i++)
    {
        QListWidgetItem *it = item(i);
        ProgrammeTV prog = it->data(Qt::UserRole).value<ProgrammeTV>();
        QPixmap pix(visualItemRect(it).width(), visualItemRect(it).height());
        pix.fill(Qt::white);
        QPainter painter(&pix);
        painter.drawRect(pix.rect());
        painter.setBrush( QColor(Qt::blue).light(180) );
        float f = (float)prog.start.secsTo( prog.stop ) / (float)prog.start.secsTo( QDateTime::currentDateTime() );
//QD << prog.start.time() << prog.start.secsTo( prog.stop ) << prog.stop.time() << prog.start.secsTo( QDateTime::currentDateTime()) << f;
//QD << pix.width() << (float)pix.width()*f;
//QD << prog.start.time() << prog.stop.time() << f << ((float)pix.width()/(float)prog.start.secsTo( prog.stop ))*f;
        painter.drawRect(0,0, (float)visualItemRect(it).width()/f, pix.height());
        painter.end();
        it->setBackground( QBrush(pix) );
    }
}


