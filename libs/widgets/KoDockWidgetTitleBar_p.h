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

#include <WidgetsDebug.h>

#include <QAbstractButton>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>

class Q_DECL_HIDDEN KoDockWidgetTitleBar::Private
{
public:
    Private(KoDockWidgetTitleBar* thePublic)
        : thePublic(thePublic),
            collapsable(true),
            collapsableSet(true),
            lockable(true),
            textVisibilityMode(KoDockWidgetTitleBar::FullTextAlwaysVisible),
            preCollapsedWidth(-1),
            locked(false)
    {
    }

    KoDockWidgetTitleBar* thePublic;
    QIcon openIcon, closeIcon, lockIcon, floatIcon, removeIcon; // close/open are for collapsing
    QAbstractButton* closeButton;
    QAbstractButton* floatButton;
    QAbstractButton* collapseButton;
    bool collapsable;
    bool collapsableSet;
    QAbstractButton* lockButton;
    bool lockable;
    KoDockWidgetTitleBar::TextVisibilityMode textVisibilityMode;
    int preCollapsedWidth;
    bool locked;
    QDockWidget::DockWidgetFeatures features;

    void toggleFloating();
    void toggleCollapsed();
    void topLevelChanged(bool topLevel);
    void featuresChanged(QDockWidget::DockWidgetFeatures features);
    void updateIcons();
};
#endif
