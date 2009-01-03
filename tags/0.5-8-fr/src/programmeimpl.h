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
	MainWindowImpl::Type type() { return m_type; };
	void setType(MainWindowImpl::Type t) { m_type = t; };
private slots:
	void on_nomFichier_cursorPositionChanged(int , int );
	void on_boutonRegarder_clicked();
private:
	ProgrammeTV m_prog;
	MainWindowImpl *m_mainWindowImpl;
	MainWindowImpl::Type m_type;
};
#endif





