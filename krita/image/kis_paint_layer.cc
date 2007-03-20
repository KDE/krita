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
#include <QIcon>
#include <QImage>
#include <QUndoCommand>

#include <KoColorProfile.h>

#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_meta_registry.h"
#include "KoColorSpaceRegistry.h"
#include "kis_datamanager.h"
#include "kis_undo_adapter.h"



class KisPaintLayer::Private
{
public:
    KisPaintDeviceSP paintdev;
    KisPaintDeviceSP mask; // The mask that we can edit and display easily
    KisSelectionSP maskAsSelection; //
                                    // The mask as selection, to apply and render easily
    KisPaintDeviceSP projection;

    KisMaskSP transparencyMask;
    KisMaskSP protectionMask;
    KisSelectionSP selection;
    vKisMaskSP effectMasks; // The first mask is the bottom-one in the
                            // mask stack!

    bool renderMask;
    bool editMask;
};

KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev)
    : super(img, name, opacity)
    , m_d( new Private() )
{
    Q_ASSERT(img);
    Q_ASSERT(dev);
    m_d->paintdev = dev;
    m_d->paintdev->setParentLayer(this);
    init();
}


KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity)
    : super(img, name, opacity)
    , m_d( new Private() )
{
    Q_ASSERT(img);
    m_d->paintdev = new KisPaintDevice(this, img->colorSpace(), name);
    init();
}

KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KoColorSpace * colorSpace)
    : super(img, name, opacity)
    , m_d( new Private() )
{
    Q_ASSERT(img);
    Q_ASSERT(colorSpace);
    m_d->paintdev = new KisPaintDevice(this, colorSpace, name);
    init();
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs) :
    KisLayer(rhs), KisLayerSupportsIndirectPainting(rhs)
{
    m_d->paintdev = new KisPaintDevice( *rhs.m_d->paintdev.data() );
    m_d->paintdev->setParentLayer(this);
    init();
    if (rhs.hasMask()) {
        m_d->mask = new KisPaintDevice(*rhs.m_d->mask.data());
        m_d->mask->setParentLayer(this);
    }
    m_d->renderMask = rhs.m_d->renderMask;
    m_d->editMask = rhs.m_d->editMask;
}

void KisPaintLayer::init()
{
    m_d->paintdev->startBackgroundFilters();
    connect(m_d->paintdev.data(), SIGNAL(colorSpaceChanged(KoColorSpace*)), this, SLOT(slotColorSpaceChanged()));
    connect(m_d->paintdev.data(), SIGNAL(profileChanged(KoColorProfile*)), this, SLOT(slotColorSpaceChanged()));

    m_d->mask = 0;
    m_d->maskAsSelection = 0;
    m_d->renderMask = false;
    m_d->editMask = true;
}

QIcon KisPaintLayer::icon() const
{
    return QIcon();
}

KoDocumentSectionModel::PropertyList KisPaintLayer::properties() const
{
    PropertyList l = super::properties();
    l << Property(i18n("Colorspace"), paintDevice()->colorSpace()->name());
    if( KoColorProfile *profile = paintDevice()->colorSpace()->profile() )
        l << Property(i18n("Profile"), profile->productName());
    return l;
}

KisLayerSP KisPaintLayer::clone() const
{
    // ### porting 1.6->2.0 lost here an if (m_d->mask != 0) m_d->mask->setParentLayer(0); ???
    return KisLayerSP(new KisPaintLayer(*this));
}

KisPaintLayer::~KisPaintLayer()
{
    if (!m_d->paintdev.isNull()) {
        m_d->paintdev->setParentLayer(0);
    }
    delete m_d;
}

void KisPaintLayer::paint(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h)
{
    if (m_d->paintdev && m_d->paintdev->hasSelection()) {
        m_d->paintdev->selection()->paint(img, x, y, w, h);
    } else if (m_d->mask && m_d->editMask && m_d->mask->hasSelection()) {
        m_d->mask->selection()->paint(img, x, y, w, h);
    }
}

void KisPaintLayer::paint(QImage &img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize)
{
    if (m_d->paintdev && m_d->paintdev->hasSelection()) {
        m_d->paintdev->selection()->paint(img, scaledImageRect, scaledImageSize, imageSize);
    } else if (m_d->mask && m_d->editMask && m_d->mask->hasSelection()) {
        m_d->mask->selection()->paint(img, scaledImageRect, scaledImageSize, imageSize);
    }
}

void KisPaintLayer::paintMaskInactiveLayers(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h)
{
    uchar *j = img.bits();

    KoColorSpace *cs = m_d->paintdev->colorSpace();
    KisHLineConstIteratorPixel it = m_d->paintdev->createHLineIterator(x, y, w);
    for (qint32 y2 = y; y2 < h + y; ++y2) {

        while ( ! it.isDone()) {
            quint8 s = cs->alpha(it.rawData());
            if(s==0)
            {
                quint8 g = (*(j + 0)  + *(j + 1 ) + *(j + 2 )) / 9;

                *(j+0) = 128+g ;
                *(j+1) = 165+g;
                *(j+2) = 128+g;
            }
            j+=4;
            ++it;
        }
        it.nextRow();
    }
}

QImage KisPaintLayer::createThumbnail(qint32 w, qint32 h)
{
    if (m_d->paintdev)
        return m_d->paintdev->createThumbnail(w, h);
    else
        return QImage();
}

/// Returns the paintDevice that accompanies this layer
KisPaintDeviceSP KisPaintLayer::paintDevice() const
{
    return m_d->paintdev;
}

/// Returns the paintDevice that accompanies this layer (or mask, see editMask)
KisPaintDeviceSP KisPaintLayer::paintDeviceOrMask() const
{
    if (hasMask() && editMask())
        return m_d->mask;
    return m_d->paintdev;
}

bool KisPaintLayer::hasMask() const
{
    return !m_d->mask.isNull();
}

bool KisPaintLayer::renderMask() const
{
    return m_d->renderMask;
}
qint32 KisPaintLayer::x() const { if (m_d->paintdev) return m_d->paintdev->getX(); else return 0; }

void KisPaintLayer::setX(qint32 x)
{
    if (m_d->paintdev)
        m_d->paintdev->setX(x);
}

qint32 KisPaintLayer::y() const {
    if (m_d->paintdev)
        return m_d->paintdev->getY();
    else
        return 0;
}
void KisPaintLayer::setY(qint32 y) {
    if (m_d->paintdev)
        m_d->paintdev->setY(y);
}

QRect KisPaintLayer::extent() const {
    if (m_d->paintdev)
        return m_d->paintdev->extent();
    else
        return QRect();
}

QRect KisPaintLayer::exactBounds() const {
    if (m_d->paintdev)
        return m_d->paintdev->exactBounds();
    else
        return QRect();
}

void KisPaintLayer::slotColorSpaceChanged()
{
    notifyPropertyChanged(this);
}

void KisPaintLayer::removeMask() {
    if (!hasMask())
        return;

    m_d->mask->setParentLayer(0);
    m_d->mask = 0;
    m_d->maskAsSelection = 0;
    setDirty();

    emit sigMaskInfoChanged();
}

// ### XXX Do we apply the mask outside the image boundaries too? I'd say no, but I'm not sure
void KisPaintLayer::applyMask() {
    if (!hasMask())
        return;

    int x, y, w, h;
    m_d->paintdev->extent(x, y, w, h);

    // A bit slow; but it works
    KisPaintDeviceSP temp = new KisPaintDevice(m_d->paintdev->colorSpace());
    KisPainter gc(temp);
    gc.bltSelection(x, y, COMPOSITE_OVER, m_d->paintdev, m_d->maskAsSelection, OPACITY_OPAQUE, x, y, w, h);
    gc.end();
    gc.begin(m_d->paintdev);
    gc.bitBlt(x, y, COMPOSITE_COPY, temp, OPACITY_OPAQUE, x, y, w, h);
    gc.end();

    removeMask();
}

KisPaintDeviceSP KisPaintLayer::createMask() {
    if (hasMask())
        return m_d->mask;

    // Grey8 nicely fits our needs of being intuitively comparable to other apps'
    // mask layer interfaces. It does have an alpha component though, which is a bit
    // less appropriate in this context.
    // Bart: why not use ALPHA -- that is a single channel 8-bit
    // colorspace, which is best suited for this purpose (BSAR).
    m_d->mask = new KisPaintDevice(KoColorSpaceRegistry::instance()
            ->colorSpace(KoID("GRAYA"), 0));

    genericMaskCreationHelper();

    return m_d->mask;
}

// FIXME If from is a paint device is not grey8!!
void KisPaintLayer::createMaskFromPaintDevice(KisPaintDeviceSP from) {
    if (hasMask())
        return; // Or overwrite? XXX

    m_d->mask = from; // KisPaintDevice(*from); XXX

    genericMaskCreationHelper();
}

void KisPaintLayer::createMaskFromSelection(KisSelectionSP from) {
    m_d->mask = new KisPaintDevice(KoColorSpaceRegistry::instance()
            ->colorSpace(KoID("GRAYA"), 0));
    m_d->mask->setParentLayer(this);

    m_d->maskAsSelection = new KisSelection(); // Anonymous selection is good enough

    // Default pixel is opaque white == don't mask?
    Q_UINT8 const defPixel[] = { 255, 255 };
    m_d->mask->dataManager()->setDefaultPixel(defPixel);

    if (from) {
        QRect r(extent());

        int w = r.width();
        int h = r.height();

        KisHLineConstIteratorPixel srcIt = from->createHLineIterator(r.x(), r.y(), w);
        KisHLineIteratorPixel dstIt = m_d->mask->createHLineIterator(r.x(), r.y(), w);

        for (int y = r.y(); y < h; y++) {

            while(!dstIt.isDone()) {
                // XXX same remark as in convertMaskToSelection
                *dstIt.rawData() = *srcIt.rawData();
                ++srcIt;
                ++dstIt;
            }
            srcIt.nextRow();
            dstIt.nextRow();
        }
    }

    convertMaskToSelection(extent());
    m_d->paintdev->deselect();

    setDirty();
    emit sigMaskInfoChanged();
}

KisPaintDeviceSP KisPaintLayer::getMask() {
    createMask();
    return m_d->mask;
}

KisSelectionSP KisPaintLayer::getMaskAsSelection() {
    createMask();
    return m_d->maskAsSelection;
}

bool KisPaintLayer::editMask() const
{
    return m_d->editMask;
}

void KisPaintLayer::setEditMask(bool b)
{
    m_d->editMask = b;
    emit sigMaskInfoChanged();
}

void KisPaintLayer::setRenderMask(bool b)
{
    m_d->renderMask = b;

    if (hasMask())
        setDirty();

    emit sigMaskInfoChanged();
}

void KisPaintLayer::convertMaskToSelection(const QRect& r)
{
    KisRectConstIteratorPixel srcIt = m_d->mask->createRectConstIterator(r.x(), r.y(),
            r.width(), r.height());
    KisRectIteratorPixel dstIt = m_d->maskAsSelection->createRectIterator(r.x(), r.y(),
            r.width(), r.height());

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
    m_d->mask->setParentLayer(this);

    m_d->maskAsSelection = new KisSelection(); // Anonymous selection is good enough

    // Default pixel is opaque white == don't mask?
    Q_UINT8 const defPixel[] = { 255, 255 };
    m_d->mask->dataManager()->setDefaultPixel(defPixel);

    setDirty();
    emit sigMaskInfoChanged();
}

void KisPaintLayer::setDirty() {
    QRect rc = extent();
    if (hasMask())
        convertMaskToSelection(rc);
    super::setDirty(rc);
}

void KisPaintLayer::setDirty(const QRect & rect) {
    if (hasMask())
        convertMaskToSelection(rect);
    super::setDirty(rect);
}

void KisPaintLayer::setDirty(const QRegion & region) {
    if (hasMask()) {
        QVector<QRect> regionRects = region.rects();
        QVector<QRect>::iterator it = regionRects.begin();
        QVector<QRect>::iterator end = regionRects.end();
        while ( it != end ) {
            convertMaskToSelection(*it);
            ++it;
        }
    }
    super::setDirty(region);
}

void KisPaintLayer::resetProjection(KisPaintDeviceSP to)
{
    if (to)
        m_d->projection = new KisPaintDevice(*to); /// XXX ### look into Copy on Write here (CoW)
    else
        m_d->projection = new KisPaintDevice(this, image()->colorSpace(), name().toLatin1());

}

KisPaintDeviceSP KisPaintLayer::projection()
{
    // There are masks about that change the rendering, and there's no
    // projection yet.
    if ( !m_d->projection && ( m_d->transparencyMask || !m_d->effectMasks.isEmpty() ) ) {
        updateProjection( m_d->paintdev->extent() );
    }

    if ( m_d->projection )
        return m_d->projection;
    else
        return m_d->paintdev;
}

void KisPaintLayer::updateProjection(const QRect & rc)
{
    // Nothing that changes the rendering, get rid of the projection
    if ( !m_d->transparencyMask && m_d->effectMasks.isEmpty() ) {
        m_d->projection = 0;
    }

    KisPainter gc( m_d->projection );
    gc.setCompositeOp( m_d->paintdev->colorSpace()->compositeOp( COMPOSITE_COPY ) );
    gc.bitBlt( rc.topLeft(), m_d->paintdev, rc );

    foreach( KisMaskSP mask, m_d->effectMasks ) {
        // XXX: Filter the projection (once the effect mask class is done
    }

    // XXX: Filter the alpha with the transparency mask (once the apha
    // filter is done)

    // Done.
}



// Undoable versions code
namespace {
    class KisCreateMaskCommand : public QUndoCommand {
        typedef QUndoCommand super;
        KisPaintLayerSP m_layer;
        KisPaintDeviceSP m_mask;
    public:
        KisCreateMaskCommand(KisPaintLayer* layer)
            : super(i18n("Create Layer Mask")), m_layer(layer) {}
        virtual void redo() {
            if (!m_mask)
                m_mask = m_layer->createMask();
            else
                m_layer->createMaskFromPaintDevice(m_mask);
        }
        virtual void undo() {
            m_layer->removeMask();
        }
    };

    class KisMaskFromSelectionCommand : public QUndoCommand {
        typedef QUndoCommand super;
        KisPaintLayerSP m_layer;
        KisPaintDeviceSP m_maskBefore;
        KisPaintDeviceSP m_maskAfter;
        KisSelectionSP m_selection;
    public:
        KisMaskFromSelectionCommand(KisPaintLayer* layer)
            : super(i18n("Mask From Selection")), m_layer(layer) {
            if (m_layer->hasMask())
                m_maskBefore = m_layer->getMask();
            else
                m_maskBefore = 0;
            m_maskAfter = 0;
            if (m_layer->paintDevice()->hasSelection())
                m_selection = m_layer->paintDevice()->selection();
            else
                m_selection = 0;
        }
        virtual void redo() {
            if (!m_maskAfter) {
                m_layer->createMaskFromSelection(m_selection);
                m_maskAfter = m_layer->getMask();
            } else {
                m_layer->paintDevice()->deselect();
                m_layer->createMaskFromPaintDevice(m_maskAfter);
            }
        }
        virtual void undo() {
            m_layer->paintDevice()->setSelection(m_selection);
            if (m_maskBefore)
                m_layer->createMaskFromPaintDevice(m_maskBefore);
            else
                m_layer->removeMask();
        }
    };

    class KisMaskToSelectionCommand : public QUndoCommand {
        typedef QUndoCommand super;
        KisPaintLayerSP m_layer;
        KisPaintDeviceSP m_mask;
        KisSelectionSP m_selection;
    public:
        KisMaskToSelectionCommand(KisPaintLayer* layer)
            : super(i18n("Mask To Selection")), m_layer(layer) {
            m_mask = m_layer->getMask();
            if (m_layer->paintDevice()->hasSelection())
                m_selection = m_layer->paintDevice()->selection();
            else
                m_selection = 0;
        }
        virtual void redo() {
            m_layer->paintDevice()->setSelection(m_layer->getMaskAsSelection());
            m_layer->removeMask();
        }
        virtual void undo() {
            if (m_selection)
                m_layer->paintDevice()->setSelection(m_selection);
            else
                m_layer->paintDevice()->deselect();
            m_layer->createMaskFromPaintDevice(m_mask);
        }
    };


    class KisRemoveMaskCommand : public QUndoCommand {
        typedef QUndoCommand super;
        KisPaintLayerSP m_layer;
        KisPaintDeviceSP m_mask;
    public:
        KisRemoveMaskCommand(KisPaintLayer* layer)
            : super(i18n("Remove Layer Mask")), m_layer(layer) {
            m_mask = m_layer->getMask();
            }
            virtual void redo() {
                m_layer->removeMask();
            }
            virtual void undo() {
                // I hope that if the undo stack unwinds, it will end up here in the right
                // state again; taking a deep-copy sounds like wasteful to me
                m_layer->createMaskFromPaintDevice(m_mask);
            }
    };

    class KisApplyMaskCommand : public QUndoCommand {
        typedef QUndoCommand super;
        KisPaintLayerSP m_layer;
        KisPaintDeviceSP m_mask;
        KisPaintDeviceSP m_original;
    public:
        KisApplyMaskCommand(KisPaintLayer* layer)
            : super(i18n("Apply Layer Mask")), m_layer(layer) {
            m_mask = m_layer->getMask();
            m_original = new KisPaintDevice(*m_layer->paintDevice());
        }
        virtual void redo() {
            m_layer->applyMask();
        }
        virtual void undo() {
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

QUndoCommand* KisPaintLayer::createMaskCommand() {
    return new KisCreateMaskCommand(this);
}

QUndoCommand* KisPaintLayer::maskFromSelectionCommand() {
    return new KisMaskFromSelectionCommand(this);
}

QUndoCommand* KisPaintLayer::maskToSelectionCommand() {
    return new KisMaskToSelectionCommand(this);
}

QUndoCommand* KisPaintLayer::removeMaskCommand() {
    return new KisRemoveMaskCommand(this);
}

QUndoCommand* KisPaintLayer::applyMaskCommand() {
    return new KisApplyMaskCommand(this);
}



#include "kis_paint_layer.moc"
