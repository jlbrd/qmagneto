#ifndef __DEFS_H__
#define __DEFS_H__

#include <QList>
#include <QDateTime>
#include <QMetaType>

typedef struct
{
	QString id;
	QString name;
	QString icon;	
	bool enabled;
} TvChannel;
Q_DECLARE_METATYPE(TvChannel)
//
class PairIcon
{
public:
	PairIcon(QString s=QString(), QPixmap p=QPixmap());
	QString icon() { return m_icon; }
	QPixmap pixmap() { return m_pixmap; }
private:
	QString m_icon;
	QPixmap m_pixmap;
};
Q_DECLARE_METATYPE(PairIcon)
//
typedef struct
{
	QDateTime start;
	QDateTime stop;
	QString channel;
	QString channelName;
	QString title;
	QString subTitle;
	QStringList category;
	QStringList resume;
	QString story;
	QString aspect;
	QString credits;
	QString director;
	QString star;
	QString icon;
} TvProgram;
Q_DECLARE_METATYPE(TvProgram)

#endif // __DEFS_H__
