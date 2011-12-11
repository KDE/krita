/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef PICTURESHAPE_H
#define PICTURESHAPE_H

#include <QPixmap>
#include <QImage>
#include <QRunnable>
#include <QMutex>

#include <KoTosContainer.h>
#include <KoFrameShape.h>
#include <SvgShape.h>

#include "ClippingRect.h"

#define PICTURESHAPEID "PictureShape"

class KoImageData;
class KoImageCollection;
class RenderQueue;
class KJob;
class PictureShape;

namespace _Private
{
    class PictureShapeProxy: public QObject
    {
        Q_OBJECT
    public:
        PictureShapeProxy(PictureShape *p):
            m_pictureShape(p) { }

    public slots:
        void setImage(const QString& key, const QImage& image);

    private:
        PictureShape *m_pictureShape;
    };

    class PixmapScaler: public QObject, public QRunnable
    {
        Q_OBJECT
    public:
        PixmapScaler(PictureShape *pictureShape, const QSize& pixmapSize);
        virtual void run();
        
    signals:
        void finished(const QString&, const QImage&);
        
    private:
        QSize   m_size;
        QImage  m_image;
        quint64 m_imageKey;
    };
}

class PictureShape : public KoTosContainer, public KoFrameShape, public SvgShape
{
    friend class _Private::PixmapScaler;
    friend class _Private::PictureShapeProxy;
    
public:
    enum ColorMode {
        Standard,
        Greyscale,
        Mono,
        Watermark
    };
    
    PictureShape();
    
    // reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    // reimplemented
    virtual void waitUntilReady(const KoViewConverter &converter, bool asynchronous) const;
    // reimplemented from SvgShape
    virtual bool saveSvg(SvgSavingContext &context);
    // reimplemented from SvgShape
    virtual bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context);
    /**
     * Get the collection used in the shape.
     */
    KoImageCollection *imageCollection() const;
    KoImageData *imageData() const;
    ColorMode colorMode() const;
    QRectF cropRect() const;
    bool isPictureInProportion() const;

    void setImageCollection(KoImageCollection *collection) { m_imageCollection = collection; }
    void setCropRect(const QRectF& rect);
    void setColorMode(ColorMode mode);

protected:
    virtual bool loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual QString saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const;
    virtual void loadStyle(const KoXmlElement& element, KoShapeLoadingContext& context);

private:
    QSize calcOptimalPixmapSize(const QSizeF& shapeSize, const QSizeF& imageSize) const;
    ClippingRect parseClippingRectString(QString string) const;

private:
    KoImageCollection *m_imageCollection;
    mutable QImage m_printQualityImage;
    ColorMode m_mode;
    ClippingRect m_clippingRect;
    _Private::PictureShapeProxy m_proxy;
};

class LoadWaiter : public QObject
{
    Q_OBJECT
public:
    LoadWaiter(PictureShape *shape) : m_pictureShape(shape) { }

public slots:
    void setImageData(KJob *job);

private:
    PictureShape *m_pictureShape;
};

#endif
