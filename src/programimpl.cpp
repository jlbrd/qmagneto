#include "programimpl.h"
//
ProgramImpl::ProgramImpl( QWidget * parent,TvProgram prog, QString formatNomFichier)
        : QDialog(parent)
{
    setupUi(this);
    m_mainWindowImpl = (MainWindowImpl *)parent;
    m_kind = MainWindowImpl::Recording;
    m_prog.channelName = prog.channelName;
    m_prog.title = prog.title;
    m_prog.start = prog.start;
    filename->setText( formatNomFichier );
    beginDate->setDate( prog.start.date() );
    beginHour->setTime( prog.start.time() );
    endDate->setDate( prog.stop.date() );
    endHour->setTime( prog.stop.time() );
    channel->setText( prog.channelName );
    programName->setTitle( prog.title );
    desc->clear();
    desc->setText( m_mainWindowImpl->showDescription( prog ) );

}
//

void ProgramImpl::on_filename_cursorPositionChanged(int , int )
{
    QString nouveauTitre = filename->text();
    nouveauTitre.replace("%n", m_prog.channelName);
    nouveauTitre.replace("%t", m_prog.title);
    nouveauTitre.replace("%y", m_prog.start.date().toString("yyyy"));
    nouveauTitre.replace("%m", m_prog.start.date().toString("MMM"));
    nouveauTitre.replace("%d", m_prog.start.date().toString("dd"));
    nouveauTitre.remove('/').remove('\\');
    overviewFilename->setText( m_mainWindowImpl->directory() + nouveauTitre );
}

void ProgramImpl::on_boutonRegarder_clicked()
{
    m_kind = MainWindowImpl::Reading;
    accept();
}
