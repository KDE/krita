/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef KOUNAVAILSHAPE_H
#define KOUNAVAILSHAPE_H


// Calligra
#include <KoFrameShape.h>
#include <KoShapeContainer.h>


class QPainter;

#define KoUnavailShape_SHAPEID "UnavailShapeID"


/**
 * The KoUnavailShape is a frame shape that takes care of all frame
 * based objects that are not handled by any of the shape plugins.
 *
 * The KoUnavailShape stores the data associated with the frame, even
 * if this data is stored in embedded files inside the ODF container.
 * To the user, it shows an empty frame with an indicator that there
 * is an object here. If a preview of some type is available, the
 * Unavail shape will attempt to load them read-only, in a fallback
 * manner and show them in that frame. If no shape is found at all
 * which supports any of the fallbacks, it will show a placeholder
 * graphic, to indicate the fact that it is an unknown item.
 *
 * The KoUnavailShape always has to be present, and is the only shape
 * that is not implemented as a plugin.
 */
class KoUnavailShape : public KoFrameShape, public KoShapeContainer {
public:
    KoUnavailShape();
    ~KoUnavailShape() override;

    // Inherited methods

    /// reimplemented
    void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext) override;
    /// reimplemented from KoShapeContainer
    void paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext) override;
    /// reimplemented
    void saveOdf(KoShapeSavingContext & context) const override;
    /// reimplemented
    bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context ) override;
    /// Load the real contents of the frame shape.
    bool loadOdfFrameElement(const KoXmlElement& frameElement,
                                     KoShapeLoadingContext& context) override;

private:
    class Private;
    Private * const d;
};

#endif
