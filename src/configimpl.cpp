#include "configimpl.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
//
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

ConfigImpl::ConfigImpl( QWidget * parent, Qt::WFlags f)
        : QDialog(parent, f)
{
    setupUi(this);
    m_mainWindowImpl = (MainWindowImpl *) parent;
    connect(choixRepertoire, SIGNAL(clicked()), this, SLOT(slotChoixRepertoire()) );
    connect(choixFichierXML, SIGNAL(clicked()), this, SLOT(slotFichierXML()) );
}
//

void ConfigImpl::on_populateDB_clicked()
{
    if ( depuisFichier->isChecked() )
    {
        m_mainWindowImpl->populateDB(true, nomFichierXML->text() );
    }
    else
    {
        m_mainWindowImpl->populateDB(false, comboURL->currentText() );
    }
}

void ConfigImpl::slotChoixRepertoire()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the project directory"),
                    repertoire->text(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    repertoire->setText( s );
}
void ConfigImpl::slotFichierXML()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Fichier XML"),
                nomFichierXML->text(),
                tr("Fichiers XML (*.xml *.XML *)"));
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    nomFichierXML->setText( s );
}
