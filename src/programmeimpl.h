#ifndef PROGRAMMEIMPL_H
#define PROGRAMMEIMPL_H
//
#include <QDialog>
#include "ui_programme.h"
#include "mainwindowimpl.h"
#include "xmldefaulthandler.h"
//
class ProgrammeImpl : public QDialog, public Ui::Programme
{
Q_OBJECT
public:
	ProgrammeImpl( QWidget * parent, ProgrammeTV prog, QString formatNomFichier);
private slots:
	void on_nomFichier_cursorPositionChanged(int , int );
private:
	ProgrammeTV m_prog;
	MainWindowImpl *m_mainWindowImpl;
};
#endif





