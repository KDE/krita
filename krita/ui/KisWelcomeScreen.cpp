/* This file is part of the KDE project
   Copyright (C) 2015 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "KisWelcomeScreen.h"

#include <QLabel>
#include <QHBoxLayout>

KisWelcomeScreen::KisWelcomeScreen(QWidget *parent) : QWidget(parent)
{

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    m_lblHelp = new QLabel(this);
    m_lblHelp->setTextFormat(Qt::RichText);
    layout->addWidget(m_lblHelp);

    m_lblNews = new QLabel(this);
    m_lblNews->setTextFormat(Qt::RichText);
    layout->addWidget(m_lblHelp);

    m_lblForum = new QLabel(this);
    m_lblForum->setTextFormat(Qt::RichText);
    layout->addWidget(m_lblForum);

    m_newsModel.addFeed(QLatin1String("https://krita.org/feed/"));
    m_forumModel.addFeed(QLatin1String("http://forum.kde.org/search.php?keywords=&terms=all&author=&tags=&sv=0&fid%5B%5D=136&fid%5B%5D=137&fid%5B%5D=138&fid%5B%5D=139&fid%5B%5D=156&sc=1&sf=all&sk=t&sd=d&st=0&feed_type=RSS2.0&feed_style=HTML&countlimit=100&submit=Search"));
}

