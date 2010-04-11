#include "application.h"
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

Application::Application( int & argc, char ** argv ) : QApplication( argc, argv)
{
}
bool Application::notify(QObject *receiver, QEvent *e)

{
    try
    {
        return QApplication::notify(receiver, e);
    }
    catch (...)
    {
    	QD << "*******************************************************************************************************************************";
    	QD << "*******************************************************************************************************************************";
        QD << "************************************************" << receiver << e->type() << "************************************************" ;
    	QD << "*******************************************************************************************************************************";
    	QD << "*******************************************************************************************************************************";

    }

}

