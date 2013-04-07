/* This file is part of the KDE project
   Copyright (c) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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
#ifndef KoDockWidgetTitleBar_p_h
#define KoDockWidgetTitleBar_p_h

#include "KoDockWidgetTitleBar.h"
#include "KoDockWidgetTitleBarButton.h"

#include <KoIcon.h>

#include <kdebug.h>

#include <QAbstractButton>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>

class KoDockWidgetTitleBar::Private
{
public:
    Private(KoDockWidgetTitleBar* thePublic)
        : thePublic(thePublic),
            openIcon(thePublic->style()->standardIcon(QStyle::SP_TitleBarShadeButton)),
            closeIcon(thePublic->style()->standardIcon(QStyle::SP_TitleBarUnshadeButton)),
            textVisibilityMode(KoDockWidgetTitleBar::FullTextAlwaysVisible),
            preCollapsedWidth(-1)
    {
        if (openIcon.isNull())
            openIcon = koIcon("arrow-down");
        if (closeIcon.isNull())
            closeIcon = koIcon("arrow-right");
    }
    KoDockWidgetTitleBar* thePublic;
    KIcon openIcon, closeIcon;
    QAbstractButton* closeButton;
    QAbstractButton* floatButton;
    QAbstractButton* collapseButton;

    KoDockWidgetTitleBar::TextVisibilityMode textVisibilityMode;

    int preCollapsedWidth;

    void toggleFloating();
    void toggleCollapsed();
    void featuresChanged(QDockWidget::DockWidgetFeatures features);
};
#endif