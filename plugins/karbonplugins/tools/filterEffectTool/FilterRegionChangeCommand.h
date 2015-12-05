/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FILTERREGIONCHANGECOMMAND_H
#define FILTERREGIONCHANGECOMMAND_H

#include <kundo2command.h>
#include <QRectF>

class KoShape;
class KoFilterEffect;

/// A command to change the region of a filter effect
class FilterRegionChangeCommand : public KUndo2Command
{
public:
    /**
     * Creates new command to change filter region of a filter effect
     * @param effect the effect to change the filter region of
     * @param filterRegion the new filter region to set
     * @param shape the shape the filter effect is applied to
     * @param parent the parent undo command
     */
    explicit FilterRegionChangeCommand(KoFilterEffect *effect, const QRectF &filterRegion, KoShape *shape = 0, KUndo2Command *parent = 0);

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KoFilterEffect *m_effect;  ///< the filter effect we are working on
    QRectF m_oldRegion; ///< the old filter region
    QRectF m_newRegion; ///< the new filter region
    KoShape *m_shape;   ///< the shape the effect is applied to, might be zero
};

#endif // FILTERREGIONCHANGECOMMAND_H
