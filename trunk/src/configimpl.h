#ifndef CONFIGIMPL_H
#define CONFIGIMPL_H
//
#include <QDialog>
#include "ui_config.h"
#include <mainwindowimpl.h>
//
class ConfigImpl : public QDialog, public Ui::Config
{
Q_OBJECT
public:
	ConfigImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
	void on_populateDB_clicked();
	void slotChoixRepertoire();
	void slotFichierXML();
private:
	MainWindowImpl *m_mainWindowImpl;
};
#endif





