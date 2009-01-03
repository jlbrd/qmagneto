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
    connect(directoryButton, SIGNAL(clicked()), this, SLOT(slotDirectory()) );
    connect(XmlFilenameButton, SIGNAL(clicked()), this, SLOT(slotXml()) );
}
//

void ConfigImpl::on_populateDB_clicked()
{
    if ( fromFile->isChecked() )
    {
        m_mainWindowImpl->populateDB(true, XmlFilename->text() );
    }
    else
    {
        m_mainWindowImpl->populateDB(false, comboURL->currentText() );
    }
}

void ConfigImpl::slotDirectory()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the project directory"),
                    directory->text(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    directory->setText( s );
}
void ConfigImpl::slotXml()
{
    QString s = QFileDialog::getOpenFileName(this, tr("XML Filename"),
                XmlFilename->text(),
                tr("XML Files (*.xml *.XML *)"));
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    XmlFilename->setText( s );
}
