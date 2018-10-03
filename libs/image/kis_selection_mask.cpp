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


struct Q_DECL_HIDDEN KisSelectionMask::Private
{
public:
    Private(KisSelectionMask *_q)
        : q(_q)
        , updatesCompressor(0)
    {}
    KisSelectionMask *q;
    KisImageWSP image;
    KisThreadSafeSignalCompressor *updatesCompressor;

    void slotSelectionChangedCompressed();
};

KisSelectionMask::KisSelectionMask(KisImageWSP image)
    : KisMask("selection")
    , m_d(new Private(this))
{
    setActive(false);

    m_d->image = image;

    m_d->updatesCompressor =
            new KisThreadSafeSignalCompressor(300, KisSignalCompressor::POSTPONE);

    connect(m_d->updatesCompressor, SIGNAL(timeout()), SLOT(slotSelectionChangedCompressed()));
    this->moveToThread(image->thread());
}

KisSelectionMask::KisSelectionMask(const KisSelectionMask& rhs)
    : KisMask(rhs)
    , m_d(new Private(this))
{
    m_d->image = rhs.image();
    m_d->updatesCompressor =
            new KisThreadSafeSignalCompressor(300, KisSignalCompressor::POSTPONE);

    connect(m_d->updatesCompressor, SIGNAL(timeout()), SLOT(slotSelectionChangedCompressed()));
    this->moveToThread(m_d->image->thread());
}

KisSelectionMask::~KisSelectionMask()
{
    m_d->updatesCompressor->deleteLater();
    delete m_d;
}

QIcon KisSelectionMask::icon() const {
    return KisIconUtils::loadIcon("selectionMask");
}

void KisSelectionMask::setSelection(KisSelectionSP selection)
{
    if (selection) {
        KisMask::setSelection(selection);
    } else {
        KisMask::setSelection(new KisSelection());

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
    KisMask::setSectionModelProperties(properties);
    setActive(properties.at(2).state.toBool());
}

void KisSelectionMask::setVisible(bool visible, bool isLoading)
{
    setNodeProperty("visible", visible);

    if (!isLoading) {
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
        if (activeMask) {
            activeMask->setActive(false);
        }
    }

    setNodeProperty("active", active);

    if (image) {
        image->nodeChanged(this);
        image->undoAdapter()->emitSelectionChanged();
    }
}

void KisSelectionMask::notifySelectionChangedCompressed()
{
    m_d->updatesCompressor->start();
}

void KisSelectionMask::Private::slotSelectionChangedCompressed()
{
    KisSelectionSP currentSelection = q->selection();
    if (!currentSelection) return;

    currentSelection->notifySelectionChanged();
}

#include "moc_kis_selection_mask.cpp"
