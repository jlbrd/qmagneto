/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2009  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify.
*  But to reuse the source code, permission of the author is essential. Without authorization, code reuse is prohibited.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://code.google.com/p/qmagneto/
*
*/

#include "listnow.h"
#include "xmldefaulthandler.h"
#include "mainwindowimpl.h"
#include <QPainter>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
ListNow::ListNow( QWidget *main )
        : QListWidget(main)
{
    m_main = (MainWindowImpl *)main;
}
