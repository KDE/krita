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
#include "kis_fill_painter.h"
#include <KoCompositeOp.h>
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_pixel_selection.h"
#include "kis_undo_adapter.h"
#include <KoIcon.h>

struct KisSelectionMask::Private
{
public:
    KisImageWSP image;
};

KisSelectionMask::KisSelectionMask(KisImageWSP image)
        : KisMask("selection")
        , m_d(new Private())
{
    setActive(false);
    m_d->image = image;
}

KisSelectionMask::KisSelectionMask(const KisSelectionMask& rhs)
        : KisMask(rhs)
        , m_d(new Private())
{
    setActive(false);
    m_d->image = rhs.image();
}

KisSelectionMask::~KisSelectionMask()
{
    delete m_d;
}

QIcon KisSelectionMask::icon() const {
    return koIcon("edit-paste");
}

bool KisSelectionMask::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return false;
}


void KisSelectionMask::setSelection(KisSelectionSP selection)
{
    if (selection) {
        KisMask::setSelection(selection);
    } else {
        KisMask::setSelection(new KisSelection());

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->alpha8();
        KisFillPainter gc(KisPaintDeviceSP(this->selection()->getOrCreatePixelSelection().data()));
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

KoDocumentSectionModel::PropertyList KisSelectionMask::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisBaseNode::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("Active"), koIcon("local_selection_active"), koIcon("local_selection_inactive"), active());
    return l;
}

void KisSelectionMask::setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties)
{
    setVisible(properties.at(0).state.toBool());
    setUserLocked(properties.at(1).state.toBool());
    setActive(properties.at(2).state.toBool());
}

void KisSelectionMask::setVisible(bool visible)
{
    nodeProperties().setProperty("visible", visible);

    if (selection())
        selection()->setVisible(visible);
    emit(visibilityChanged(visible));
}

bool KisSelectionMask::active() const
{
    return nodeProperties().boolProperty("active", true);
}

void KisSelectionMask::setActive(bool active)
{
    KisImageWSP image = this->image();
    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());

    if (active && parentLayer) {
        KisSelectionMaskSP activeMask = parentLayer->selectionMask();
        if (activeMask) {
            activeMask->setActive(false);
        }
    }

    nodeProperties().setProperty("active", active);

    if (image) {
        image->nodeChanged(this);
        image->undoAdapter()->emitSelectionChanged();
    }
}

