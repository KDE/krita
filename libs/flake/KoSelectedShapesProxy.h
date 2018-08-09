/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOSELECTEDSHAPESPROXY_H
#define KOSELECTEDSHAPESPROXY_H

#include <QObject>
#include "kritaflake_export.h"

class KoSelection;
class KoShapeLayer;

/**
 * @brief The KoSelectedShapesProxy class is a special interface of KoCanvasBase to
 * have a stable connection to shape selection signals in an environment when the
 * active shape manager can switch (e.g. when shape layers are switched in Krita)
 */

class KRITAFLAKE_EXPORT KoSelectedShapesProxy : public QObject
{
    Q_OBJECT
public:
    explicit KoSelectedShapesProxy(QObject *parent = 0);

    /**
     * Returns a pointer to a currently active shape selection. Don't connect to the
     * selection, unless you really know what you are doing. Use the signals provided
     * by KoSelectedShapesProxy itself. They are guaranteed to be valid all the time.
     */
    virtual KoSelection *selection() = 0;

    /**
     * @brief The shape wants to edited. This is used when a shape is passed
     * between two different tools. This notifies the new tool that it needs
     * to enter some extra edit mode.
     *
     */
    bool isRequestingToBeEdited();
    void setRequestingToBeEdited(bool value);

Q_SIGNALS:

    // forwards a corresponding signal of KoShapeManager
    void selectionChanged();

    // forwards a corresponding signal of KoShapeManager
    void selectionContentChanged();

    // forwards a corresponding signal of KoSelection
    void currentLayerChanged(const KoShapeLayer *layer);

private:
    bool m_isRequestingEditing = false;
};

#endif // KOSELECTEDSHAPESPROXY_H
