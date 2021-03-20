/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "layersplit.h"

#include <QMap>
#include <QPointer>
#include <QHash>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColor.h>

#include <kis_debug.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_action.h>
#include <KisDocument.h>
#include <kis_node.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_random_accessor_ng.h>
#include "dlg_layersplit.h"
#include "kis_node_manager.h"
#include "kis_node_commands_adapter.h"
#include "kis_undo_adapter.h"
#include <kis_image_barrier_locker.h>
#include "kis_selection_mask.h"

#include <KoUpdater.h>
#include <KoProgressUpdater.h>

K_PLUGIN_FACTORY_WITH_JSON(LayerSplitFactory, "kritalayersplit.json", registerPlugin<LayerSplit>();)

LayerSplit::LayerSplit(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action  = createAction("layersplit");
    connect(action, SIGNAL(triggered()), this, SLOT(slotLayerSplit()));
}

LayerSplit::~LayerSplit()
{
}


struct Layer {
    KoColor color;
    KisPaintDeviceSP device;
    KisRandomAccessorSP accessor;
    int pixelsWritten;

    bool operator<(const Layer& other) const
    {
        return pixelsWritten < other.pixelsWritten;
    }
};

void LayerSplit::slotLayerSplit()
{
    DlgLayerSplit dlg;

    if (dlg.exec() == QDialog::Accepted) {

        bool modeToLayer = !dlg.m_modeToMask;
        dlg.hide();

        QApplication::setOverrideCursor(Qt::WaitCursor);

        QPointer<KoUpdater> updater;
        if( modeToLayer){
            updater = viewManager()->createUnthreadedUpdater(i18n("Split into Layers"));
        }
        else {
            updater = viewManager()->createUnthreadedUpdater(i18n("Split into Masks"));
        }
        KisImageSP image = viewManager()->image();
        if (!image) return;

        KisImageBarrierLocker locker(image);

        KisNodeSP node = viewManager()->activeNode();
        if (!node) return;

        KisPaintDeviceSP projection = node->projection();
        if (!projection) return;

        QList<Layer> colorMap;

        const KoColorSpace *cs = projection->colorSpace();
        QRect rc = image->bounds();

        int fuzziness = dlg.fuzziness();

        updater->setProgress(0);

        KisRandomConstAccessorSP acc = projection->createRandomConstAccessorNG();

        for (int row = rc.y(); row < rc.height(); ++row) {

            for (int col = rc.x(); col < rc.width(); ++col) {

                acc->moveTo(col, row);

                KoColor c(cs);
                c.setColor(acc->rawDataConst(), cs);

                if (c.opacityU8() == OPACITY_TRANSPARENT_U8) {
                    continue;
                }

                if (dlg.disregardOpacity()) {
                    c.setOpacity(OPACITY_OPAQUE_U8);
                }

                bool found = false;
                Q_FOREACH (const Layer &l, colorMap) {
                    if (fuzziness == 0) {

                        found = (l.color == c);
                    }
                    else {
                        quint8 match = cs->difference(l.color.data(), c.data());
                        found = (match <= fuzziness);
                    }
                    if (found) {
                        KisRandomAccessorSP dstAcc = l.accessor;
                        dstAcc->moveTo(col, row);
                        memcpy(dstAcc->rawData(), acc->rawDataConst(), cs->pixelSize());
                        const_cast<Layer*>(&l)->pixelsWritten++;
                        break;
                    }
                }

                if (!found) {
                    QString name = "";
                    if (dlg.palette()) {
                       name = dlg.palette()->getClosestColorInfo(c).swatch.name();
                    }

                    if (name.toLower() == "untitled" || name.toLower() == "none" || name.toLower() == "") {
                        name = KoColor::toQString(c);
                    }
                    Layer l;
                    l.color = c;
                    l.device = new KisPaintDevice(cs, name);
                    l.accessor = l.device->createRandomAccessorNG();
                    l.accessor->moveTo(col, row);
                    memcpy(l.accessor->rawData(), acc->rawDataConst(), cs->pixelSize());
                    l.pixelsWritten = 1;
                    colorMap << l;
                }
            }

            if (updater->interrupted()) {
                return;
            }

            updater->setProgress((row - rc.y()) * 100 / rc.height() - rc.y());
        }

        updater->setProgress(100);

        dbgKrita << "Created" << colorMap.size() << "layers";
//        Q_FOREACH (const Layer &l, colorMap) {
//            dbgKrita << "\t" << l.device->objectName() << ":" << l.pixelsWritten;
//        }

        if (dlg.sortLayers()) {
            std::sort(colorMap.begin(), colorMap.end());
        }

        KisUndoAdapter *undo = image->undoAdapter();
        undo->beginMacro(kundo2_i18n("Split Layer"));
        KisNodeCommandsAdapter adapter(viewManager());

        if(modeToLayer){
            KisGroupLayerSP baseGroup = dynamic_cast<KisGroupLayer*>(node->parent().data());
            if (!baseGroup) {
                // Masks are never nested
                baseGroup = dynamic_cast<KisGroupLayer*>(node->parent()->parent().data());
            }

            if (dlg.hideOriginal()) {
                node->setVisible(false);
            }

            if (dlg.createBaseGroup()) {
                KisGroupLayerSP grp = new KisGroupLayer(image, i18n("Color"), OPACITY_OPAQUE_U8);
                adapter.addNode(grp, baseGroup, 1);
                baseGroup = grp;
            }

            Q_FOREACH (const Layer &l, colorMap) {
                KisGroupLayerSP grp = baseGroup;
                if (dlg.createSeparateGroups()) {
                    grp = new KisGroupLayer(image, l.device->objectName(), OPACITY_OPAQUE_U8);
                    adapter.addNode(grp, baseGroup, 1);
                }
                KisPaintLayerSP paintLayer = new KisPaintLayer(image, l.device->objectName(), OPACITY_OPAQUE_U8, l.device);
                adapter.addNode(paintLayer, grp, 0);
                paintLayer->setAlphaLocked(dlg.lockAlpha());
            }
        }
        else{
            KisLayerSP baseGroup = dynamic_cast<KisLayer*>(node.data());
            Q_FOREACH (const Layer &l, colorMap) {
                KisSelectionMaskSP mask = new KisSelectionMask(image);
                mask->setName( l.device->objectName());

                KisPaintDeviceSP temp = KisPainter::convertToAlphaAsPureAlpha(l.device);
                mask->initSelection(temp , baseGroup);
                adapter.addNode(mask, baseGroup,0);
                mask->setActive(true);
            }
        }

        undo->endMacro();
   }

    QApplication::restoreOverrideCursor();
}

#include "layersplit.moc"
