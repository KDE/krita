/* This file is part of the KDE project
 *
 * Copyright (C) 2009-2010 Inge Wallin <inge@lysator.liu.se>
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


#ifndef VECTORSHAPE_H
#define VECTORSHAPE_H


// KOffice
#include <KoShape.h>
#include <KoFrameShape.h>


#define DEBUG_VECTORSHAPE 0


class QPainter;

#define VectorShape_SHAPEID "VectorShapeID"


class VectorShape : public KoShape, public KoFrameShape {
public:
    VectorShape();
    virtual ~VectorShape();

    // reimplemented methods.

    /// reimplemented from KoShape
    void paint(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented from KoShape
    virtual void saveOdf(KoShapeSavingContext & context) const;
    /// reimplemented from KoShape
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );
    /// Load the real contents of the frame shape.  reimplemented  from KoFrameShape
    virtual bool loadOdfFrameElement(const KoXmlElement& frameElement,
                                     KoShapeLoadingContext& context);

    // Methods specific to the vector shape.

    void  setVectorBytes( char *bytes, int size, bool takeOwnership );
    char *vectorBytes();
    int   vectorSize();

private:
    // Type of vector file. Add here when we get support for more.
    enum VectorType {
        VectorTypeNone,             // Uninitialized
        VectorTypeWmf,              // Windows MetaFile
        VectorTypeEmf               // Extended MetaFile
        // ... more here later
    };

    void draw(QPainter &painter) const;
    void drawNull(QPainter &painter) const;
    void drawWmf(QPainter &painter) const;
    void drawEmf(QPainter &painter) const;

    bool isWmf() const;
    bool isEmf() const;

    // Member variables

    VectorType  m_type;

    char  *m_bytes;       // Use char* instead of void* because of QByteArray
    int    m_size;
    bool   m_ownsBytes;   // True if the data is owned by this shape.
};

#endif
