/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PAINTING_ASSISTANT_H_
#define _KIS_PAINTING_ASSISTANT_H_

#include <QString>
#include <QPointF>

#include <krita_export.h>

/**
 * A KisPaintingAssistant is an object that assist the drawing on the canvas.
 * With this class you can implement virtual equivalent to ruler or compas.
 */
class KRITAUI_EXPORT KisPaintingAssistant
{
public:
    KisPaintingAssistant(const QString& id, const QString& name);
    virtual ~KisPaintingAssistant();
    const QString& id() const;
    const QString& name() const;
    /**
     * Adjust the position given in parameter.
     * @param point the coordinates in point in the document reference
     */
    virtual QPointF adjustPosition(const QPointF& point) = 0;
    /**
     * @return the current painting assistant, or null if none.
     */
    static KisPaintingAssistant* currentAssistant();
    /**
     * Set the current painting assistant.
     *
     * This function do not take ownership of the pointer,
     * the responsibility of deleting the KisPaintingAssistant
     * is left to the caller of this function.
     *
     * Call with a null pointer to make freehand painting without
     * any assistant.
     */
    static void setCurrentAssistant(KisPaintingAssistant*);
private:
    struct Private;
    Private* const d;
};

#endif
