/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pixel_selection.h"


#include <QImage>
#include <QVector>

#include <QMutex>
#include <QPoint>
#include <QPolygon>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoIntegerMaths.h>
#include <KoCompositeOpRegistry.h>

#include "kis_layer.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_fill_painter.h"
#include "kis_outline_generator.h"
#include <kis_iterator_ng.h>
#include "kis_lod_transform.h"
#include "kundo2command.h"


struct Q_DECL_HIDDEN KisPixelSelection::Private {
    KisSelectionWSP parentSelection;

    QPainterPath outlineCache;
    bool outlineCacheValid;
    QMutex outlineCacheMutex;

    bool thumbnailImageValid;
    QImage thumbnailImage;
    QTransform thumbnailImageTransform;

    QPoint lod0CachesOffset;

    void invalidateThumbnailImage() {
        thumbnailImageValid = false;
        thumbnailImage = QImage();
        thumbnailImageTransform = QTransform();
    }
};

KisPixelSelection::KisPixelSelection(KisDefaultBoundsBaseSP defaultBounds, KisSelectionWSP parentSelection)
        : KisPaintDevice(0, KoColorSpaceRegistry::instance()->alpha8(), defaultBounds)
        , m_d(new Private)
{
    m_d->outlineCacheValid = true;
    m_d->invalidateThumbnailImage();

    m_d->parentSelection = parentSelection;
}

KisPixelSelection::KisPixelSelection(const KisPixelSelection& rhs, KritaUtils::DeviceCopyMode copyMode)
        : KisPaintDevice(rhs, copyMode)
        , KisSelectionComponent(rhs)
        , m_d(new Private)
{
    // parent selection is not supposed to be shared
    m_d->outlineCache = rhs.m_d->outlineCache;
    m_d->outlineCacheValid = rhs.m_d->outlineCacheValid;

    m_d->thumbnailImageValid = rhs.m_d->thumbnailImageValid;
    m_d->thumbnailImage = rhs.m_d->thumbnailImage;
    m_d->thumbnailImageTransform = rhs.m_d->thumbnailImageTransform;
}

KisPixelSelection::KisPixelSelection(const KisPaintDeviceSP copySource, KritaUtils::DeviceCopyMode copyMode, KisSelectionWSP parentSelection)
    : KisPaintDevice(0, KoColorSpaceRegistry::instance()->alpha8(), copySource->defaultBounds())
    , m_d(new Private)
{
    KisPaintDeviceSP tmpDevice = new KisPaintDevice(*copySource, copyMode, 0);
    tmpDevice->convertTo(this->colorSpace());

    this->makeFullCopyFrom(*tmpDevice, copyMode, 0);

    m_d->parentSelection = parentSelection;
    m_d->outlineCacheValid = false;
    m_d->invalidateThumbnailImage();
}

KisSelectionComponent* KisPixelSelection::clone(KisSelection*)
{
    return new KisPixelSelection(*this);
}

KisPixelSelection::~KisPixelSelection()
{
    delete m_d;
}

const KoColorSpace *KisPixelSelection::compositionSourceColorSpace() const
{
    return KoColorSpaceRegistry::instance()->
        colorSpace(GrayAColorModelID.id(),
                   Integer8BitsColorDepthID.id(),
                   QString());
}

bool KisPixelSelection::read(QIODevice *stream)
{
    bool retval = KisPaintDevice::read(stream);
    m_d->outlineCacheValid = false;
    m_d->invalidateThumbnailImage();
    return retval;
}

void KisPixelSelection::select(const QRect & rc, quint8 selectedness)
{
    QRect r = rc.normalized();
    if (r.isEmpty()) return;

    KisFillPainter painter(KisPaintDeviceSP(this));
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    painter.fillRect(r, KoColor(Qt::white, cs), selectedness);

    if (m_d->outlineCacheValid) {
        QPainterPath path;
        path.addRect(r);

        if (selectedness != MIN_SELECTED) {
            m_d->outlineCache += path;
        } else {
            m_d->outlineCache -= path;
        }
    }
    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::applySelection(KisPixelSelectionSP selection, SelectionAction action)
{
    switch (action) {
    case SELECTION_REPLACE:
        clear();
        addSelection(selection);
        break;
    case SELECTION_ADD:
        addSelection(selection);
        break;
    case SELECTION_SUBTRACT:
        subtractSelection(selection);
        break;
    case SELECTION_INTERSECT:
        intersectSelection(selection);
        break;
    case SELECTION_SYMMETRICDIFFERENCE:
        symmetricdifferenceSelection(selection);
        break;
    default:
        break;
    }
}

void KisPixelSelection::copyAlphaFrom(KisPaintDeviceSP src, const QRect &processRect)
{
    const KoColorSpace *srcCS = src->colorSpace();

    KisSequentialConstIterator srcIt(src, processRect);
    KisSequentialIterator dstIt(this, processRect);

    while (srcIt.nextPixel() && dstIt.nextPixel()) {
        const quint8 *srcPtr = srcIt.rawDataConst();
        quint8 *alpha8Ptr = dstIt.rawData();

        *alpha8Ptr = srcCS->opacityU8(srcPtr);
    }

    m_d->outlineCacheValid = false;
    m_d->outlineCache = QPainterPath();
    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::addSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect();
    if (r.isEmpty()) return;

    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        do {
            if (*src->oldRawData() + *dst->rawData() < MAX_SELECTED)
                *dst->rawData() = *src->oldRawData() + *dst->rawData();
            else
                *dst->rawData() = MAX_SELECTED;

        } while (src->nextPixel() && dst->nextPixel());
        dst->nextRow();
        src->nextRow();
    }

    m_d->outlineCacheValid &= selection->outlineCacheValid();

    if (m_d->outlineCacheValid) {
        m_d->outlineCache += selection->outlineCache();
    }

    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::subtractSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect();
    if (r.isEmpty()) return;


    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        do {
            if (*dst->rawData() - *src->oldRawData() > MIN_SELECTED)
                *dst->rawData() = *dst->rawData() - *src->oldRawData();
            else
                *dst->rawData() = MIN_SELECTED;

        } while (src->nextPixel() && dst->nextPixel());
        dst->nextRow();
        src->nextRow();
    }

    m_d->outlineCacheValid &= selection->outlineCacheValid();

    if (m_d->outlineCacheValid) {
        m_d->outlineCache -= selection->outlineCache();
    }

    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::intersectSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect().united(selectedRect());
    if (r.isEmpty()) return;

    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        do {
            *dst->rawData() = qMin(*dst->rawData(), *src->oldRawData());
        }  while (src->nextPixel() && dst->nextPixel());
        dst->nextRow();
        src->nextRow();
    }

    m_d->outlineCacheValid &= selection->outlineCacheValid();

    if (m_d->outlineCacheValid) {
        m_d->outlineCache &= selection->outlineCache();
    }

    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::symmetricdifferenceSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect().united(selectedRect());
    if (r.isEmpty()) return;

    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {

        do {
            *dst->rawData() = abs(*dst->rawData() - *src->oldRawData());
        }  while (src->nextPixel() && dst->nextPixel());

        dst->nextRow();
        src->nextRow();
    }
    
    m_d->outlineCacheValid &= selection->outlineCacheValid();

    if (m_d->outlineCacheValid) {
       m_d->outlineCache = (m_d->outlineCache | selection->outlineCache()) - (m_d->outlineCache & selection->outlineCache());
    }

    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::clear(const QRect & r)
{
    if (*defaultPixel().data() != MIN_SELECTED) {
        KisFillPainter painter(KisPaintDeviceSP(this));
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        painter.fillRect(r, KoColor(Qt::white, cs), MIN_SELECTED);
    } else {
        KisPaintDevice::clear(r);
    }

    if (m_d->outlineCacheValid) {
        QPainterPath path;
        path.addRect(r);

        m_d->outlineCache -= path;
    }

    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::clear()
{
    setDefaultPixel(KoColor(Qt::transparent, colorSpace()));
    KisPaintDevice::clear();

    m_d->outlineCacheValid = true;
    m_d->outlineCache = QPainterPath();

    // Empty the thumbnail image. It is a valid state.
    m_d->invalidateThumbnailImage();
    m_d->thumbnailImageValid = true;
}

void KisPixelSelection::invert()
{
    // Region is needed here (not exactBounds or extent), because
    // unselected but existing pixels need to be inverted too
    QRect rc = region().boundingRect();

    if (!rc.isEmpty()) {
        KisSequentialIterator it(this, rc);
        while(it.nextPixel()) {
            *(it.rawData()) = MAX_SELECTED - *(it.rawData());
        }
    }
    quint8 defPixel = MAX_SELECTED - *defaultPixel().data();
    setDefaultPixel(KoColor(&defPixel, colorSpace()));

    if (m_d->outlineCacheValid) {
        QPainterPath path;
        path.addRect(defaultBounds()->bounds());

        m_d->outlineCache = path - m_d->outlineCache;
    }

    m_d->invalidateThumbnailImage();
}

void KisPixelSelection::moveTo(const QPoint &pt)
{
    const int lod = defaultBounds()->currentLevelOfDetail();
    const QPoint lod0Point = !lod ? pt :
        pt * KisLodTransform::lodToInvScale(lod);

    const QPoint offset = lod0Point - m_d->lod0CachesOffset;

    if (m_d->outlineCacheValid) {
        m_d->outlineCache.translate(offset);
    }

    if (m_d->thumbnailImageValid) {
        m_d->thumbnailImageTransform =
            QTransform::fromTranslate(offset.x(), offset.y()) *
            m_d->thumbnailImageTransform;
    }

    m_d->lod0CachesOffset = lod0Point;

    KisPaintDevice::moveTo(pt);
}

bool KisPixelSelection::isTotallyUnselected(const QRect & r) const
{
    if (*defaultPixel().data() != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

QRect KisPixelSelection::selectedRect() const
{
    return extent();
}

QRect KisPixelSelection::selectedExactRect() const
{
    return exactBounds();
}

QVector<QPolygon> KisPixelSelection::outline() const
{
    QRect selectionExtent = selectedExactRect();

    /**
     * When the default pixel is not fully transparent, the
     * exactBounds() return extent of the device instead. To make this
     * value sane we should limit the calculated area by the bounds of
     * the image.
     */
    if (*defaultPixel().data() != MIN_SELECTED) {
        selectionExtent &= defaultBounds()->bounds();
    }

    qint32 xOffset = selectionExtent.x();
    qint32 yOffset = selectionExtent.y();
    qint32 width = selectionExtent.width();
    qint32 height = selectionExtent.height();

    KisOutlineGenerator generator(colorSpace(), MIN_SELECTED);
    // If the selection is small using a buffer is much faster
    try {
        quint8* buffer = new quint8[width*height];
        readBytes(buffer, xOffset, yOffset, width, height);

        QVector<QPolygon> paths = generator.outline(buffer, xOffset, yOffset, width, height);

        delete[] buffer;
        return paths;
    }
    catch(const std::bad_alloc&) {
        // Allocating so much memory failed, so we fall through to the slow option.
        warnKrita << "KisPixelSelection::outline ran out of memory allocating" << width << "*" << height << "bytes.";
    }

    return generator.outline(this, xOffset, yOffset, width, height);
}

bool KisPixelSelection::isEmpty() const
{
    return *defaultPixel().data() == MIN_SELECTED && selectedRect().isEmpty();
}

QPainterPath KisPixelSelection::outlineCache() const
{
    QMutexLocker locker(&m_d->outlineCacheMutex);
    return m_d->outlineCache;
}

void KisPixelSelection::setOutlineCache(const QPainterPath &cache)
{
    QMutexLocker locker(&m_d->outlineCacheMutex);
    m_d->outlineCache = cache;
    m_d->outlineCacheValid = true;
    m_d->thumbnailImageValid = false;
}

bool KisPixelSelection::outlineCacheValid() const
{
    QMutexLocker locker(&m_d->outlineCacheMutex);
    return m_d->outlineCacheValid;
}

void KisPixelSelection::invalidateOutlineCache()
{
    QMutexLocker locker(&m_d->outlineCacheMutex);
    m_d->outlineCacheValid = false;
    m_d->thumbnailImageValid = false;
}

void KisPixelSelection::recalculateOutlineCache()
{
    QMutexLocker locker(&m_d->outlineCacheMutex);

    m_d->outlineCache = QPainterPath();

    Q_FOREACH (const QPolygon &polygon, outline()) {
        m_d->outlineCache.addPolygon(polygon);

        /**
         * The outline generation algorithm has a small bug, which
         * results in the starting point be repeated twice in the
         * beginning of the path, instead of being put to the
         * end. Here we just explicitly close the path to workaround
         * it.
         *
         * \see KisSelectionTest::testOutlineGeneration()
         */
        m_d->outlineCache.closeSubpath();
    }

    m_d->outlineCacheValid = true;
}

bool KisPixelSelection::thumbnailImageValid() const
{
    return m_d->thumbnailImageValid;
}

QImage KisPixelSelection::thumbnailImage() const
{
    return m_d->thumbnailImage;
}

QTransform KisPixelSelection::thumbnailImageTransform() const
{
    return m_d->thumbnailImageTransform;
}

QImage deviceToQImage(KisPaintDeviceSP device,
                      const QRect &rc,
                      const QColor &maskColor)
{
    QImage image(rc.size(), QImage::Format_ARGB32);

    QColor color = maskColor;
    const qreal alphaScale = maskColor.alphaF();

    KisSequentialConstIterator it(device, rc);
    while(it.nextPixel()) {
        quint8 value = (MAX_SELECTED - *(it.rawDataConst())) * alphaScale;
        color.setAlpha(value);

        QPoint pt(it.x(), it.y());
        pt -= rc.topLeft();

        image.setPixel(pt.x(), pt.y(), color.rgba());
    }

    return image;
}

void KisPixelSelection::recalculateThumbnailImage(const QColor &maskColor)
{
    QRect rc = selectedExactRect();
    const int maxPreviewSize = 2000;

    if (rc.isEmpty()) {
        m_d->thumbnailImageTransform = QTransform();
        m_d->thumbnailImage = QImage();
        return;
    }


    if (rc.width() > maxPreviewSize ||
        rc.height() > maxPreviewSize) {

        qreal factor = 1.0;

        if (rc.width() > rc.height()) {
            factor = qreal(maxPreviewSize) / rc.width();
        } else {
            factor = qreal(maxPreviewSize) / rc.height();
        }

        int newWidth = qRound(rc.width() * factor);
        int newHeight = qRound(rc.height() * factor);

        m_d->thumbnailImageTransform =
            QTransform::fromScale(qreal(rc.width()) / newWidth,
                                  qreal(rc.height()) / newHeight) *
            QTransform::fromTranslate(rc.x(), rc.y());

        KisPaintDeviceSP thumbDevice =
            createThumbnailDevice(newWidth, newHeight, rc);

        QRect thumbRect(0, 0, newWidth, newHeight);
        m_d->thumbnailImage = deviceToQImage(thumbDevice, thumbRect, maskColor);

    } else {
        m_d->thumbnailImageTransform = QTransform::fromTranslate(rc.x(), rc.y());
        m_d->thumbnailImage = deviceToQImage(this, rc, maskColor);
    }

    m_d->thumbnailImageValid = true;
}

void KisPixelSelection::setParentSelection(KisSelectionWSP selection)
{
    m_d->parentSelection = selection;
}

KisSelectionWSP KisPixelSelection::parentSelection() const
{
    return m_d->parentSelection;
}

void KisPixelSelection::renderToProjection(KisPaintDeviceSP projection)
{
    renderToProjection(projection, selectedExactRect());
}

void KisPixelSelection::renderToProjection(KisPaintDeviceSP projection, const QRect& rc)
{
    QRect updateRect = rc & selectedExactRect();

    if (updateRect.isValid()) {
        KisPainter::copyAreaOptimized(updateRect.topLeft(), KisPaintDeviceSP(this), projection, updateRect);
    }
}
