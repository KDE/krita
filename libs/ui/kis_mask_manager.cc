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

#include <kactioncollection.h>

#include <KoProperties.h>

#include <kis_transaction.h>
#include <filter/kis_filter_configuration.h>
#include <commands/kis_node_commands.h>
#include <kis_undo_adapter.h>
#include <kis_paint_layer.h>
#include "KisDocument.h"
#include "KisViewManager.h"
#include <kis_layer.h>
#include <kis_clone_layer.h>
#include <kis_group_layer.h>
#include <kis_filter_mask.h>
#include <lazybrush/kis_colorize_mask.h>
#include <kis_transform_mask.h>
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
#include "kis_iterator_ng.h"

KisMaskManager::KisMaskManager(KisViewManager * view)
    : m_view(view)
    , m_imageView(0)
    , m_commandsAdapter(new KisNodeCommandsAdapter(m_view))
{
}

void KisMaskManager::setView(QPointer<KisView>imageView)
{
    m_imageView = imageView;
}

void KisMaskManager::setup(KActionCollection *actionCollection, KisActionManager *actionManager)
{
    Q_UNUSED(actionCollection);
    Q_UNUSED(actionManager);
}

void KisMaskManager::updateGUI()
{
    // XXX: enable/disable menu items according to whether there's a mask selected currently
    // XXX: disable the selection mask item if there's already a selection mask
    // YYY: doesn't KisAction do that already?
}

KisMaskSP KisMaskManager::activeMask()
{
    if (m_imageView) {
        return m_imageView->currentMask();
    }
    return 0;
}

KisPaintDeviceSP KisMaskManager::activeDevice()
{
    KisMaskSP mask = activeMask();
    return mask ? mask->paintDevice() : 0;
}

void KisMaskManager::activateMask(KisMaskSP mask)
{
    Q_UNUSED(mask);
}

void KisMaskManager::masksUpdated()
{
    m_view->updateGUI();
}

void KisMaskManager::adjustMaskPosition(KisNodeSP node, KisNodeSP activeNode, bool avoidActiveNode, KisNodeSP &parent, KisNodeSP &above)
{
    Q_ASSERT(node);
    Q_ASSERT(activeNode);

    if (!avoidActiveNode && activeNode->allowAsChild(node)) {
        parent = activeNode;
        above = activeNode->lastChild();
    } else if (activeNode->parent() && activeNode->parent()->allowAsChild(node)) {
        parent = activeNode->parent();
        above = activeNode;
    } else {
        KisNodeSP t = activeNode;
        while ((t = t->nextSibling())) {
            if (t->allowAsChild(node)) {
                parent = t;
                above = t->lastChild();
                break;
            }
        }

        if (!t) {
            t = activeNode;
            while ((t = t->prevSibling())) {
                if (t->allowAsChild(node)) {
                    parent = t;
                    above = t->lastChild();
                    break;
                }
            }
        }

        if (!t && activeNode->parent()) {
            adjustMaskPosition(node, activeNode->parent(), true, parent, above);
        } else if (!t) {
            KisImageWSP image = m_view->image();
            KisLayerSP layer = new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, image->colorSpace());
            m_commandsAdapter->addNode(layer, activeNode, 0);

            parent = layer;
            above = 0;
        }
    }
}

void KisMaskManager::createMaskCommon(KisMaskSP mask, KisNodeSP activeNode, KisPaintDeviceSP copyFrom, const KUndo2MagicString& macroName, const QString &nodeType, const QString &nodeName, bool suppressSelection, bool avoidActiveNode, bool updateImage)
{
    m_commandsAdapter->beginMacro(macroName);

    KisNodeSP parent;
    KisNodeSP above;
    adjustMaskPosition(mask, activeNode, avoidActiveNode, parent, above);

    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent.data());
    Q_ASSERT(parentLayer);

    if (!suppressSelection) {
        if (copyFrom) {
            mask->initSelection(copyFrom, parentLayer);
        } else {
            mask->initSelection(m_view->selection(), parentLayer);
        }
    }

    //counting number of KisSelectionMask
    QList<KisNodeSP> masks = parentLayer->childNodes(QStringList(nodeType),KoProperties());
    int number = masks.count() + 1;
    mask->setName(nodeName + QString(" ") + QString::number(number));

    m_commandsAdapter->addNode(mask, parentLayer, above, updateImage, updateImage);
    m_commandsAdapter->endMacro();

    masksUpdated();
}

KisNodeSP KisMaskManager::createSelectionMask(KisNodeSP activeNode, KisPaintDeviceSP copyFrom, bool convertActiveNode)
{
    KisSelectionMaskSP mask = new KisSelectionMask(m_view->image());
    createMaskCommon(mask, activeNode, copyFrom, kundo2_i18n("Add Selection Mask"), "KisSelectionMask", i18n("Selection"), false, convertActiveNode, false);
    mask->setActive(true);
    if (convertActiveNode) {
        m_commandsAdapter->removeNode(activeNode);
    }
    return mask;
}

KisNodeSP KisMaskManager::createTransparencyMask(KisNodeSP activeNode, KisPaintDeviceSP copyFrom, bool convertActiveNode)
{
    KisMaskSP mask = new KisTransparencyMask();
    createMaskCommon(mask, activeNode, copyFrom, kundo2_i18n("Add Transparency Mask"), "KisTransparencyMask", i18n("Transparency Mask"), false, convertActiveNode);
    if (convertActiveNode) {
        m_commandsAdapter->removeNode(activeNode);
    }
    return mask;
}


KisNodeSP KisMaskManager::createFilterMask(KisNodeSP activeNode, KisPaintDeviceSP copyFrom, bool quiet, bool convertActiveNode)
{
    KisFilterMaskSP mask = new KisFilterMask();
    createMaskCommon(mask, activeNode, copyFrom, kundo2_i18n("Add Filter Mask"), "KisFilterMask", i18n("Filter Mask"), false, convertActiveNode);

    if (convertActiveNode) {
        m_commandsAdapter->removeNode(activeNode);
    }

    /**
     * FIXME: We'll use layer's original for creation of a thumbnail.
     * Actually, we can't use it's projection as newly created mask
     * may be going to be inserted in the middle of the masks stack
     */
    KisPaintDeviceSP originalDevice = mask->parent()->original();


    KisDlgAdjustmentLayer dialog(mask, mask.data(), originalDevice,
                                 mask->name(), i18n("New Filter Mask"),
                                 m_view);

    // If we are supposed to not disturb the user, don't start asking them about things.
    if(quiet) {
        KisFilterConfigurationSP filter = KisFilterRegistry::instance()->values().first()->defaultConfiguration();
        if (filter) {
            mask->setFilter(filter);
            mask->setName(mask->name());
        }
        return mask;
    }

    if (dialog.exec() == QDialog::Accepted) {
        KisFilterConfigurationSP filter = dialog.filterConfiguration();
        if (filter) {
            QString name = dialog.layerName();
            mask->setFilter(filter);
            mask->setName(name);
        }

        return mask;

    } else {
        m_commandsAdapter->undoLastCommand();
    }

    return 0;
}


KisNodeSP KisMaskManager::createColorizeMask(KisNodeSP activeNode)
{
    KisColorizeMaskSP mask = new KisColorizeMask();
    createMaskCommon(mask, activeNode, 0, kundo2_i18n("Add Colorize Mask"), "KisColorizeMask", i18n("Colorize Mask"), true, false);
    mask->setImage(m_view->image());
    mask->initializeCompositeOp();
    delete mask->setColorSpace(mask->parent()->colorSpace());
    return mask;
}


KisNodeSP KisMaskManager::createTransformMask(KisNodeSP activeNode)
{
    KisTransformMaskSP mask = new KisTransformMask();
    createMaskCommon(mask, activeNode, 0, kundo2_i18n("Add Transform Mask"), "KisTransformMask", i18n("Transform Mask"), true, false);
    return mask;
}

void KisMaskManager::maskProperties()
{
    if (!activeMask()) return;

    if (activeMask()->inherits("KisFilterMask")) {
        KisFilterMask *mask = static_cast<KisFilterMask*>(activeMask().data());

        KisLayerSP layer = qobject_cast<KisLayer*>(mask->parent().data());
        if (! layer)
            return;


        KisPaintDeviceSP dev = layer->original();
        if (!dev) {
            return;
        }

        KisDlgAdjLayerProps dlg(layer, mask, dev, m_view, mask->filter().data(), mask->name(), i18n("Filter Mask Properties"), m_view->mainWindow(), "dlgeffectmaskprops");

        KisFilterConfigurationSP configBefore(mask->filter());
        Q_ASSERT(configBefore);
        QString xmlBefore = configBefore->toXML();

        if (dlg.exec() == QDialog::Accepted) {

            KisFilterConfigurationSP configAfter(dlg.filterConfiguration());
            Q_ASSERT(configAfter);
            QString xmlAfter = configAfter->toXML();

            mask->setName(dlg.layerName());

            if(xmlBefore != xmlAfter) {
                KisChangeFilterCmd *cmd
                    = new KisChangeFilterCmd(mask,
                                             configBefore->name(),
                                             xmlBefore,
                                             configAfter->name(),
                                             xmlAfter,
                                             false);

                // FIXME: check whether is needed
                cmd->redo();
                m_view->undoAdapter()->addCommand(cmd);
                m_view->document()->setModified(true);
            }
        }
        else {
            KisFilterConfigurationSP configAfter(dlg.filterConfiguration());
            Q_ASSERT(configAfter);
            QString xmlAfter = configAfter->toXML();

            if(xmlBefore != xmlAfter) {
                mask->setFilter(KisFilterRegistry::instance()->cloneConfiguration(configBefore.data()));
                mask->setDirty();
            }
        }

    } else {
        // Not much to show for transparency or selection masks?
    }
}
