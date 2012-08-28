/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2012 C.Boemann <cbo@boemann.dk>
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

#include "PictureShape.h"
#include "filters/GreyscaleFilterEffect.h"
#include "filters/MonoFilterEffect.h"
#include "filters/WatermarkFilterEffect.h"

#include <KoOdfWorkaround.h>
#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>
#include <KoGenStyle.h>
#include <KoFilterEffectStack.h>
#include <KoClipPath.h>
#include <SvgSavingContext.h>
#include <SvgLoadingContext.h>
#include <SvgUtil.h>
#include <KoPathShape.h>

#include <KDebug>
#include <KJob>
#include <KIO/Job>

#include <QPainter>
#include <QTimer>
#include <QPixmapCache>
#include <QThreadPool>
#include <QImage>
#include <QColor>

QString generate_key(qint64 key, const QSize & size)
{
    return QString("%1-%2-%3").arg(key).arg(size.width()).arg(size.height());
}

// ----------------------------------------------------------------- //

_Private::PixmapScaler::PixmapScaler(PictureShape *pictureShape, const QSize &pixmapSize):
    m_size(pixmapSize)
{
    m_image = pictureShape->imageData()->image();
    m_imageKey = pictureShape->imageData()->key();
    connect(this, SIGNAL(finished(QString,QImage)), &pictureShape->m_proxy, SLOT(setImage(QString,QImage)));
}

void _Private::PixmapScaler::run()
{
    QString key = generate_key(m_imageKey, m_size);

    m_image = m_image.scaled(
        m_size.width(),
        m_size.height(),
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
    );

    emit finished(key, m_image);
}

// ----------------------------------------------------------------- //

void _Private::PictureShapeProxy::setImage(const QString &key, const QImage &image)
{
    QPixmapCache::insert(key, QPixmap::fromImage(image));
    m_pictureShape->update();
}

// ----------------------------------------------------------------- //

QPainterPath _Private::generateOutline(const QImage &imageIn, int treshold)
{
    int leftArray[100];
    int rightArray[100];

    QImage image = imageIn.scaled(QSize(100, 100));

    QPainterPath path;

    for (int y = 0; y < 100; y++) {
        leftArray[y] = -1;
        for (int x = 0; x < 100; x++) {
            int a = qAlpha(image.pixel(x,y));
            if (a > treshold) {
                leftArray[y] = x;
                break;
            }
        }
    }
    for (int y = 0; y < 100; y++) {
        rightArray[y] = -1;
        if (leftArray[y] != -1) {
            for (int x = 100-1; x >= 0; x--) {
                int a = qAlpha(image.pixel(x,y));
                if (a > treshold) {
                    rightArray[y] = x;
                    break;
                }
            }
        }
    }

    // Now we know the outline let's make a path out of it
    bool first = true;
    for (int y = 0; y < 100; y++) {
        if (rightArray[y] != -1) {
            if (first) {
                path.moveTo(rightArray[y] / 99.0, y / 99.0);
                first = false;
            } else {
                path.lineTo(rightArray[y] / 99.0, y / 99.0);
            }
        }
    }
    if (first) {
        // Completely empty
        return path;
    }

    for (int y = 100-1; y >= first; y--) {
        if (leftArray[y] != -1) {
            path.lineTo(leftArray[y] / 99.0, y / 99.0);
        }
    }
    return path;
}

// ----------------------------------------------------------------- //

PictureShape::PictureShape()
    : KoFrameShape(KoXmlNS::draw, "image")
    , m_imageCollection(0)
    , m_mirrorMode(MirrorNone)
    , m_colorMode(Standard)
    , m_proxy(this)
{
    setKeepAspectRatio(true);
    KoFilterEffectStack * effectStack = new KoFilterEffectStack();
    effectStack->setClipRect(QRectF(0, 0, 1, 1));
    setFilterEffectStack(effectStack);
}

KoImageData* PictureShape::imageData() const
{
    return qobject_cast<KoImageData*>(userData());
}

QRectF PictureShape::cropRect() const
{
    return m_clippingRect.toRect();
}

bool PictureShape::isPictureInProportion() const
{
    QSizeF clippingRectSize(
        imageData()->imageSize().width() * m_clippingRect.width(),
        imageData()->imageSize().height() * m_clippingRect.height()
    );

    qreal shapeAspect = size().width() / size().height();
    qreal rectAspect = clippingRectSize.width() / clippingRectSize.height();

    return qAbs(shapeAspect - rectAspect) <= 0.025;
}

void PictureShape::setCropRect(const QRectF& rect)
{
    m_clippingRect.setRect(rect, true);
    update();
}

QSize PictureShape::calcOptimalPixmapSize(const QSizeF& shapeSize, const QSizeF& imageSize) const
{
    qreal imageAspect = imageSize.width() / imageSize.height();
    qreal shapeAspect = shapeSize.width() / shapeSize.height();
    qreal scale = 1.0;

    if (shapeAspect > imageAspect) {
        scale = shapeSize.width()  / imageSize.width()  / m_clippingRect.width();
    }
    else {
        scale = shapeSize.height() / imageSize.height() / m_clippingRect.height();
    }

    scale = qMin<qreal>(1.0, scale); // prevent upscaling
    return (imageSize * scale).toSize();
}

ClippingRect PictureShape::parseClippingRectString(const QString &originalString) const
{
    ClippingRect rect;
    QString string = originalString.trimmed();

    if (string.startsWith(QLatin1String("rect(")) &&
        string.endsWith(QLatin1Char(')'))) {
        // remove "rect(" & ")"
        string.remove(0,5).chop(1);

#ifndef NWORKAROUND_ODF_BUGS
        KoOdfWorkaround::fixClipRectOffsetValuesString(string);
#endif
        // split into the 4 values
        const QStringList valueStrings = string.split(QLatin1Char(','));

        if (valueStrings.count() != 4) {
            kWarning() << "Not exactly 4 values for attribute fo:clip=rect(...):" << originalString << ", please report.";
            // hard to guess which value is for which offset, so just cancel parsing and return with the default rect
            return rect;
        }

        // default is 0.0 for all offsets
        qreal values[4] = { 0, 0, 0, 0 };
        const QLatin1String autoValueString("auto");

        for (int i=0; i<4; ++i) {
            const QString valueString = valueStrings.at(i).trimmed();
            // "auto" means: keep default 0.0
            if (valueString != autoValueString) {
                values[i] = KoUnit::parseValue(valueString, 0.0);
            }
        }

        rect.top = values[0];
        rect.right = values[1];
        rect.bottom = values[2];
        rect.left = values[3];
        rect.uniform = false;
        rect.inverted = true;
    }

    return rect;
}

QPainterPath PictureShape::shadowOutline() const
{
    // Always return an outline for a shadow even if no fill is defined.
    return outline();
}

void PictureShape::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &)
{
    QRectF viewRect = converter.documentToView(QRectF(QPointF(0,0), size()));

    if (imageData() == 0) {
        painter.fillRect(viewRect, QColor(Qt::gray));
        return;
    }

    QSize pixmapSize = calcOptimalPixmapSize(viewRect.size(), imageData()->image().size());

    // Normalize the clipping rect if it isn't already done.
    m_clippingRect.normalize(imageData()->imageSize());

    // Handle style:mirror, i.e. mirroring horizontally and/or vertically.
    // 
    // NOTE: At this time we don't handle HorizontalOnEven
    //       and HorizontalOnOdd, which have to know which
    //       page they are on.  In those cases we treat it as
    //       no horizontal mirroring at all.
    bool   doFlip = false;
    QSizeF shapeSize = size();
    QSizeF viewSize = converter.documentToView(shapeSize);
    qreal  midpointX = 0.0;
    qreal  midpointY = 0.0;
    qreal  scaleX = 1.0;
    qreal  scaleY = 1.0;
    if (m_mirrorMode & MirrorHorizontal) {
        midpointX = viewSize.width() / qreal(2.0);
        scaleX = -1.0;
        doFlip = true;
    }
    if (m_mirrorMode & MirrorVertical) {
        midpointY = viewSize.height() / qreal(2.0);
        scaleY = -1.0;
        doFlip = true;
    }
    if (doFlip) {
        QTransform outputTransform = painter.transform();
        QTransform worldTransform  = QTransform();

        //kDebug(31000) << "Flipping" << midpointX << midpointY << scaleX << scaleY;
        worldTransform.translate(midpointX, midpointY);
        worldTransform.scale(scaleX, scaleY);
        worldTransform.translate(-midpointX, -midpointY);
        //kDebug(31000) << "After flipping for window" << worldTransform;

        QTransform newTransform = worldTransform * outputTransform;
        painter.setWorldTransform(newTransform);
    }

    // Paint the image as prepared in waitUntilReady()
    if (!m_printQualityImage.isNull() && pixmapSize != m_printQualityRequestedSize) {
        QSizeF imageSize = m_printQualityImage.size();
        QRectF cropRect(
            imageSize.width()  * m_clippingRect.left,
            imageSize.height() * m_clippingRect.top,
            imageSize.width()  * m_clippingRect.width(),
            imageSize.height() * m_clippingRect.height()
        );

        painter.drawImage(viewRect, m_printQualityImage, cropRect);
        m_printQualityImage = QImage(); // free memory
    }
    else {
        QPixmap pixmap;
        QString key(generate_key(imageData()->key(), pixmapSize));

        // If the required pixmap is not in the cache
        // launch a task in a background thread that scales
        // the source image to the required size
        if (!QPixmapCache::find(key, &pixmap)) {
            QThreadPool::globalInstance()->start(new _Private::PixmapScaler(this, pixmapSize));
            painter.fillRect(viewRect, QColor(Qt::gray)); // just paint a gray rect as long as we don't have the required pixmap
        }
        else {
            QRectF cropRect(
                pixmapSize.width()  * m_clippingRect.left,
                pixmapSize.height() * m_clippingRect.top,
                pixmapSize.width()  * m_clippingRect.width(),
                pixmapSize.height() * m_clippingRect.height()
            );

            painter.drawPixmap(viewRect, pixmap, cropRect);
        }
    }
}

void PictureShape::waitUntilReady(const KoViewConverter &converter, bool asynchronous) const
{
    KoImageData *imageData = qobject_cast<KoImageData*>(userData());
    if (imageData == 0) {
        return;
    }

    if (asynchronous) {
        // get pixmap and schedule it if not
        QSize pixels = converter.documentToView(QRectF(QPointF(0,0), size())).size().toSize();
        QImage image = imageData->image();
        if (image.isNull()) {
            return;
        }
        m_printQualityRequestedSize = pixels;
        if (image.size().width() < pixels.width()) { // don't scale up.
            pixels = image.size();
        }
        m_printQualityImage = image.scaled(pixels, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    else {
        QSize pixmapSize = calcOptimalPixmapSize(converter.documentToView(QRectF(QPointF(0,0), size())).size(), imageData->image().size());
        QString key(generate_key(imageData->key(), pixmapSize));
        if (QPixmapCache::find(key) == 0) {
            QPixmap pixmap = imageData->pixmap(pixmapSize);
            QPixmapCache::insert(key, pixmap);
        }
    }
}

void PictureShape::saveOdf(KoShapeSavingContext &context) const
{
    // make sure we have a valid image data pointer before saving
    KoImageData *imageData = qobject_cast<KoImageData*>(userData());
    if (imageData == 0) {
        return;
    }

    KoXmlWriter &writer = context.xmlWriter();

    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    writer.startElement("draw:image");
    // In the spec, only the xlink:href attribute is marked as mandatory, cool :)
    QString name = context.imageHref(imageData);
    writer.addAttribute("xlink:type", "simple");
    writer.addAttribute("xlink:show", "embed");
    writer.addAttribute("xlink:actuate", "onLoad");
    writer.addAttribute("xlink:href", name);
    saveText(context);
    writer.endElement(); // draw:image
    QSizeF scaleFactor(imageData->imageSize().width() / size().width(),
                  imageData->imageSize().height() / size().height());
    saveOdfClipContour(context, scaleFactor);
    writer.endElement(); // draw:frame

    context.addDataCenter(m_imageCollection);
}

bool PictureShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfAllAttributes);

    if (loadOdfFrame(element, context)) {
        // load contour (clip)
        KoImageData *imageData = qobject_cast<KoImageData*>(userData());

        QSizeF scaleFactor(size().width() / imageData->imageSize().width(),
                 size().height() / imageData->imageSize().height());

        loadOdfClipContour(element, context, scaleFactor);

        return true;
    }
    return false;
}

bool PictureShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (m_imageCollection) {
        const QString href = element.attribute("href");
        // this can happen in case it is a presentation:placeholder
        if (!href.isEmpty()) {
            KoStore *store = context.odfLoadingContext().store();
            KoImageData *data = m_imageCollection->createImageData(href, store);
            setUserData(data);
        } else {
            // check if we have an office:binary data element containing the image data
            const KoXmlElement &binaryData(KoXml::namedItemNS(element, KoXmlNS::office, "binary-data"));
            if (!binaryData.isNull()) {
                QImage image;
                if (image.loadFromData(QByteArray::fromBase64(binaryData.text().toLatin1()))) {
                    KoImageData *data = m_imageCollection->createImageData(image);
                    setUserData(data);
                }
            }
        }
    }

    loadText(element, context);

    return true;
}

KoImageCollection *PictureShape::imageCollection() const
{
    return m_imageCollection;
}

QString PictureShape::saveStyle(KoGenStyle& style, KoShapeSavingContext& context) const
{
    if(transparency() > 0.0) {
        style.addProperty("draw:image-opacity", QString("%1%").arg((1.0 - transparency()) * 100.0));
    }

    // this attribute is need to work around a bug in LO 3.4 to make it recognice us as an
    // image and not just any shape. But we shouldn't produce illegal odf so: only for testing!
    // style.addAttribute("style:parent-style-name", "dummy");

    // Mirroring
    if (m_mirrorMode != MirrorNone) {
        QString mode;

        if (m_mirrorMode & MirrorHorizontal)
            mode = "horizontal";
        else if (m_mirrorMode & MirrorHorizontalOnEven)
            mode = "horizontal-on-even";
        else if (m_mirrorMode & MirrorHorizontalOnOdd)
            mode = "horizontal-on-odd";

        if (m_mirrorMode & MirrorVertical) {
            if (!mode.isEmpty())
                mode += ' ';
            mode += "vertical";
        }

        style.addProperty("style:mirror", mode);
    }

    switch(m_colorMode)
    {
    case Standard:
        style.addProperty("draw:color-mode", "standard");
        break;
    case Greyscale:
        style.addProperty("draw:color-mode", "greyscale");
        break;
    case Watermark:
        style.addProperty("draw:color-mode", "watermark");
        break;
    case Mono:
        style.addProperty("draw:color-mode", "mono");
        break;
    }

    KoImageData *imageData = qobject_cast<KoImageData*>(userData());

    if (imageData != 0) {
        QSizeF imageSize = imageData->imageSize();
        ClippingRect rect = m_clippingRect;

        rect.normalize(imageSize);
        rect.bottom = 1.0 - rect.bottom;
        rect.right = 1.0 - rect.right;

        if (!qFuzzyCompare(rect.left + rect.right + rect.top + rect.bottom, qreal(0))) {
            style.addProperty("fo:clip", QString("rect(%1pt, %2pt, %3pt, %4pt)")
                .arg(rect.top * imageSize.height())
                .arg(rect.right * imageSize.width())
                .arg(rect.bottom * imageSize.height())
                .arg(rect.left * imageSize.width())
            );
        }
    }

    return KoTosContainer::saveStyle(style, context);
}

void PictureShape::loadStyle(const KoXmlElement& element, KoShapeLoadingContext& context)
{
    // Load the common parts of the style.
    KoTosContainer::loadStyle(element, context);

    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    // Mirroring
    if (styleStack.hasProperty(KoXmlNS::style, "mirror")) {
        QString mirrorMode = styleStack.property(KoXmlNS::style, "mirror");

        QFlags<PictureShape::MirrorMode>  mode = 0;

        // Only one of the horizontal modes
        if (mirrorMode.contains("horizontal-on-even")) {
            mode |= MirrorHorizontalOnEven;
        }
        else if (mirrorMode.contains("horizontal-on-odd")) {
            mode |= MirrorHorizontalOnOdd;
        }
        else if (mirrorMode.contains("horizontal")) {
            mode |= MirrorHorizontal;
        }

        if (mirrorMode.contains("vertical")) {
            mode |= MirrorVertical;
        }

        m_mirrorMode = mode;
    }

    // Color-mode (effects)
    if (styleStack.hasProperty(KoXmlNS::draw, "color-mode")) {
        QString colorMode = styleStack.property(KoXmlNS::draw, "color-mode");
        if (colorMode == "greyscale") {
            setColorMode(Greyscale);
        }
        else if (colorMode == "mono") {
            setColorMode(Mono);
        }
        else if (colorMode == "watermark") {
            setColorMode(Watermark);
        }
    }

    // image opacity
    QString opacity(styleStack.property(KoXmlNS::draw, "image-opacity"));
    if (! opacity.isEmpty() && opacity.right(1) == "%") {
        setTransparency(1.0 - (opacity.left(opacity.length() - 1).toFloat() / 100.0));
    }

    // clip rect
    m_clippingRect = parseClippingRectString(styleStack.property(KoXmlNS::fo, "clip"));
}

QFlags<PictureShape::MirrorMode> PictureShape::mirrorMode() const
{
    return m_mirrorMode;
}

PictureShape::ColorMode PictureShape::colorMode() const
{
    return m_colorMode;
}

void PictureShape::setMirrorMode(QFlags<PictureShape::MirrorMode> mode)
{
    // Sanity check
    mode &= MirrorMask;

    // Make sure only one bit of the horizontal modes is set.
    if (mode & MirrorHorizontal)
        mode &= ~(MirrorHorizontalOnEven | MirrorHorizontalOnOdd);
    else if (mode & MirrorHorizontalOnEven)
        mode &= ~MirrorHorizontalOnOdd;

    // If the mode changes, redraw the image.
    if (mode != m_mirrorMode) {
        m_mirrorMode = mode;
        update();
    }
}

void PictureShape::setColorMode(PictureShape::ColorMode mode)
{
    if (mode != m_colorMode) {
        filterEffectStack()->removeFilterEffect(0);

        switch(mode)
        {
        case Greyscale:
            filterEffectStack()->appendFilterEffect(new GreyscaleFilterEffect());
            break;
        case Mono:
            filterEffectStack()->appendFilterEffect(new MonoFilterEffect());
            break;
        case Watermark:
            filterEffectStack()->appendFilterEffect(new WatermarkFilterEffect());
            break;
        default:
            break;
        }

        m_colorMode = mode;
        update();
    }
}

KoClipPath *PictureShape::generateClipPath()
{
    QPainterPath path = _Private::generateOutline(imageData()->image());
    path = path * QTransform().scale(size().width(), size().height());

    KoPathShape *pathShape = KoPathShape::createShapeFromPainterPath(path);

    //createShapeFromPainterPath converts the path topleft into a shape topleft
    //and the pathShape needs to be on top of us. So to preserve both we do:
    pathShape->setTransformation(pathShape->transformation() * transformation());

    return new KoClipPath(this, new KoClipData(pathShape));
}

bool PictureShape::saveSvg(SvgSavingContext &context)
{
    KoImageData *imageData = qobject_cast<KoImageData*>(userData());
    if (!imageData) {
        qWarning() << "Picture has no image data. Omitting.";
        return false;
    }

    context.shapeWriter().startElement("image");
    context.shapeWriter().addAttribute("id", context.getID(this));

    QTransform m = transformation();
    if (m.type() == QTransform::TxTranslate) {
        const QPointF pos = position();
        context.shapeWriter().addAttributePt("x", pos.x());
        context.shapeWriter().addAttributePt("y", pos.y());
    } else {
        context.shapeWriter().addAttribute("transform", SvgUtil::transformToString(m));
    }

    const QSizeF s = size();
    context.shapeWriter().addAttributePt("width", s.width());
    context.shapeWriter().addAttributePt("height", s.height());
    context.shapeWriter().addAttribute("xlink:href", context.saveImage(imageData));
    context.shapeWriter().endElement();

    return true;
}

bool PictureShape::loadSvg(const KoXmlElement &element, SvgLoadingContext &context)
{
    const qreal x = SvgUtil::parseUnitX(context.currentGC(), element.attribute("x", "0"));
    const qreal y = SvgUtil::parseUnitY(context.currentGC(), element.attribute("y", "0"));
    const qreal w = SvgUtil::parseUnitX(context.currentGC(), element.attribute("width", "0"));
    const qreal h = SvgUtil::parseUnitY(context.currentGC(), element.attribute("height", "0"));

    // zero width of height disables rendering this image (see svg spec)
    if (w == 0.0 || h == 0.0)
        return 0;

    const QString href = element.attribute("xlink:href");

    QImage image;

    if (href.startsWith(QLatin1String("data:"))) {
        int start = href.indexOf("base64,");
        if (start <= 0)
            return false;
        if(!image.loadFromData(QByteArray::fromBase64(href.mid(start + 7).toLatin1())))
            return false;
    } else if (!image.load(context.absoluteFilePath(href))) {
        return false;
    }

    KoImageCollection *imageCollection = context.imageCollection();
    if (!imageCollection)
        return false;

    // TODO use it already for loading
    KoImageData *data = imageCollection->createImageData(image);

    setUserData(data);
    setSize(QSizeF(w, h));
    setPosition(QPointF(x, y));
    return true;
}
