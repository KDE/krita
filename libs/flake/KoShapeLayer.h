/* This file is part of the KDE project
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef __KOSHAPELAYER_H__
#define __KOSHAPELAYER_H__

#include "KoShapeContainer.h"
#include "flake_export.h"

class KoShapeLayerPrivate;

/**
 * Provides arranging shapes into layers.
 * This makes it possible to have a higher key of a number of objects
 * in a document.
 * A layer is always invisible and unselectable.
 */
class FLAKE_EXPORT KoShapeLayer : public KoShapeContainer
{
public:
    /// The default constructor
    KoShapeLayer();
    /**
     * Constructor with custom model
     * @param model the custom modem
     */
    KoShapeLayer(KoShapeContainerModel *model);

    /**
     * Empty implementation, as the layer itself is not visible
     */
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);
    virtual bool hitTest(const QPointF &position) const;
    virtual QRectF boundingRect() const;
    virtual void saveOdf(KoShapeSavingContext & context) const;
    virtual bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context);

private:
    Q_DECLARE_PRIVATE(KoShapeLayer)
};

#endif // __KOSHAPELAYER_H__

