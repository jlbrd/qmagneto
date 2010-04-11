#ifndef FINDGLOBALIMPL_H
#define FINDGLOBALIMPL_H
//
#include <QDialog>
#include "ui_findglobal.h"
//
class XmlDefaultHandler;
class MainWindowImpl;
class GraphicsRectItem;

class FindGlobalImpl : public QDialog, public Ui::FindGlobal
{
Q_OBJECT
public:
	void setCategories(QStringList value);
	FindGlobalImpl( QWidget * parent, XmlDefaultHandler *handler, Qt::WFlags f = 0 );
    QList<GraphicsRectItem *> listItemProgrammes()
    {
        return m_programsItemsList;
    };
private slots:
	void on_findButton_clicked();
	void slotHours(int value);
	void slotHours(bool value);
private:
    MainWindowImpl *m_main;
	QStringList m_categories;
	XmlDefaultHandler *m_handler;
    QList<GraphicsRectItem *> m_programsItemsList;

protected:
	void resizeEvent(QResizeEvent * event);
};
#endif





