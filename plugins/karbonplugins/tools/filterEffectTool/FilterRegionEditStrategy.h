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

#ifndef FILTERREGIONEDITSTRATEGY_H
#define FILTERREGIONEDITSTRATEGY_H

#include <KoInteractionStrategy.h>
#include "KarbonFilterEffectsTool.h"

#include <QRectF>

class KoShape;
class KoFilterEffect;

class FilterRegionEditStrategy : public KoInteractionStrategy
{
public:
    FilterRegionEditStrategy(KoToolBase *parent, KoShape *shape, KoFilterEffect *effect, KarbonFilterEffectsTool::EditMode mode);

    // reimplemented from KoInteractionStrategy
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    // reimplemented from KoInteractionStrategy
    virtual KUndo2Command *createCommand();
    // reimplemented from KoInteractionStrategy
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);
    // reimplemented from KoInteractionStrategy
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

private:
    KoFilterEffect *m_effect;
    KoShape *m_shape;
    QRectF m_sizeRect;
    QRectF m_filterRect;
    KarbonFilterEffectsTool::EditMode m_editMode;
    QPointF m_lastPosition;
};

#endif // FILTERREGIONEDITSTRATEGY_H
