#ifndef CANAUXIMPL_H
#define CANAUXIMPL_H
//
#include <QDialog>
#include "xmldefaulthandler.h"
#include "ui_canaux.h"
//
class CanauxImpl : public QDialog, public Ui::Canaux
{
Q_OBJECT
public:
	CanauxImpl( QWidget * parent, QList<ChaineTV> chaines );
private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
private:
};
#endif





