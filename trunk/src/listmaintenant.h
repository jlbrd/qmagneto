#ifndef LISTMAINTENANT_H
#define LISTMAINTENANT_H
//
#include <QListWidget>
//
class MainWindowImpl;
class ListMaintenant : public QListWidget
{
Q_OBJECT
protected:
public:
	ListMaintenant(QWidget *main);
private:
	MainWindowImpl *m_main;	
private slots:
};
#endif
