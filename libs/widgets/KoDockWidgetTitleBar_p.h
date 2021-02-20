/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>

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
            lockable(true),
            textVisibilityMode(KoDockWidgetTitleBar::FullTextAlwaysVisible),
            locked(false)
    {
    }

    KoDockWidgetTitleBar* thePublic;
    QIcon lockIcon, floatIcon, removeIcon; 
    QAbstractButton* closeButton;
    QAbstractButton* floatButton;
    QAbstractButton* lockButton;
    bool lockable;
    KoDockWidgetTitleBar::TextVisibilityMode textVisibilityMode;
    bool locked;
    QDockWidget::DockWidgetFeatures features;

    void toggleFloating();
    void topLevelChanged(bool topLevel);
    void featuresChanged(QDockWidget::DockWidgetFeatures features);
    void updateIcons();
};
#endif
