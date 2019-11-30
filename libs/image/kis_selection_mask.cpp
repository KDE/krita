/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_selection_mask.h"

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoProperties.h>
#include "kis_fill_painter.h"
#include <KoCompositeOp.h>
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_pixel_selection.h"
#include "kis_undo_adapter.h"
#include <KoIcon.h>
#include <kis_icon.h>
#include "kis_thread_safe_signal_compressor.h"
#include "kis_layer_properties_icons.h"
#include "kis_cached_paint_device.h"

#include "kis_image_config.h"
#include "KisImageConfigNotifier.h"


struct Q_DECL_HIDDEN KisSelectionMask::Private
{
public:
    Private(KisSelectionMask *_q)
        : q(_q)
        , updatesCompressor(0)
        , maskColor(Qt::green, KoColorSpaceRegistry::instance()->rgb8())
    {}
    KisSelectionMask *q;
    KisImageWSP image;
    KisCachedPaintDevice paintDeviceCache;
    KisCachedSelection cachedSelection;
    KisThreadSafeSignalCompressor *updatesCompressor;
    KoColor maskColor;

    void slotSelectionChangedCompressed();
    void slotConfigChangedImpl(bool blockUpdates);
    void slotConfigChanged();
};

KisSelectionMask::KisSelectionMask(KisImageWSP image)
    : KisEffectMask()
    , m_d(new Private(this))
{
    setName("selection");
    setActive(false);
    setSupportsLodMoves(false);

    m_d->image = image;

    m_d->updatesCompressor =
            new KisThreadSafeSignalCompressor(50, KisSignalCompressor::FIRST_ACTIVE);

    connect(m_d->updatesCompressor, SIGNAL(timeout()), SLOT(slotSelectionChangedCompressed()));
    this->moveToThread(image->thread());

    connect(KisImageConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    m_d->slotConfigChangedImpl(false);
}

KisSelectionMask::KisSelectionMask(const KisSelectionMask& rhs)
    : KisEffectMask(rhs)
    , m_d(new Private(this))
{
    m_d->image = rhs.image();
    m_d->updatesCompressor =
            new KisThreadSafeSignalCompressor(300, KisSignalCompressor::POSTPONE);

    connect(m_d->updatesCompressor, SIGNAL(timeout()), SLOT(slotSelectionChangedCompressed()));
    this->moveToThread(m_d->image->thread());

    connect(KisImageConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    m_d->slotConfigChangedImpl(false);
}

KisSelectionMask::~KisSelectionMask()
{
    m_d->updatesCompressor->deleteLater();
    delete m_d;
}

QIcon KisSelectionMask::icon() const {
    return KisIconUtils::loadIcon("selectionMask");
}

void KisSelectionMask::mergeInMaskInternal(KisPaintDeviceSP projection,
                                           KisSelectionSP effectiveSelection,
                                           const QRect &applyRect,
                                           const QRect &preparedNeedRect,
                                           KisNode::PositionToFilthy maskPos) const
{
    Q_UNUSED(maskPos);
    Q_UNUSED(preparedNeedRect);
    if (!effectiveSelection) return;

    {
        KisSelectionSP mainMaskSelection = this->selection();
        if (mainMaskSelection &&
            (!mainMaskSelection->isVisible() ||
             mainMaskSelection->pixelSelection()->defaultBounds()->externalFrameActive())) {

            return;
        }
    }

    KisCachedPaintDevice::Guard d1(projection, m_d->paintDeviceCache);
    KisPaintDeviceSP fillDevice = d1.device();
    fillDevice->setDefaultPixel(m_d->maskColor);

    const QRect selectionExtent = effectiveSelection->selectedRect();

    if (selectionExtent.contains(applyRect) || selectionExtent.intersects(applyRect)) {
        KisCachedSelection::Guard s1(m_d->cachedSelection);
        KisSelectionSP invertedSelection = s1.selection();

        invertedSelection->pixelSelection()->makeCloneFromRough(effectiveSelection->pixelSelection(), applyRect);
        invertedSelection->pixelSelection()->invert();

        KisPainter gc(projection);
        gc.setSelection(invertedSelection);
        gc.bitBlt(applyRect.topLeft(), fillDevice, applyRect);

    } else {
        KisPainter gc(projection);
        gc.bitBlt(applyRect.topLeft(), fillDevice, applyRect);
    }
}

bool KisSelectionMask::paintsOutsideSelection() const
{
    return true;
}

void KisSelectionMask::setSelection(KisSelectionSP selection)
{
    if (selection) {
        KisEffectMask::setSelection(selection);
    } else {
        KisEffectMask::setSelection(new KisSelection());

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->alpha8();
        KisFillPainter gc(KisPaintDeviceSP(this->selection()->pixelSelection().data()));
        gc.fillRect(image()->bounds(), KoColor(Qt::white, cs), MAX_SELECTED);
        gc.end();
    }
    setDirty();
}

KisImageWSP KisSelectionMask::image() const
{
    return m_d->image;
}

bool KisSelectionMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisSelectionMask::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

KisBaseNode::PropertyList KisSelectionMask::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisBaseNode::sectionModelProperties();
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::selectionActive, active());
    return l;
}

void KisSelectionMask::setSectionModelProperties(const KisBaseNode::PropertyList &properties)
{
    KisEffectMask::setSectionModelProperties(properties);
    setActive(properties.at(2).state.toBool());
}

void KisSelectionMask::setVisible(bool visible, bool isLoading)
{
    const bool oldVisible = this->visible(false);
    setNodeProperty("visible", visible);

    if (!isLoading && visible != oldVisible) {
        if (selection())
            selection()->setVisible(visible);
    }
}

bool KisSelectionMask::active() const
{
    return nodeProperties().boolProperty("active", true);
}

void KisSelectionMask::setActive(bool active)
{
    KisImageWSP image = this->image();
    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent().data());

    if (active && parentLayer) {
        KisSelectionMaskSP activeMask = parentLayer->selectionMask();
        if (activeMask && activeMask != this) {
            activeMask->setActive(false);
        }
    }

    const bool oldActive = this->active();
    setNodeProperty("active", active);


    /**
     * WARNING: we have a direct link to the image here, but we
     * must not use it for notification until we are a part of
     * the nore graph! Notifications should be emitted iff we
     * have graph listener link set up.
     */
    if (graphListener() &&
        image && oldActive != active) {

        baseNodeChangedCallback();
        image->undoAdapter()->emitSelectionChanged();
    }
}

QRect KisSelectionMask::needRect(const QRect &rect, KisNode::PositionToFilthy pos) const
{
    Q_UNUSED(pos);

    // selection masks just add an overlay, so the needed rect is simply passed through
    return rect;
}

QRect KisSelectionMask::changeRect(const QRect &rect, KisNode::PositionToFilthy pos) const
{
    Q_UNUSED(pos);

    // selection masks just add an overlay, so the changed rect is simply passed through
    return rect;
}

QRect KisSelectionMask::extent() const
{
    // since mask overlay is inverted, the mask paints over
    // the entire image bounds

    QRect resultRect;

    KisSelectionSP selection = this->selection();

    if (selection) {
        resultRect = selection->pixelSelection()->defaultBounds()->bounds();
    } else if (KisNodeSP parent = this->parent()) {
        KisPaintDeviceSP dev = parent->projection();
        if (dev) {
            resultRect = dev->defaultBounds()->bounds();
        }
    }

    return resultRect;
}

QRect KisSelectionMask::exactBounds() const
{
    return extent();
}

void KisSelectionMask::notifySelectionChangedCompressed()
{
    m_d->updatesCompressor->start();
}

bool KisSelectionMask::decorationsVisible() const
{
    return selection()->isVisible();
}

void KisSelectionMask::setDecorationsVisible(bool value, bool update)
{
    if (value == decorationsVisible()) return;

    const QRect oldExtent = extent();

    selection()->setVisible(value);

    if (update) {
        setDirty(oldExtent | extent());
    }
}

void KisSelectionMask::flattenSelectionProjection(KisSelectionSP selection, const QRect &dirtyRect) const
{
    Q_UNUSED(selection);
    Q_UNUSED(dirtyRect);
}

void KisSelectionMask::Private::slotSelectionChangedCompressed()
{
    KisSelectionSP currentSelection = q->selection();
    if (!currentSelection) return;

    currentSelection->notifySelectionChanged();
}

void KisSelectionMask::Private::slotConfigChangedImpl(bool doUpdates)
{
    const KoColorSpace *cs = image ?
        image->colorSpace() :
        KoColorSpaceRegistry::instance()->rgb8();

    KisImageConfig cfg(true);

    maskColor = KoColor(cfg.selectionOverlayMaskColor(), cs);

    if (doUpdates && image && image->overlaySelectionMask() == q) {
        q->setDirty();
    }
}

void KisSelectionMask::Private::slotConfigChanged()
{
    slotConfigChangedImpl(true);
}

#include "moc_kis_selection_mask.cpp"
