/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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

#include "kis_mask_manager.h"


#include <kstandardaction.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>


#include <kis_transaction.h>
#include <filter/kis_filter_configuration.h>
#include <commands/kis_node_commands.h>
#include <kis_undo_adapter.h>
#include <kis_paint_layer.h>
#include "kis_doc2.h"
#include "kis_view2.h"
#include <kis_layer.h>
#include <kis_filter_mask.h>
#include <kis_transparency_mask.h>
#include <kis_selection_mask.h>
#include <kis_effect_mask.h>
#include "dialogs/kis_dlg_adjustment_layer.h"
#include "widgets/kis_mask_widgets.h"
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_pixel_selection.h>
#include "dialogs/kis_dlg_adj_layer_props.h"
#include <kis_image.h>
#include <kis_transform_worker.h>
#include <KoColorSpace.h>
#include <KoColor.h>
#include "kis_node_commands_adapter.h"
#include "commands/kis_selection_commands.h"


KisMaskManager::KisMaskManager(KisView2 * view)
        : m_view(view)
        , m_activeMask(0)
        , m_maskToSelection(0)
        , m_maskToLayer(0)
        , m_commandsAdapter(new KisNodeCommandsAdapter(m_view))
{
}

void KisMaskManager::setup(KActionCollection * actionCollection)
{
    m_maskToSelection  = new KAction(i18n("Mask To Selection"), this);
    actionCollection->addAction("create_selection_from_mask", m_maskToSelection);
    connect(m_maskToSelection, SIGNAL(triggered()), this, SLOT(maskToSelection()));

    m_maskToLayer = new KAction(i18n("Create Layer from Mask"), this);
    actionCollection->addAction("create_layer_from_mask", m_maskToLayer);
    connect(m_maskToLayer, SIGNAL(triggered()), this, SLOT(maskToLayer()));
}

void KisMaskManager::updateGUI()
{
    // XXX: enable/disable menu items according to whether there's a mask selected currently
    // XXX: disable the selection mask item if there's already a selection mask
}

KisMaskSP KisMaskManager::activeMask()
{
    return m_activeMask;
}

KisPaintDeviceSP KisMaskManager::activeDevice()
{
    // XXX: we may also need to have a possibility of getting the vector
    // part of selection here

    KisSelectionSP selection;
    return m_activeMask && (selection = m_activeMask->selection()) ?
           selection->getOrCreatePixelSelection() : 0;
}

void KisMaskManager::activateMask(KisMaskSP mask)
{
    m_activeMask = mask;
    emit sigMaskActivated(mask);
}

void KisMaskManager::masksUpdated()
{
    m_view->updateGUI();
}

void KisMaskManager::createTransparencyMask()
{
    KisLayerSP layer = m_view->activeLayer();

    if (layer) {
        KisNodeSP above = m_activeMask ?
                          static_cast<KisNode*>(m_activeMask.data()) : static_cast<KisNode*>(layer->firstChild().data());
        createTransparencyMask(layer, above);
    }
}

void KisMaskManager::createFilterMask()
{
    KisLayerSP layer = m_view->activeLayer();

    if (layer) {
        KisNodeSP above = m_activeMask ?
                          static_cast<KisNode*>(m_activeMask.data()) : static_cast<KisNode*>(layer->firstChild().data());
        createFilterMask(layer, above);
    }
}

void KisMaskManager::createSelectionMask(KisNodeSP parent, KisNodeSP above)
{
    KisLayer * layer = dynamic_cast<KisLayer*>(parent.data());
    if (layer && layer->selectionMask())
        layer->selectionMask()->setActive(false);

    KisSelectionMask *mask = new KisSelectionMask(m_view->image());
    mask->initSelection(m_view->selection(), layer);

    connect(mask,SIGNAL(changeActivity(KisSelectionMask*,bool)),this,SLOT(changeActivity(KisSelectionMask*,bool)));

    //counting number of KisSelectionMask
    QList<KisNodeSP> selectionMasks = layer->childNodes(QStringList("KisSelectionMask"),KoProperties());

    mask->setName(i18n("Selection ")+QString::number(selectionMasks.count()+1));
    m_commandsAdapter->addNode(mask, parent, above);

    activateMask(mask);
    masksUpdated();
}

void KisMaskManager::createTransparencyMask(KisNodeSP parent, KisNodeSP above)
{
    KisLayer *layer = dynamic_cast<KisLayer*>(parent.data());
    KisMask *mask = new KisTransparencyMask();
    mask->initSelection(m_view->selection(), layer);

    QList<KisNodeSP> transparencyMasks = layer->childNodes(QStringList("KisTransparencyMask"),KoProperties());
    mask->setName(i18n("Transparency Mask")+QString::number(transparencyMasks.count()+1));
    m_commandsAdapter->addNode(mask, parent, above);

    activateMask(mask);
    masksUpdated();
}

void KisMaskManager::createFilterMask(KisNodeSP parent, KisNodeSP above)
{
    KisLayer *layer = dynamic_cast<KisLayer*>(parent.data());
    KisFilterMask *mask = new KisFilterMask();
    mask->initSelection(m_view->selection(), layer);

    mask->setName(i18n("New filter mask"));
    m_commandsAdapter->addNode(mask, parent, above);

    /**
     * FIXME: We'll use layer's original for creation of a thumbnail.
     * Actually, we can't use it's projection as newly created mask
     * may be going to be inserted in the middle of the masks stack
     */
    KisPaintDeviceSP originalDevice = layer->original();


    KisDlgAdjustmentLayer dialog(mask, mask, originalDevice, m_view->image(),
                                 mask->name(), i18n("New Filter Mask"),
                                 m_view, "dlgfiltermask");

    if (dialog.exec() == QDialog::Accepted) {
        KisFilterConfiguration *filter = dialog.filterConfiguration();
        if (filter) {
            QString name = dialog.layerName();
            mask->setFilter(filter);
            mask->setName(name);
            activateMask(mask);
        }
    } else {
        m_commandsAdapter->undoLastCommand();
    }
    masksUpdated();
}

void KisMaskManager::maskToSelection()
{
    if (!m_activeMask) return;
    KisImageWSP image = m_view->image();
    if (!image) return;
    m_commandsAdapter->beginMacro(i18n("Mask to Selection"));
    KUndo2Command* cmd = new KisSetGlobalSelectionCommand(image, 0, m_activeMask->selection());
    image->undoAdapter()->addCommand(cmd);
    m_commandsAdapter->removeNode(m_activeMask);
    m_commandsAdapter->endMacro();

    activateMask(0);
    masksUpdated();
}

void KisMaskManager::maskToLayer()
{
    // XXX: Should we also be able to create other layertypes than paint layer from masks?
    // XXX: Right now, I create black pixels with the alpha channel set to the selection,
    //      should we create grayscale pixels?
    if (!m_activeMask) return;
    KisImageWSP image = m_view->image();
    if (!image) return;
    KisLayerSP activeLayer = m_view->activeLayer();
    if (!activeLayer) return;

    KisSelectionSP selection = m_activeMask->selection();
    selection->updateProjection();
    KisPaintLayerSP layer =
        new KisPaintLayer(image, m_activeMask->name(), OPACITY_OPAQUE_U8);

    const KoColorSpace * cs = layer->colorSpace();
    QRect rc = selection->selectedExactRect();
    KoColor color(Qt::black, cs);
    int pixelsize = cs->pixelSize();
    KisHLineIteratorPixel dstiter = layer->paintDevice()->createHLineIterator(rc.x(), rc.y(), rc.width(), selection);
    for (int row = 0; row < rc.height(); ++row) {
        while (!dstiter.isDone()) {
            cs->setOpacity(color.data(), dstiter.selectedness(), 1);
            memcpy(dstiter.rawData(), color.data(), pixelsize);
            ++dstiter;
        }
        dstiter.nextRow();
    }

    m_commandsAdapter->beginMacro(i18n("Layer from Mask"));
    m_commandsAdapter->removeNode(m_activeMask);
    m_commandsAdapter->addNode(layer, activeLayer->parent(), activeLayer);
    m_commandsAdapter->endMacro();

    activateMask(0);
    masksUpdated();
}

void KisMaskManager::duplicateMask()
{
    if (!m_activeMask) return;
    if (!m_view->image()) return;

    KisNodeSP newMask = m_activeMask->clone();
    KisSelectionMask *selMask=NULL;

    if (m_activeMask->inherits("KisSelectionMask")) {
        KisLayerSP l=m_view->activeLayer();
        if (l && l->selectionMask())
               l->selectionMask()->setActive(false);
        selMask=dynamic_cast<KisSelectionMask*>(newMask.data());
        connect(selMask,SIGNAL(changeActivity(KisSelectionMask*,bool)),this,SLOT(changeActivity(KisSelectionMask*,bool)));
    }
    newMask->setName(i18n("Duplication of ") + m_activeMask->name());
    m_commandsAdapter->addNode(newMask, m_activeMask->parent(), m_activeMask);
    activateMask(dynamic_cast<KisMask*>(newMask.data()));
    if (selMask)
        changeActivity(selMask,true);
    masksUpdated();
}

void KisMaskManager::removeMask()
{
    if (!m_activeMask) return;
    if (!m_view->image()) return;
    m_commandsAdapter->removeNode(m_activeMask);

    activateMask(0);
    masksUpdated();
}

void KisMaskManager::mirrorMaskX()
{
    // XXX_NODE: This is a load of copy-past from KisLayerManager -- how can I fix that?
    // XXX_NODE: we should also mirror the shape-based part of the selection!
    if (!m_activeMask) return;

    KisPaintDeviceSP dev = m_activeMask->selection()->getOrCreatePixelSelection();
    if (!dev) return;

    KisTransaction transaction(i18n("Mirror Mask X"), dev);

    QRect dirty = KisTransformWorker::mirrorX(dev, m_view->selection());
    m_activeMask->setDirty(dirty);

    transaction.commit(m_view->image()->undoAdapter());

    m_view->document()->setModified(true);
    m_activeMask->selection()->updateProjection();
    masksUpdated();
    m_view->canvas()->update();
}

void KisMaskManager::mirrorMaskY()
{
    // XXX_NODE: This is a load of copy-past from Kisupanager -- how can I fix that?
    // XXX_NODE: we should also mirror the shape-based part of the selection!
    if (!m_activeMask) return;

    KisPaintDeviceSP dev = m_activeMask->selection()->getOrCreatePixelSelection();
    if (!dev) return;

    KisTransaction transaction(i18n("Mirror Layer Y"), dev);

    QRect dirty = KisTransformWorker::mirrorY(dev, m_view->selection());
    m_activeMask->setDirty(dirty);

    transaction.commit(m_view->image()->undoAdapter());

    m_view->document()->setModified(true);
    m_activeMask->selection()->updateProjection();
    masksUpdated();
    m_view->canvas()->update();
}

void KisMaskManager::maskProperties()
{
    if (!m_activeMask) return;

    if (m_activeMask->inherits("KisFilterMask")) {
        KisFilterMask * mask = static_cast<KisFilterMask*>(m_activeMask.data());

        KisLayerSP layer = dynamic_cast<KisLayer*>(mask->parent().data());
        if (! layer)
            return;

        KisPaintDeviceSP dev = layer->paintDevice();
        KisDlgAdjLayerProps dlg(dev, layer->image(), mask->filter(), mask->name(), i18n("Effect Mask Properties"), m_view, "dlgeffectmaskprops");
        QString before;
        if (dlg.filterConfiguration())
            before = dlg.filterConfiguration()->toXML();
        if (dlg.exec() == QDialog::Accepted) {
            QString after;
            if (dlg.filterConfiguration())
                after = dlg.filterConfiguration()->toXML();
            KisChangeFilterCmd<KisFilterMaskSP> * cmd = new KisChangeFilterCmd<KisFilterMaskSP>(mask,
                    dlg.filterConfiguration(),
                    before,
                    after);

            // FIXME: Check why don't we use m_commandsAdapter instead
            cmd->redo();
            m_view->undoAdapter()->addCommand(cmd);
            m_view->document()->setModified(true);
            mask->setDirty();
        }
    } else {
        // Not much to show for transparency or selection masks?
    }
}

void KisMaskManager::raiseMask()
{
    if (!m_activeMask) return;
    if (!m_view->image()) return;
    m_commandsAdapter->raise(m_activeMask);
}

void KisMaskManager::lowerMask()
{
    if (!m_activeMask) return;
    if (!m_view->image()) return;
    m_commandsAdapter->lower(m_activeMask);
}

void KisMaskManager::maskToTop()
{
    if (!m_activeMask) return;
    if (!m_view->image()) return;
    m_commandsAdapter->toTop(m_activeMask);
}

void KisMaskManager::maskToBottom()
{
    if (!m_activeMask) return;
    if (!m_view->image()) return;
    m_commandsAdapter->toBottom(m_activeMask);
}

void KisMaskManager::changeActivity(KisSelectionMask *mask,bool active)
{
    KisLayer * layer = dynamic_cast<KisLayer*>(mask->parent().data());

    if (active && layer && layer->selectionMask()) {
        KisSelectionMask* selectionMask = layer->selectionMask().data();
        layer->selectionMask()->nodeProperties().setProperty("active", false);
        if (m_view->image()) {
            m_view->image()->nodeChanged(selectionMask);
        }
    }

    mask->nodeProperties().setProperty("active", active);
    m_view->selectionManager()->selectionChanged();
}


#include "kis_mask_manager.moc"
