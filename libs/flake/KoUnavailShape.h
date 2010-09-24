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


// KOffice
#include <KoShape.h>
#include <KoFrameShape.h>


class QPainter;

#define KoUnavailShape_SHAPEID "UnavailShapeID"


/**
 * The KoUnavailShape is a frame shape that takes care of all frame
 * based objects that are not handled by any of the shape plugins.
 *
 * The KoUnavailShape stores the data associated with the frame, even
 * if this data is stored in embedded files inside the ODF container.
 * To the user, it shows an empty frame with an indicator that there
 * is an object here.  In the future it may try to dig out any preview
 * picture embedded into the object.
 *
 * The KoUnavailShape always has to be present, and is the only shape
 * that is not implemented as a plugin.
 */
class KoUnavailShape : public KoShape, public KoFrameShape {
public:
    KoUnavailShape();
    virtual ~KoUnavailShape();

    // Inherited methods

    /// reimplemented
    void paint(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented
    void paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas);
    /// reimplemented
    virtual void saveOdf(KoShapeSavingContext & context) const;
    /// reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );
    /// Load the real contents of the frame shape.
    virtual bool loadOdfFrameElement(const KoXmlElement& frameElement,
                                     KoShapeLoadingContext& context);

private:
    class Private;
    Private * const d;

    void draw(QPainter &painter) const;
    void drawNull(QPainter &painter) const;
};

#endif
