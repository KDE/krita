/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_STROKE_SHORTCUT_H
#define __KIS_STROKE_SHORTCUT_H

#include "kis_abstract_shortcut.h"

class QMouseEvent;
class QPointF;

/**
 * This class represents a shortcut that starts an action that can
 * involve pressing the mouse button and, probably, moving the cursor.
 *
 * The stroke shortcut may be represented as a simple state machine:
 * It transits between 3 states:
 *
 * Idle <-> Ready <-> Running
 *
 * The possibility of transition between Idle <-> Ready is defined
 * with a matchReady() method. The transition Ready <-> Running is
 * defined by matchBegin(). The Ready state is used for showing the
 * user the cursor of the upcoming action and the Running state shows
 * that the action linked to the shortcut should be activated.
 */

class KRITAUI_EXPORT KisStrokeShortcut : public KisAbstractShortcut
{
public:
    KisStrokeShortcut(KisAbstractInputAction *action, int index);
    ~KisStrokeShortcut() override;

    int priority() const override;

    /**
     * Sets the configuration for this shortcut
     *
     * \param modifiers keyboard keys that should be holded
     *                  for the shortcut to trigger
     * \param buttons mouse buttons that should be pressed (simultaneously)
     *                for the shortcut to trigger
     */
    void setButtons(const QSet<Qt::Key> &modifiers,
                    const QSet<Qt::MouseButton> &buttons);

    /**
     * Reports whether all but one buttons and modifiers are pressed
     * for the shortcut. Such configuration means that the input manager
     * can show the user that pressing the mouse button will start some
     * action. This can be done with, e.g. changing the cursor.
     */

    bool matchReady(const QSet<Qt::Key> &modifiers,
                    const QSet<Qt::MouseButton> &buttons);
    /**
     * Reports whether the shortcut can transit form the "Ready"
     * to "Running" state. It means that the last button of the shortcut
     * is pressed.
     */
    bool matchBegin(Qt::MouseButton button);

    QMouseEvent fakeEndEvent(const QPointF &localPos) const;

private:
    class Private;
    Private * const m_d;
};

#endif /* __KIS_STROKE_SHORTCUT_H */
