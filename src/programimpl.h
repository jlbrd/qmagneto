#ifndef PROGRAMIMPL_H
#define PROGRAMIMPL_H
//
#include <QDialog>
#include "ui_program.h"
#include "mainwindowimpl.h"
#include "xmldefaulthandler.h"
//
class ProgramImpl : public QDialog, public Ui::Program
{
Q_OBJECT
public:
	ProgramImpl( QWidget * parent, TvProgram prog, QString formatNomFichier);
	MainWindowImpl::Kind kind() { return m_kind; };
	void setType(MainWindowImpl::Kind t) { m_kind = t; };
private slots:
	void on_filename_cursorPositionChanged(int , int );
	void on_boutonRegarder_clicked();
private:
	TvProgram m_prog;
	MainWindowImpl *m_mainWindowImpl;
	MainWindowImpl::Kind m_kind;
};
#endif





