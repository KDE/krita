/* This file is part of the KDE project
 *
 * Copyright (C) 2009-2011 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
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

// Qt
#include <QByteArray>
#include <QCache>
#include <QSize>
#include <QRunnable>
#include <QMutex>

// Calligra
#include <KoShape.h>
#include <KoFrameShape.h>

#define DEBUG_VECTORSHAPE 0

class QPainter;
class VectorShape;

#define VectorShape_SHAPEID "VectorShapeID"

class VectorShape : public QObject, public KoShape, public KoFrameShape
{
    Q_OBJECT
public:
    // Type of vector file. Add here when we get support for more.
    enum VectorType {
        VectorTypeNone,         // Uninitialized
        VectorTypeWmf,          // Windows MetaFile
        VectorTypeEmf,          // Extended MetaFile
        VectorTypeSvm,          // StarView Metafile
        VectorTypeSvg           // Scalable Vector Graphics
        // ... more here later
    };

    VectorShape();
    virtual ~VectorShape();

    // reimplemented methods.

    /// reimplemented from KoShape
    void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
    /// reimplemented from KoShape
    virtual void saveOdf(KoShapeSavingContext &context) const;
    /// reimplemented from KoShape
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    /// Load the real contents of the frame shape.  reimplemented  from KoFrameShape
    virtual bool loadOdfFrameElement(const KoXmlElement &frameElement,
                                     KoShapeLoadingContext &context);
    /// reimplemented from KoShape
    virtual void waitUntilReady(const KoViewConverter &converter, bool asynchronous = true) const;

    // Methods specific to the vector shape.
    QByteArray  compressedContents() const;
    VectorType vectorType() const;
    void  setCompressedContents(const QByteArray &newContents, VectorType vectorType);

    static VectorShape::VectorType vectorType(const QByteArray &contents);

private Q_SLOTS:
    void renderFinished(QSize boundingSize, QImage *image);

private:
    static bool isWmf(const QByteArray &bytes);
    static bool isEmf(const QByteArray &bytes);
    static bool isSvm(const QByteArray &bytes);
    static bool isSvg(const QByteArray &bytes);

    // Member variables
    mutable VectorType  m_type;
    mutable QByteArray  m_contents;
    mutable bool m_isRendering;
    mutable QMutex m_mutex;
    QCache<int, QImage> m_cache;

    QImage *render(const KoViewConverter &converter, bool asynchronous, bool useCache) const;
};

class RenderThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    RenderThread(const QByteArray &contents, VectorShape::VectorType type,
                 const QSizeF &size, const QSize &boundingSize, qreal zoomX, qreal zoomY);
    virtual ~RenderThread();
    virtual void run();
Q_SIGNALS:
    void finished(QSize boundingSize, QImage *image);
private:
    void draw(QPainter &painter);
    void drawNull(QPainter &painter) const;
    void drawWmf(QPainter &painter) const;
    void drawEmf(QPainter &painter) const;
    void drawSvm(QPainter &painter) const;
    void drawSvg(QPainter &painter) const;
private:
    const QByteArray m_contents;
    VectorShape::VectorType m_type;
    QSizeF m_size;
    QSize m_boundingSize;
    qreal m_zoomX, m_zoomY;
};

#endif
