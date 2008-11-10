#include "programmeimpl.h"
//
ProgrammeImpl::ProgrammeImpl( QWidget * parent,ProgrammeTV prog, QString formatNomFichier) 
	: QDialog(parent)
{
	setupUi(this);
	m_mainWindowImpl = (MainWindowImpl *)parent;
	m_prog.channelName = prog.channelName;
	m_prog.title = prog.title;
	m_prog.start = prog.start;
	nomFichier->setText( formatNomFichier );
}
//

void ProgrammeImpl::on_nomFichier_cursorPositionChanged(int , int )
{
	QString nouveauTitre = nomFichier->text();
	nouveauTitre.replace("%n", m_prog.channelName);
	nouveauTitre.replace("%t", m_prog.title);
	nouveauTitre.replace("%a", m_prog.start.date().toString("yyyy"));
	nouveauTitre.replace("%m", m_prog.start.date().toString("MMM"));
	nouveauTitre.replace("%j", m_prog.start.date().toString("dd"));
	nouveauTitre.remove('/').remove('\\');
	apercuNomFichier->setText( m_mainWindowImpl->repertoire() + nouveauTitre );
}

