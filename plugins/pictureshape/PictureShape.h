/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#include <QPainterPath>
#include <QPixmap>
#include <QImage>
#include <QRunnable>

#include <KoTosContainer.h>
#include <KoFrameShape.h>
#include <SvgShape.h>

#include "ClippingRect.h"

#define PICTURESHAPEID "PictureShape"

class KoImageData;
class KoImageCollection;
class KJob;
class PictureShape;
class KoPathShape;
class KoClipPath;

namespace _Private
{
    /**
     * This class acts as a proxy for the PictureShape class
     * since it is not possible to add slots to it
     * (MOC always complains)
     */
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

    /**
     * This class will scale an image to a given size.
     * Instances of this class can be executed in a thread pool
     * therefore the scaling process can be done in the background
     */
    class PixmapScaler: public QObject, public QRunnable
    {
        Q_OBJECT
    public:
        PixmapScaler(PictureShape *pictureShape, const QSize &pixmapSize);
        virtual void run();

    signals:
        void finished(const QString &, const QImage &);

    private:
        QSize m_size;
        QImage m_image;
        quint64 m_imageKey;
    };

    /**
     * This method will create an outline path out of the image
     */
    QPainterPath generateOutline(const QImage &imageIn, int treshold = 20);
}


class PictureShape : public KoTosContainer, public KoFrameShape, public SvgShape
{
    friend class _Private::PixmapScaler;
    friend class _Private::PictureShapeProxy;

public:
    // Odf 1.2: 20.313  style:mirror
    // The value could be 0, or a combination of one of the Horizontal* and/or Vertical
    // separated by whitespace.
    enum MirrorMode {
        MirrorNone             = 0x00,
        MirrorHorizontal       = 0x01,
        MirrorHorizontalOnEven = 0x02,
        MirrorHorizontalOnOdd  = 0x04,
        MirrorVertical         = 0x08,

        MirrorMask = 0x0f      // Only used as a mask, never as a value.
    };

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
    virtual QPainterPath shadowOutline() const;
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
    QFlags<MirrorMode> mirrorMode() const;
    ColorMode colorMode() const;
    QRectF cropRect() const;
    bool isPictureInProportion() const;

    void setImageCollection(KoImageCollection *collection) { m_imageCollection = collection; }
    void setCropRect(const QRectF& rect);
    void setMirrorMode(QFlags<MirrorMode> mode);
    void setColorMode(ColorMode mode);
    KoClipPath *generateClipPath();


protected:
    virtual bool loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual QString saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const;
    virtual void loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context);

private:
    QSize calcOptimalPixmapSize(const QSizeF &shapeSize, const QSizeF &imageSize) const;
    ClippingRect parseClippingRectString(const QString &string) const;

private:
    KoImageCollection *m_imageCollection;
    mutable QImage m_printQualityImage;
    mutable QSizeF m_printQualityRequestedSize;

    QFlags<MirrorMode> m_mirrorMode;
    ColorMode m_colorMode;
    ClippingRect m_clippingRect;

    _Private::PictureShapeProxy m_proxy;
};

#endif
