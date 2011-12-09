/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include <KoTosContainer.h>
#include <KoFrameShape.h>
#include <SvgShape.h>

#define PICTURESHAPEID "PictureShape"

class KoImageData;
class KoImageCollection;
class RenderQueue;
class KJob;

class PictureShape : public KoTosContainer, public KoFrameShape, public SvgShape
{
    struct ClippingRect
    {
        ClippingRect():
            top(0), right(1), bottom(1), left(0),
            uniform(true), inverted(false) { }

        ClippingRect(const ClippingRect& rect):
            top(rect.top), right(rect.right), bottom(rect.bottom), left(rect.left),
            uniform(rect.uniform), inverted(rect.inverted) { }
        
        void scale(const QSizeF& size, bool isUniform)
        {
            top    *= size.height();
            right  *= size.width();
            bottom *= size.height();
            left   *= size.width();
            uniform = isUniform;
        }

        void normalize(const QSizeF& size)
        {
            if (!uniform) {
                scale(QSizeF(1.0/size.width(), 1.0/size.height()), true);
            }

            if(inverted) {
                right    = 1.0 - right;
                bottom   = 1.0 - bottom;
                inverted = false;
            }
        }

        void setRect(const QRectF& rect, bool isUniform)
        {
            top      = rect.top();
            right    = rect.right();
            bottom   = rect.bottom();
            left     = rect.left();
            uniform  = isUniform;
            inverted = false;
        }

        qreal  width()  const { return right - left; }
        qreal  height() const { return bottom - top; }
        QRectF toRect() const { return QRectF(left, top, width(), height()); }

        qreal top;
        qreal right;
        qreal bottom;
        qreal left;
        bool uniform;
        bool inverted;
    };
    
public:
    enum PictureMode {
        Standard,
        Greyscale,
        Mono,
        Watermark
    };

    PictureShape();
    virtual ~PictureShape();

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
    PictureMode mode() const;
    QRectF cropRect() const;

    void setImageCollection(KoImageCollection *collection) { m_imageCollection = collection; }
    void setCropRect(const QRectF& rect);
    void setMode(PictureMode mode);

protected:
    virtual bool loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual QString saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const;
    virtual void loadStyle(const KoXmlElement& element, KoShapeLoadingContext& context);

private:
    QSize calcOptimalPixmapSize(const QSizeF& shapeSize, const QSizeF& imageSize) const;
    ClippingRect parseClippingRectString(QString string) const;

private:
    KoImageCollection *m_imageCollection;
    RenderQueue *m_renderQueue;
    mutable QImage m_printQualityImage;
    PictureMode m_mode;
    ClippingRect m_clippingRect;
};

class RenderQueue : public QObject
{
    Q_OBJECT
public:
    RenderQueue(PictureShape *shape) : m_pictureShape(shape) { }

    void addSize(const QSize &size) { m_wantedImageSize << size; }

public slots:
    void renderImage();
    void updateShape();

private:
    KoShape *m_pictureShape;
    QList<QSize> m_wantedImageSize;
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
