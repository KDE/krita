/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
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


class QPainter;

#define VectorShape_SHAPEID "VectorShapeID"


class VectorShape : public KoShape, public KoFrameShape {
public:
    VectorShape();
    virtual ~VectorShape();

    /// reimplemented to be empty (this shape is fully non-printing)
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

    // Methods (none so far)
    // ...

    void setPrintable(bool on);
    bool printable() const { return m_printable; }

    void  setVectorBytes( char *bytes, int size, bool takeOwnership );
    char *vectorBytes();
    int   vectorSize();

private:
    // Type of vector file. Add here when we get support for more.
    enum VectorType {
        VectorTypeNone,             // Uninitialized
        VectorTypeWmf,              // Windows MetaFile
        VectorTypeEmf               // Extended MetaFile
    };

    void draw(QPainter &painter) const;
    void drawNull(QPainter &painter) const;
    void drawWmf(QPainter &painter) const;
    void drawEmf(QPainter &painter) const;

    bool isWmf() const;
    bool isEmf() const;

    VectorType  m_type;
    char  *m_bytes;       // Use char* instead of void* because of QByteArray
    int    m_size;
    bool   m_ownsBytes;

    bool   m_printable;
};

#endif
