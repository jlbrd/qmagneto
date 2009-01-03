#ifndef LISTMAINTENANT_H
#define LISTMAINTENANT_H
//
#include <QListWidget>
//
class MainWindowImpl;
class ListNow : public QListWidget
{
Q_OBJECT
protected:
public:
	ListNow(QWidget *main);
private:
	MainWindowImpl *m_main;	
private slots:
};
#endif
