/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <kdebug.h>
#include <qimage.h>

#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_datamanager.h"
#include "kis_undo_adapter.h"

KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisPaintDeviceSP dev)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    Q_ASSERT(dev);
    m_paintdev = dev;
    m_mask = 0;
    m_maskAsSelection = 0;
    m_paintdev->setParentLayer(this);
    m_renderMask = false;
    m_editMask = true;
}


KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    m_paintdev = new KisPaintDevice(this, img->colorSpace(), name.latin1());
    m_mask = 0;
    m_maskAsSelection = 0;
    m_renderMask = false;
    m_editMask = true;
}

KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisColorSpace * colorSpace)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    Q_ASSERT(colorSpace);
    m_paintdev = new KisPaintDevice(this, colorSpace,  name.latin1());
    m_mask = 0;
    m_maskAsSelection = 0;
    m_renderMask = false;
    m_editMask = true;
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs) : KisLayer(rhs)
{
    m_paintdev = new KisPaintDevice( *rhs.m_paintdev.data() );
    m_paintdev->setParentLayer(this);
    m_mask = new KisPaintDevice(*rhs.m_mask.data());
    m_mask->setParentLayer(this);
    m_renderMask = rhs.m_renderMask;
    m_editMask = rhs.m_editMask;
}

KisLayerSP KisPaintLayer::clone() const
{
    return new KisPaintLayer(*this);
}

KisPaintLayer::~KisPaintLayer()
{
    if (m_paintdev != 0) {
        m_paintdev->setParentLayer(0);
    }
    if (m_mask != 0) {
        m_mask->setParentLayer(0);
    }
}

void KisPaintLayer::paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    if (m_paintdev->hasSelection())
            m_paintdev->selection()->paintSelection(img, x, y, w, h);
}

void KisPaintLayer::paintSelection(QImage &img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize)
{
    if (m_paintdev && m_paintdev->hasSelection()) {
        m_paintdev->selection()->paintSelection(img, scaledImageRect, scaledImageSize, imageSize);
    }
}

void KisPaintLayer::paintMaskInactiveLayers(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    uchar *j = img.bits();

    KisColorSpace *cs = m_paintdev->colorSpace();

    for (Q_INT32 y2 = y; y2 < h + y; ++y2) {
        KisHLineIteratorPixel it = m_paintdev->createHLineIterator(x, y2, w, false);
        while ( ! it.isDone()) {
            Q_UINT8 s = cs->getAlpha(it.rawData());
            if(s==0)
            {
                Q_UINT8 g = (*(j + 0)  + *(j + 1 ) + *(j + 2 )) / 9;

                *(j+0) = 128+g ;
                *(j+1) = 165+g;
                *(j+2) = 128+g;
            }
            j+=4;
            ++it;
        }
    }
}

QImage KisPaintLayer::createThumbnail(Q_INT32 w, Q_INT32 h)
{
    if (m_paintdev)
        return m_paintdev->createThumbnail(w, h);
    else
        return QImage();
}


Q_INT32 KisPaintLayer::x() const {
    if (m_paintdev)
        return m_paintdev->getX();
    else return 0;
}

void KisPaintLayer::setX(Q_INT32 x)
{
    if (m_paintdev)
        m_paintdev->setX(x);
}

Q_INT32 KisPaintLayer::y() const {
    if (m_paintdev)
        return m_paintdev->getY();
    else
        return 0;
}

void KisPaintLayer::setY(Q_INT32 y) {
    if (m_paintdev)
        m_paintdev->setY(y);
}

QRect KisPaintLayer::extent() const {
    if (m_paintdev)
        return m_paintdev->extent();
    else
        return QRect();
}

QRect KisPaintLayer::exactBounds() const {
    if (m_paintdev)
        return m_paintdev->exactBounds();
    else
        return QRect();
}

void KisPaintLayer::removeMask() {
    if (!hasMask())
        return;

    m_mask->setParentLayer(0);
    m_mask = 0;
    m_maskAsSelection = 0;
    setDirty();

    emit sigMaskInfoChanged();
}

// ### XXX Do we apply the mask outside the image boundaries too? I'd say no, but I'm not sure
void KisPaintLayer::applyMask() {
    if (!hasMask())
        return;

    int x, y, w, h;
    m_paintdev->extent(x, y, w, h);

    KisPainter gc(m_paintdev);
    gc.bltSelection(x, y, COMPOSITE_OVER, m_paintdev, m_maskAsSelection, OPACITY_OPAQUE, x, y, w, h);
    gc.end();

    removeMask();
}

KisPaintDeviceSP KisPaintLayer::createMask() {
    if (hasMask())
        return m_mask;

    kdDebug() << k_funcinfo << endl;
    // Grey8 nicely fits our needs of being intuitively comparable to other apps'
    // mask layer interfaces. It does have an alpha component though, which is a bit
    // less appropriate in this context.
    m_mask = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()
            ->getColorSpace(KisID("GRAYA"), 0));

    genericMaskCreationHelper();

    return m_mask;
}

// FIXME If from is a paint device is not grey8!!
void KisPaintLayer::createMaskFromPaintDevice(KisPaintDeviceSP from) {
    if (hasMask())
        return; // Or overwrite? XXX

    kdDebug() << k_funcinfo << endl;
    m_mask = from; // KisPaintDevice(*from); XXX

    genericMaskCreationHelper();
}

// Not tested
void KisPaintLayer::createMaskFromSelection(KisSelectionSP from) {
    if (hasMask())
        return; // Or overwrite? XXX

    kdDebug() << k_funcinfo << endl;
    m_mask = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()
            ->getColorSpace(KisID("GRAYA"), 0));

    QRect r(from->extent());

    KisRectIteratorPixel srcIt = from->createRectIterator(r.x(), r.y(),
            r.width(), r.height(), false);
    KisRectIteratorPixel dstIt = m_mask->createRectIterator(r.x(), r.y(),
            r.width(), r.height(), true);

    while(!dstIt.isDone()) {
        // XXX same remark as in convertMaskToSelection
        *dstIt.rawData() = *srcIt.rawData();
        ++srcIt;
        ++dstIt;
    }

    convertMaskToSelection(extent());

    genericMaskCreationHelper();
}

KisPaintDeviceSP KisPaintLayer::getMask() {
    createMask();
    kdDebug() << k_funcinfo << endl;
    return m_mask;
}

KisSelectionSP KisPaintLayer::getMaskAsSelection() {
    createMask();
    kdDebug() << k_funcinfo << endl;
    return m_maskAsSelection;
}

void KisPaintLayer::setEditMask(bool b) {
    m_editMask = b;
    emit sigMaskInfoChanged();
}

void KisPaintLayer::setRenderMask(bool b) {
    m_renderMask = b;

    if (hasMask())
        setDirty();

    emit sigMaskInfoChanged();
}

void KisPaintLayer::convertMaskToSelection(const QRect& r) {
    KisRectIteratorPixel srcIt = m_mask->createRectIterator(r.x(), r.y(),
            r.width(), r.height(), false);
    KisRectIteratorPixel dstIt = m_maskAsSelection->createRectIterator(r.x(), r.y(),
            r.width(), r.height(), true);

    while(!dstIt.isDone()) {
        // src is grey8 (grey + alpha), dst is alpha8. We convert the grey value to
        // alpha8 manually and ignore the alpha (that's why we don't convert using default
        // functions, and interpret the data raw!) [ XXX ]
        *dstIt.rawData() = *srcIt.rawData();
        ++srcIt;
        ++dstIt;
    }
}

void KisPaintLayer::genericMaskCreationHelper() {
    m_mask->setParentLayer(this);

    m_maskAsSelection = new KisSelection(); // Anonymous selection is good enough

    // Default pixel is opaque white == don't mask?
    Q_UINT8 const defPixel[] = { 255, 255 };
    m_mask->dataManager()->setDefaultPixel(defPixel);

    setDirty();
    emit sigMaskInfoChanged();
}

void KisPaintLayer::setDirty(bool propagate) {
    if (hasMask())
        convertMaskToSelection(extent());
    super::setDirty(propagate);
}

void KisPaintLayer::setDirty(const QRect & rect, bool propagate) {
    if (hasMask())
        convertMaskToSelection(rect);
    super::setDirty(rect, propagate);
}

// Undoable versions code
namespace {
    class KisCreateMaskCommand : public KNamedCommand {
    typedef KNamedCommand super;
    KisPaintLayerSP m_layer;
    public:
        KisCreateMaskCommand(const QString& name, KisPaintLayer* layer)
            : super(name), m_layer(layer) {}
        virtual void execute() {
            kdDebug() << k_funcinfo << endl;
            m_layer->createMask();
        }
        virtual void unexecute() {
            m_layer->removeMask();
        }
    };

    class KisRemoveMaskCommand : public KNamedCommand {
    typedef KNamedCommand super;
    KisPaintLayerSP m_layer;
    KisPaintDeviceSP m_mask;
    public:
        KisRemoveMaskCommand(const QString& name, KisPaintLayer* layer)
            : super(name), m_layer(layer) {
            m_mask = m_layer->getMask();
        }
        virtual void execute() {
            kdDebug() << k_funcinfo << endl;
            m_layer->removeMask();
        }
        virtual void unexecute() {
            // I hope that if the undo stack unwinds, it will end up here in the right
            // state again; taking a deep-copy sounds like wasteful to me
            m_layer->createMaskFromPaintDevice(m_mask);
        }
    };

    class KisApplyMaskCommand : public KNamedCommand {
        typedef KNamedCommand super;
        KisPaintLayerSP m_layer;
        KisPaintDeviceSP m_mask;
        KisPaintDeviceSP m_original;
        public:
            KisApplyMaskCommand(const QString& name, KisPaintLayer* layer)
            : super(name), m_layer(layer) {
                m_mask = m_layer->getMask();
                m_original = new KisPaintDevice(*m_layer->paintDevice());
            }
            virtual void execute() {
                m_layer->applyMask();
            }
            virtual void unexecute() {
                // I hope that if the undo stack unwinds, it will end up here in the right
                // state again; taking a deep-copy sounds like wasteful to me
                KisPainter gc(m_layer->paintDevice());
                int x, y, w, h;
                m_layer->paintDevice()->extent(x, y, w, h);

                gc.bitBlt(x, y, COMPOSITE_COPY, m_original, OPACITY_OPAQUE, x, y, w, h);
                gc.end();

                m_layer->createMaskFromPaintDevice(m_mask);
            }
    };
}

KNamedCommand* KisPaintLayer::createMaskCommand() {
    return new KisCreateMaskCommand(i18n("Create Layer Mask"), this);
}

KNamedCommand* KisPaintLayer::removeMaskCommand() {
    return new KisRemoveMaskCommand(i18n("Remove Layer Mask"), this);
}

KNamedCommand* KisPaintLayer::applyMaskCommand() {
    return new KisApplyMaskCommand(i18n("Apply Layer Mask"), this);
}


#include "kis_paint_layer.moc"
