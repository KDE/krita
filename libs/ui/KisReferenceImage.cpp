/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisReferenceImage.h"

#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QApplication>
#include <QClipboard>
#include <QSharedData>
#include <QFileInfo>
#include <QImageReader>
#include <QUrl>
#include <QPainterPath>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QColorSpace>
#endif

#include <kundo2command.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoTosContainer_p.h>
#include <krita_utils.h>
#include <kis_coordinates_converter.h>
#include <kis_dom_utils.h>
#include <SvgUtil.h>
#include <libs/flake/svg/parsers/SvgTransformParser.h>
#include <libs/brush/kis_qimage_pyramid.h>
#include <utils/KisClipboardUtil.h>

#include <KisDocument.h>
#include <KisPart.h>
#include <KoClipPath.h>
#include <kis_command_ids.h>
#include <kis_canvas2.h>

struct KisReferenceImage::Private : public QSharedData
{
    // Filename within .kra (for embedding)
    QString internalFilename;

    // File on disk (for linking)
    QString externalFilename;

    QImage image;
    QImage cachedImage;
    KisQImagePyramid mipmap;

    QRectF cropRect;
    bool crop{false};

    qreal saturation{1.0};
    int id{-1};
    bool embed{true};

    bool loadFromFile() {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!externalFilename.isEmpty(), false);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(QFileInfo(externalFilename).exists(), false);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(QFileInfo(externalFilename).isReadable(), false);
        {
            QImageReader reader(externalFilename);
            reader.setDecideFormatFromContent(true);
            image = reader.read();

            if (image.isNull()) {
                reader.setAutoDetectImageFormat(true);
                image = reader.read();
            }

        }

        if (image.isNull()) {
            image.load(externalFilename);
        }

        if (image.isNull()) {
            KisDocument * doc = KisPart::instance()->createTemporaryDocument();
            if (doc->openPath(externalFilename, KisDocument::DontAddToRecent)) {
                image = doc->image()->convertToQImage(doc->image()->bounds(), 0);
            }
            KisPart::instance()->removeDocument(doc);
        }

        // See https://bugs.kde.org/show_bug.cgi?id=416515 -- a jpeg image
        // loaded into a qimage cannot be saved to png unless we explicitly
        // convert the colorspace of the QImage
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        image.convertToColorSpace(QColorSpace(QColorSpace::SRgb));
#endif

        return (!image.isNull());
    }

    bool loadFromClipboard() {
        image = KisClipboardUtil::getImageFromClipboard();
        return !image.isNull();
    }

    void updateCache() {
        if (saturation < 1.0) {
            cachedImage = KritaUtils::convertQImageToGrayA(image);

            if (saturation > 0.0) {
                QPainter gc2(&cachedImage);
                gc2.setOpacity(saturation);
                gc2.drawImage(QPoint(), image);
            }
        } else {
            cachedImage = image;
        }

        mipmap = KisQImagePyramid(cachedImage, false);
    }
};


KisReferenceImage::SetSaturationCommand::SetSaturationCommand(const QList<KoShape *> &shapes, qreal newSaturation, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set saturation"), parent)
    , newSaturation(newSaturation)
{
    images.reserve(shapes.count());

    Q_FOREACH(auto *shape, shapes) {
        auto *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_BREAK(reference);
        images.append(reference);
    }

    Q_FOREACH(auto *image, images) {
        oldSaturations.append(image->saturation());
    }
}

void KisReferenceImage::SetSaturationCommand::undo()
{
    auto saturationIterator = oldSaturations.begin();
    Q_FOREACH(auto *image, images) {
        image->setSaturation(*saturationIterator);
        image->update();
        saturationIterator++;
    }
}

void KisReferenceImage::SetSaturationCommand::redo()
{
    Q_FOREACH(auto *image, images) {
        image->setSaturation(newSaturation);
        image->update();
    }
}

KisReferenceImage::CropReferenceImage::CropReferenceImage(KoShape *shape, QRectF rect, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Crop Reference Image"), parent)
    , newRect(rect.toRect())
{
    referenceImage  = dynamic_cast<KisReferenceImage*>(shape);
    KIS_SAFE_ASSERT_RECOVER_BREAK(referenceImage);

    oldImage = referenceImage->image();
    oldPos = shape->absolutePosition(KoFlake::TopLeft);
    oldShapeSize = shape->size();

    QTransform transform = QTransform::fromScale(referenceImage->image().width() / referenceImage->size().width(),
                                                 referenceImage->image().height() / referenceImage->size().height());
    imageRect = transform.mapRect(rect.toRect());
}

void KisReferenceImage::CropReferenceImage::undo()
{
    referenceImage->setSize(oldShapeSize);
    referenceImage->setImage(oldImage);
    referenceImage->setAbsolutePosition(oldPos, KoFlake::TopLeft);
    referenceImage->updateAbsolute(QRectF(QPointF(), oldShapeSize));
}

void KisReferenceImage::CropReferenceImage::redo()
{
    QImage newImage = referenceImage->image().copy(imageRect);
    referenceImage->setSize(newRect.size());
    referenceImage->setAbsolutePosition(oldPos + newRect.topLeft(), KoFlake::TopLeft);
    referenceImage->setImage(newImage);
    referenceImage->updateAbsolute(QRectF(QPointF(), oldShapeSize));
}

int KisReferenceImage::CropReferenceImage::id() const
{
    return KisCommandUtils::CropReferenceImageId;
}

bool KisReferenceImage::CropReferenceImage::mergeWith(const KUndo2Command *command)
{
    const CropReferenceImage *other = dynamic_cast<const CropReferenceImage*>(command);

    if (other->referenceImage != referenceImage) {
        return false;
    }

    newRect = other->newRect;
    imageRect = other->imageRect;
    return true;
}

KisReferenceImage::KisReferenceImage()
    : d(new Private())
    , m_pinRotate(false)
    , m_pinMirror(false)
    , m_pinPosition(false)
    , m_pinZoom(false)
    , m_mirrorX(false)
    , m_mirrorY(false)
    , m_initialValues(false)
    , m_previousAngle(0)
    , m_transform(QTransform())
{
    setKeepAspectRatio(true);
    d->cropRect.setSize(size());
    setAbsolute(false);
}

KisReferenceImage::KisReferenceImage(const KisReferenceImage &rhs)
    : KoTosContainer(rhs)
    , m_pinRotate(false)
    , m_pinMirror(false)
    , m_pinPosition(false)
    , m_pinZoom(false)
    , m_mirrorX(false)
    , m_mirrorY(false)
    , m_initialValues(false)
    , m_previousAngle(0)
    , m_transform(QTransform())
    , d(rhs.d)
{}

KisReferenceImage::~KisReferenceImage()
{}

KisReferenceImage * KisReferenceImage::fromFile(const QString &filename, const KisCoordinatesConverter &converter, QWidget *parent)
{
    KisReferenceImage *reference = new KisReferenceImage();
    reference->d->externalFilename = filename;
    bool ok = reference->d->loadFromFile();

    if (ok) {
        QRect r = QRect(QPoint(), reference->d->image.size());
        QSizeF shapeSize = converter.imageToDocument(r).size();
        reference->setSize(shapeSize);
    } else {
        delete reference;

        if (parent) {
            QMessageBox::critical(parent, i18nc("@title:window", "Krita"), i18n("Could not load %1.", filename));
        }

        return nullptr;
    }

    return reference;
}

KisReferenceImage *KisReferenceImage::fromClipboard(const KisCoordinatesConverter &converter)
{
    KisReferenceImage *reference = new KisReferenceImage();
    bool ok = reference->d->loadFromClipboard();

    if (ok) {
        QRect r = QRect(QPoint(), reference->d->image.size());
        QSizeF size = converter.imageToDocument(r).size();
        reference->setSize(size);
    } else {
        delete reference;
        reference = nullptr;
    }

    return reference;
}

void KisReferenceImage::paint(QPainter &gc, KoShapePaintingContext &/*paintcontext*/) const
{
    if (!parent()) return;

    gc.save();

    QSizeF shapeSize = size();
    // scale and rotation done by the user (excluding zoom)
    QTransform transform = QTransform::fromScale(shapeSize.width() / d->image.width(), shapeSize.height() / d->image.height());

    if (d->cachedImage.isNull()) {
        // detach the data
        const_cast<KisReferenceImage*>(this)->d->updateCache();
    }

    qreal scale;
    // scale from the highDPI display
    QTransform devicePixelRatioFTransform = QTransform::fromScale(gc.device()->devicePixelRatioF(), gc.device()->devicePixelRatioF());
    // all three transformations: scale and rotation done by the user, scale from highDPI display, and zoom + rotation of the view
    // order: zoom/rotation of the view; scale to high res; scale and rotation done by the user
    QImage prescaled = d->mipmap.getClosestWithoutWorkaroundBorder(transform * devicePixelRatioFTransform * gc.transform(), &scale);
    transform.scale(1.0 / scale, 1.0 / scale);

    if (scale > 1.0) {
        // enlarging should be done without smooth transformation
        // so the user can see pixels just as they are painted
        gc.setRenderHints(QPainter::Antialiasing);
    } else {
        gc.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    }

    //gc.setClipRect(d->cropRect, Qt::IntersectClip); Fix this with scaling
    gc.setClipRect(QRectF(QPointF(), shapeSize), Qt::IntersectClip);
    gc.setTransform(transform, true);
    gc.drawImage(QPoint(), prescaled);

    gc.restore();
}

void KisReferenceImage::setSaturation(qreal saturation)
{
    d->saturation = saturation;
    d->cachedImage = QImage();
}

qreal KisReferenceImage::saturation() const
{
    return d->saturation;
}

void KisReferenceImage::setEmbed(bool embed)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(embed || !d->externalFilename.isEmpty());
    d->embed = embed;
}

bool KisReferenceImage::embed()
{
    return d->embed;
}

bool KisReferenceImage::hasLocalFile()
{
    return !d->externalFilename.isEmpty();
}

QString KisReferenceImage::filename() const
{
    return d->externalFilename;
}

QString KisReferenceImage::internalFile() const
{
    return d->internalFilename;
}


void KisReferenceImage::setFilename(const QString &filename)
{
    d->externalFilename = filename;
    d->embed = false;
}

QColor KisReferenceImage::getPixel(QPointF position)
{
    if (transparency() == 1.0) return Qt::transparent;

    const QSizeF shapeSize = size();
    const QTransform scale = QTransform::fromScale(d->image.width() / shapeSize.width(), d->image.height() / shapeSize.height());

    const QTransform transform = absoluteTransformation().inverted() * scale;
    const QPointF localPosition = position * transform;

    if (d->cachedImage.isNull()) {
        d->updateCache();
    }

    return d->cachedImage.pixelColor(localPosition.toPoint());
}

void KisReferenceImage::saveXml(QDomDocument &document, QDomElement &parentElement, int id)
{
    d->id = id;

    QDomElement element = document.createElement("referenceimage");

    if (d->embed) {
        d->internalFilename = QString("reference_images/%1.png").arg(id);
    }
    
    const QString src = d->embed ? d->internalFilename : (QString("file://") + d->externalFilename);
    element.setAttribute("src", src);

    const QSizeF &shapeSize = size();
    element.setAttribute("width", KisDomUtils::toString(shapeSize.width()));
    element.setAttribute("height", KisDomUtils::toString(shapeSize.height()));
    element.setAttribute("keepAspectRatio", keepAspectRatio() ? "true" : "false");
    element.setAttribute("transform", SvgUtil::transformToString(transform()));

    element.setAttribute("opacity", KisDomUtils::toString(1.0 - transparency()));
    element.setAttribute("saturation", KisDomUtils::toString(d->saturation));

    parentElement.appendChild(element);
}

KisReferenceImage * KisReferenceImage::fromXml(const QDomElement &elem)
{
    auto *reference = new KisReferenceImage();

    const QString &src = elem.attribute("src");

    if (src.startsWith("file://")) {
        reference->d->externalFilename = src.mid(7);
        reference->d->embed = false;
    } else {
        reference->d->internalFilename = src;
        reference->d->embed = true;
    }

    qreal width = KisDomUtils::toDouble(elem.attribute("width", "100"));
    qreal height = KisDomUtils::toDouble(elem.attribute("height", "100"));
    reference->setSize(QSizeF(width, height));
    reference->setKeepAspectRatio(elem.attribute("keepAspectRatio", "true").toLower() == "true");

    auto transform = SvgTransformParser(elem.attribute("transform")).transform();
    reference->setTransformation(transform);

    qreal opacity = KisDomUtils::toDouble(elem.attribute("opacity", "1"));
    reference->setTransparency(1.0 - opacity);

    qreal saturation = KisDomUtils::toDouble(elem.attribute("saturation", "1"));
    reference->setSaturation(saturation);

    return reference;
}

bool KisReferenceImage::saveImage(KoStore *store) const
{
    if (!d->embed) return true;

    if (!store->open(d->internalFilename)) {
        return false;
    }

    bool saved = false;

    KoStoreDevice storeDev(store);
    if (storeDev.open(QIODevice::WriteOnly)) {
        saved = d->image.save(&storeDev, "PNG");
    }

    return store->close() && saved;
}

bool KisReferenceImage::loadImage(KoStore *store)
{
    if (!d->embed) {
        return d->loadFromFile();
    }

    if (!store->open(d->internalFilename)) {
        return false;
    }

    KoStoreDevice storeDev(store);
    if (!storeDev.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (!d->image.load(&storeDev, "PNG")) {
        return false;
    }

    return store->close();
}

QImage KisReferenceImage::image()
{
    return d->image;
}

void KisReferenceImage::setImage(QImage image)
{
    d->image = image;
    d->cachedImage = QImage();
}


void KisReferenceImage::reloadImage()
{
    d->loadFromFile();
    d->cachedImage = QImage();
}

KoShape *KisReferenceImage::cloneShape() const
{
    return new KisReferenceImage(*this);
}

bool KisReferenceImage::cropEnabled()
{
    return d->crop;
}

void KisReferenceImage::setCrop(bool v)
{
    d->crop = v;
    if(v) {
       // Handle the Lock Button here
       d->cropRect = outlineRect();
    }
}

QRectF KisReferenceImage::cropRect()
{
    return d->cropRect;
}

void KisReferenceImage::setCropRect(QRectF rect)
{
   d->cropRect = rect;
   update();
}


bool KisReferenceImage::pinRotate()
{
    return m_pinRotate;
}

void KisReferenceImage::setPinRotate(bool value)
{
    m_pinRotate = value;
}

bool KisReferenceImage::pinMirror()
{
    return m_pinMirror;
}

void KisReferenceImage::setPinMirror(bool value)
{
    m_pinMirror = value;
}

bool KisReferenceImage::pinPosition()
{
    return m_pinPosition;
}

void KisReferenceImage::setPinPosition(bool value)
{
    m_pinPosition = value;
}

bool KisReferenceImage::pinZoom()
{
    return m_pinZoom;
}

void KisReferenceImage::setPinZoom(bool value)
{
    m_pinZoom = value;
}

void KisReferenceImage::addCanvasTransformation(KisCanvas2 *kisCanvas)
{
    if(!m_initialValues) {
        m_docOffset = kisCanvas->documentOffset();
        m_zoom = kisCanvas->viewConverter()->zoom();
        m_transform *= QTransform::fromScale(m_zoom,m_zoom);
        m_initialValues = true;
    }

    if(!qFuzzyCompare(kisCanvas->viewConverter()->zoom(), m_zoom)) {
        if(!m_pinZoom) {
            qreal diff = kisCanvas->viewConverter()->zoom()- m_zoom;
            m_transform *= QTransform::fromScale((m_zoom + diff)/m_zoom,(m_zoom + diff)/m_zoom);
            m_zoom = kisCanvas->viewConverter()->zoom();

        }
        m_docOffset = kisCanvas->documentOffset();
    }

    if(m_mirrorX != kisCanvas->coordinatesConverter()->xAxisMirrored()
                || m_mirrorY != kisCanvas->coordinatesConverter()->yAxisMirrored()) {
        if(!m_pinMirror) {
            qreal scaleX = m_mirrorX != kisCanvas->coordinatesConverter()->xAxisMirrored() ? -1 : 1;
            qreal scaleY = m_mirrorY != kisCanvas->coordinatesConverter()->yAxisMirrored() ? -1 : 1;
            QPointF center = kisCanvas->coordinatesConverter()->widgetCenterPoint();
            m_transform *= QTransform::fromTranslate(-center.x(),-center.y()) *
                    QTransform::fromScale(scaleX, scaleY) * QTransform::fromTranslate(center.x(),center.y());
        }
        m_docOffset = kisCanvas->documentOffset();
        m_previousAngle = kisCanvas->rotationAngle();
    }

    if(m_previousAngle != kisCanvas->rotationAngle())
    {
        if(!m_pinRotate) {
            QPointF center = kisCanvas->coordinatesConverter()->widgetCenterPoint();
            qreal diff = kisCanvas->rotationAngle() - m_previousAngle;
            QTransform rot;
            rot.rotate(diff);
            m_transform *= QTransform::fromTranslate(-center.x(),-center.y()) * rot
                    * QTransform::fromTranslate(center.x(),center.y());
        }
        m_docOffset = kisCanvas->documentOffset();
    }

    if(kisCanvas->documentOffset() != m_docOffset && !m_pinPosition) {
      QPointF diff = kisCanvas->documentOffset() - m_docOffset;
        m_transform *= QTransform::fromTranslate(-diff.x(), -diff.y());
    }

    m_zoom = kisCanvas->viewConverter()->zoom();
    m_docOffset = kisCanvas->documentOffset();
    m_mirrorX = kisCanvas->coordinatesConverter()->xAxisMirrored();
    m_mirrorY = kisCanvas->coordinatesConverter()->yAxisMirrored();
    m_previousAngle = kisCanvas->rotationAngle();

    if(!qFuzzyCompare(m_transform, extraTransform())) {
        setExtraTransform(m_transform);
    }
}
