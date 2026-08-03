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
class KRITAUI_EXPORT KisTouchShortcut : public KisAbstractShortcut
{
        using GestureAction = KisShortcutConfiguration::GestureAction;

    public:
        KisTouchShortcut(KisAbstractInputAction* action, int index, GestureAction type);
        ~KisTouchShortcut() override;

        int priority() const override;
        bool isHoldType() const;
        bool isAvailable(KisInputActionGroupsMask mask) const override;

        void setMinimumTouchPoints( int min );
        void setMaximumTouchPoints( int max );

        void setIsTouchPainting(bool value);

        bool matchTapType(const QTouchEvent *event, Qt::TouchPointStates allowedStates);
        bool matchDragType(const QTouchEvent *event, Qt::TouchPointStates allowedStates);
        bool matchHoldType(const QTouchEvent *event, Qt::TouchPointStates allowedStates);
        bool matchTouchPoint(const QTouchEvent *event, Qt::TouchPointStates allowedStates);

        qreal minDragThreshold() const;
        void setMinDragThreshold(qreal value);

        static inline Qt::TouchPointStates allTouchStates() {
            return Qt::TouchPointStationary | Qt::TouchPointPressed | Qt::TouchPointMoved | Qt::TouchPointReleased;
        }

        static inline Qt::TouchPointStates pressedOnlyTouchStates() {
            return Qt::TouchPointStationary | Qt::TouchPointPressed | Qt::TouchPointMoved;
        }

        static int countTouchPoints(const QTouchEvent *event, Qt::TouchPointStates allowedStates);

        static qreal touchDragDistance(const QTouchEvent *event, Qt::TouchPointStates allowedStates);

    private:
        class Private;
        Private * const d;
};

#endif // KISTOUCHSHORTCUT_H
