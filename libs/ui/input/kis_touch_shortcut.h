/*
 *  This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef KISTOUCHSHORTCUT_H
#define KISTOUCHSHORTCUT_H

#include "kis_abstract_shortcut.h"
#include "kis_shortcut_configuration.h"

class QTouchEvent;
/**
 * @brief The KisTouchShortcut class only handles touch gestures
 * it _does not_ handle tool invocation i.e painting (which is being
 * handled in KisShortcutMatcher).
 */
class KisTouchShortcut : public KisAbstractShortcut
{
        using GestureAction = KisShortcutConfiguration::GestureAction;

    public:
        KisTouchShortcut(KisAbstractInputAction* action, int index, GestureAction type);
        ~KisTouchShortcut() override;

        int priority() const override;

        void setMinimumTouchPoints( int min );
        void setMaximumTouchPoints( int max );

        bool match( QTouchEvent* event );

    private:
        class Private;
        Private * const d;
};

#endif // KISTOUCHSHORTCUT_H
