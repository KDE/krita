/* This file is part of the KDE project
   Copyright (c) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
