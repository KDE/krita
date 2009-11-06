/*
 *  Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_layer_manager.h"
#include <QRect>
#include <QApplication>
#include <QCursor>
#include <QString>
#include <QDialog>

#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kurl.h>

#include <KoFilterManager.h>
#include <KoDocument.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoPointerEvent.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoProgressUpdater.h>

#include <filter/kis_filter_configuration.h>
#include <filter/kis_filter.h>
#include <kis_filter_strategy.h>
#include <generator/kis_generator_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_clone_layer.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_selected_transaction.h>
#include <kis_selection.h>
#include <flake/kis_shape_layer.h>
#include <kis_shear_visitor.h>
#include <kis_transform_worker.h>
#include <kis_undo_adapter.h>
#include <kis_painter.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "dialogs/kis_dlg_adj_layer_props.h"
#include "dialogs/kis_dlg_adjustment_layer.h"
#include "dialogs/kis_dlg_layer_properties.h"
#include "dialogs/kis_dlg_generator_layer.h"
#include "kis_doc2.h"
#include "kis_filter_manager.h"
#include "commands/kis_image_commands.h"
#include "commands/kis_layer_commands.h"
#include "commands/kis_node_commands.h"
#include "kis_canvas_resource_provider.h"
#include "kis_selection_manager.h"
#include "kis_statusbar.h"
#include "kis_view2.h"
#include "kis_zoom_manager.h"
#include "canvas/kis_canvas2.h"
#include "widgets/kis_meta_data_merge_strategy_chooser_widget.h"
#include "widgets/kis_wdg_generator.h"
#include "kis_progress_widget.h"
#include "kis_node_commands_adapter.h"
#include "kis_node_manager.h"

KisLayerManager::KisLayerManager(KisView2 * view, KisDoc2 * doc)
        : m_view(view)
        , m_doc(doc)
        , m_imgFlatten(0)
        , m_imgMergeLayer(0)
        , m_layerSaveAs(0)
        , m_actLayerVis(false)
        , m_imgResizeToLayer(0)
        , m_flattenLayer(0)
        , m_activeLayer(0)
        , m_commandsAdapter(new KisNodeCommandsAdapter(m_view))
{
}

KisLayerManager::~KisLayerManager()
{
    delete m_commandsAdapter;
}

KisLayerSP KisLayerManager::activeLayer()
{
    return m_activeLayer;
}

KisPaintDeviceSP KisLayerManager::activeDevice()
{
    if (m_activeLayer)
        return m_activeLayer->paintDevice();
    else
        return 0;
}

void KisLayerManager::activateLayer(KisLayerSP layer)
{
    m_activeLayer = layer;
    emit sigLayerActivated(layer);
    layersUpdated();
    if (layer) {
        m_view->resourceProvider()->slotNodeActivated(layer.data());
    }
}


void KisLayerManager::setup(KActionCollection * actionCollection)
{
    m_imgFlatten  = new KAction(i18n("&Flatten image"), this);
    actionCollection->addAction("flatten_image", m_imgFlatten);
    m_imgFlatten->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E));
    connect(m_imgFlatten, SIGNAL(triggered()), this, SLOT(flattenImage()));

    m_imgMergeLayer  = new KAction(i18n("&Merge with Layer Below"), this);
    actionCollection->addAction("merge_layer", m_imgMergeLayer);
    m_imgMergeLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(m_imgMergeLayer, SIGNAL(triggered()), this, SLOT(mergeLayer()));

    m_flattenLayer  = new KAction(i18n("&Flatten Layer"), this);
    actionCollection->addAction("flatten_layer", m_flattenLayer);
    connect(m_flattenLayer, SIGNAL(triggered()), this, SLOT(flattenLayer()));

    m_layerSaveAs  = new KAction(KIcon("document-save"), i18n("Save Layer as Image..."), this);
    actionCollection->addAction("save_layer_as_image", m_layerSaveAs);
    connect(m_layerSaveAs, SIGNAL(triggered()), this, SLOT(saveLayerAsImage()));

    m_imgResizeToLayer  = new KAction(i18n("Size Canvas to Size of Current Layer"), this);
    actionCollection->addAction("resizeimgtolayer", m_imgResizeToLayer);
    connect(m_imgResizeToLayer, SIGNAL(triggered()), this, SLOT(imgResizeToActiveLayer()));
}

void KisLayerManager::addAction(QAction * action)
{
    m_pluginActions.append(action);
}

void KisLayerManager::updateGUI()
{
    KisImageWSP img = m_view->image();

    KisLayerSP layer;
    qint32 nlayers = 0;
    qint32 nvisible = 0;


    if (img) {
        layer = m_activeLayer;
        nlayers = img->nlayers();
        nvisible = nlayers - img->nHiddenLayers();
    }

    bool enable = img && layer && layer->visible() && !layer->userLocked() && !layer->systemLocked();

    m_layerSaveAs->setEnabled(enable);

    // XXX these should be named layer instead of img
    m_imgFlatten->setEnabled(nlayers > 1);
    m_imgMergeLayer->setEnabled(nlayers > 1 && layer && layer->prevSibling());
    m_flattenLayer->setEnabled(nlayers > 1 && layer && layer->firstChild());

    m_imgResizeToLayer->setEnabled(activeLayer());

    if (m_view->statusBar())
        m_view->statusBar()->setProfile(img);
}

void KisLayerManager::imgResizeToActiveLayer()
{
    KisLayerSP layer;
    KisUndoAdapter * undoAdapter = m_view->undoAdapter();

    KisImageWSP img = m_view->image();

    if (img && (layer = activeLayer())) {

        if (undoAdapter && undoAdapter->undo()) {
            undoAdapter->beginMacro(i18n("Resize Image to Size of Current Layer"));
        }

        img->lock();

        QRect r = layer->exactBounds();
        img->resize(r.width(), r.height(), r.x(), r.y(), true);

        img->unlock();

        if (undoAdapter && undoAdapter->undo()) {
            undoAdapter->endMacro();
        }
    }
}

void KisLayerManager::actLayerVisChanged(int show)
{
    m_actLayerVis = (show != 0);
}

void KisLayerManager::layerProperties()
{
    KisLayerSP layer = m_activeLayer;

    if (!layer) return;

    const KoColorSpace * cs = 0;
    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if (pl) {
        cs = pl->paintDevice()->colorSpace();
    } else {
        cs = layer->image()->colorSpace();
    }


    if (KisAdjustmentLayerSP alayer = KisAdjustmentLayerSP(dynamic_cast<KisAdjustmentLayer*>(layer.data()))) {
        KisPaintDeviceSP dev = alayer->projection();
        KisLayerSP prev = dynamic_cast<KisLayer*>(alayer->prevSibling().data());
        if (prev) dev = prev->projection();

        KisDlgAdjLayerProps dlg(dev, alayer->image(), alayer->filter(), alayer->name(), i18n("Filter Layer Properties"), m_view, "dlgadjlayerprops");
        dlg.resize(dlg.minimumSizeHint());
        KisFilterConfiguration* config = dlg.filterConfiguration();
        QString before;
        if (config) {
            before = config->toLegacyXML();
        }
        if (dlg.exec() == QDialog::Accepted) {

            QString after;
            if (dlg.filterConfiguration()) {
                after = dlg.filterConfiguration()->toLegacyXML();
            }
            if (after != before) {
                alayer->setName(dlg.layerName());

                KisChangeFilterCmd<KisAdjustmentLayerSP> * cmd
                = new KisChangeFilterCmd<KisAdjustmentLayerSP>(alayer,
                        dlg.filterConfiguration(),
                        before,
                        after);
                cmd->redo();
                m_view->undoAdapter()->addCommand(cmd);
                m_doc->setModified(true);
            }
        }
    } else if (KisGeneratorLayerSP alayer = KisGeneratorLayerSP(dynamic_cast<KisGeneratorLayer*>(layer.data()))) {

        KisDlgGeneratorLayer dlg(alayer->name(), m_view);
        dlg.setCaption(i18n("Generator Layer Properties"));

        QString before = alayer->generator()->toLegacyXML();
        dlg.setConfiguration(alayer->generator());
        dlg.resize(dlg.minimumSizeHint());

        if (dlg.exec() == QDialog::Accepted) {

            QString after = dlg.configuration()->toLegacyXML();
            if (after != before) {
                KisChangeGeneratorCmd<KisGeneratorLayerSP> * cmd
                = new KisChangeGeneratorCmd<KisGeneratorLayerSP>(alayer,
                        dlg.configuration(),
                        before,
                        after
                                                                );

                cmd->redo();
                m_view->undoAdapter()->addCommand(cmd);
                m_doc->setModified(true);
            }

        }
    } else {
        KisDlgLayerProperties dlg(layer->name(),
                                  layer->opacity(),
                                  layer->compositeOp(),
                                  cs,
                                  layer->channelFlags());
        dlg.resize(dlg.minimumSizeHint());

        if (dlg.exec() == QDialog::Accepted) {

            QBitArray newChannelFlags = dlg.getChannelFlags();
            for (int i = 0; i < newChannelFlags.size(); ++i) {
                dbgUI << "we got flags: " << i << " is " << newChannelFlags.testBit(i);
            }
            QBitArray oldChannelFlags = layer->channelFlags();
            for (int i = 0; i < oldChannelFlags.size(); ++i) {
                dbgUI << "the old ones were: " << i << " is " << oldChannelFlags.testBit(i);
            }

            dbgUI << " and are they the same: " << (oldChannelFlags == newChannelFlags);

            if (layer->name() != dlg.getName() ||
                    layer->opacity() != dlg.getOpacity() ||
                    layer->compositeOp()->id() != dlg.getCompositeOp() ||
                    oldChannelFlags != newChannelFlags
               ) {
                QApplication::setOverrideCursor(KisCursor::waitCursor());
                m_view->undoAdapter()->addCommand(new KisLayerPropsCommand(layer,
                                                  layer->opacity(), dlg.getOpacity(),
                                                  layer->compositeOpId(), dlg.getCompositeOp(),
                                                  layer->name(), dlg.getName(),
                                                  oldChannelFlags, newChannelFlags));
                QApplication::restoreOverrideCursor();
                m_doc->setModified(true);
            }
        }
    }

}

void KisLayerManager::layerAdd()
{
    KisImageWSP img = m_view->image();
    if (img && activeLayer()) {
        addLayer(activeLayer()->parent(), activeLayer());
    } else if (img)
        addLayer(img->rootLayer(), KisLayerSP(0));
}

void KisLayerManager::addLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP img = m_view->image();
    if (img) {
        KisConfig cfg;
        QString profilename;
        KisLayerSP layer = new KisPaintLayer(img.data(), img->nextLayerName(), OPACITY_OPAQUE, img->colorSpace());
        if (layer) {
            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent.data(), above.data());
            m_view->canvas()->update();
            m_view->nodeManager()->activateNode(layer);
        } else {
            KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
        }
    }
}

void KisLayerManager::addGroupLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP img = m_view->image();
    if (img) {
        KisLayerSP layer = KisLayerSP(new KisGroupLayer(img.data(), img->nextLayerName(), OPACITY_OPAQUE));
        if (layer) {
            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent.data(), above.data());
            m_view->canvas()->update();
        } else {
            KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
        }
    }
}


void KisLayerManager::addCloneLayer()
{
    KisImageWSP img = m_view->image();
    if (img && activeLayer()) {
        addCloneLayer(activeLayer()->parent(), activeLayer());
    } else if (img)
        addCloneLayer(img->rootLayer(), KisLayerSP(0));
}

void KisLayerManager::addCloneLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP img = m_view->image();
    if (img) {
        // Check whether we are not cloning a parent layer
        if (KisGroupLayer * from = dynamic_cast<KisGroupLayer*>(m_activeLayer.data())) {
            KisNodeSP parent = parent;
            while (parent && parent != img->root()) {
                if (parent.data() == from) {
                    // The chosen layer is one of our own parents -- this will
                    // lead to cyclic behaviour when updating. Don't do that!
                    return;
                }
                parent = parent->parent();
            }
        }

        KisLayerSP layer = new KisCloneLayer(m_activeLayer, img.data(), img->nextLayerName(), OPACITY_OPAQUE);

        if (layer) {

            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent.data(), above.data());

            m_view->canvas()->update();

        } else {
            KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
        }
    }
}


void KisLayerManager::addShapeLayer()
{
    KisImageWSP img = m_view->image();
    if (img && activeLayer()) {
        addShapeLayer(activeLayer()->parent(), activeLayer());
    } else if (img)
        addShapeLayer(img->rootLayer(), KisLayerSP(0));
}


void KisLayerManager::addShapeLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP img = m_view->image();
    if (img) {
        // XXX: Make work with nodes!
        KisLayer * parentLayer = dynamic_cast<KisLayer*>(parent.data());
        KoShapeContainer * parentContainer =
            dynamic_cast<KoShapeContainer*>(m_doc->shapeForNode(parentLayer));
        if (!parentContainer) return;

        KisLayerSP layer = new KisShapeLayer(parentContainer, m_doc->shapeController(), img.data(), img->nextLayerName(), OPACITY_OPAQUE);
        if (layer) {
            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent, above.data());
            m_view->canvas()->update();
        } else {
            KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
        }
    }
}


void KisLayerManager::addAdjustmentLayer()
{
    addAdjustmentLayer(activeLayer()->parent(), activeLayer());
}

void KisLayerManager::addAdjustmentLayer(KisNodeSP parent, KisNodeSP above)
{
    Q_ASSERT(parent);

    KisImageWSP img = m_view->image();
    if (!img) return;

    KisLayerSP l = activeLayer();

    KisPaintDeviceSP dev = l->projection();

    KisSelectionSP selection;
    if (l->selection())
        selection = l->selection();
    else
        selection = img->globalSelection();

    KisAdjustmentLayerSP adjl = addAdjustmentLayer(parent, above, QString(), 0, selection);

    KisDlgAdjustmentLayer dlg(adjl, adjl.data(), dev, adjl->image(), img->nextLayerName(), i18n("New Filter Layer"), m_view, "dlgadjustmentlayer");
    dlg.resize(dlg.minimumSizeHint());

    if (dlg.exec() != QDialog::Accepted) {
        m_view->image()->removeNode(adjl);
    } else {
        adjl->setName(dlg.layerName());
    }
}

KisAdjustmentLayerSP KisLayerManager::addAdjustmentLayer(KisNodeSP parent, KisNodeSP above, const QString & name,
        KisFilterConfiguration * filter, KisSelectionSP selection)
{
    Q_ASSERT(parent);

    KisImageWSP img = m_view->image();
    if (!img) return 0;

    KisAdjustmentLayerSP l = new KisAdjustmentLayer(img, name, filter, selection);
    m_commandsAdapter->addNode(l.data(), parent, above);
    l->setDirty(img->bounds());
    return l;
}

void KisLayerManager::addGeneratorLayer()
{
    addGeneratorLayer(activeLayer()->parent(), activeLayer());
}

void KisLayerManager::addGeneratorLayer(KisNodeSP parent, KisNodeSP above)
{
    Q_ASSERT(parent);

    KisImageWSP img = m_view->image();
    if (!img) return;

    KisDlgGeneratorLayer dlg(img->nextLayerName(), m_view);
    dlg.resize(dlg.minimumSizeHint());

    if (dlg.exec() == QDialog::Accepted) {
        KisSelectionSP selection = m_view->selection();
        KisFilterConfiguration * generator = dlg.configuration();
        QString name = dlg.layerName();
        addGeneratorLayer(parent, above, name, generator, selection);
    }

}

void KisLayerManager::addGeneratorLayer(KisNodeSP parent, KisNodeSP above, const QString & name, KisFilterConfiguration * generator, KisSelectionSP selection)
{
    Q_ASSERT(parent);
    Q_ASSERT(generator);

    KisImageWSP img = m_view->image();
    if (!img) return;

    KisGeneratorLayerSP l = new KisGeneratorLayer(img, name, generator, selection);
    m_commandsAdapter->addNode(l.data(), parent, above.data());
    if (l->selection())
        l->setDirty(l->selection()->selectedExactRect());
    else
        l->setDirty(img->bounds());

}


void KisLayerManager::layerRemove()
{
    KisImageWSP img = m_view->image();

    if (img) {
        KisLayerSP layer = activeLayer();
        if (layer) {
            QRect extent = layer->extent();
            KisNodeSP parent = layer->parent();

            m_commandsAdapter->removeNode(layer);

            if (parent)
                parent->setDirty(extent);

            m_view->canvas()->update();
            m_view->updateGUI();
        }
    }
}

void KisLayerManager::layerDuplicate()
{
    KisImageWSP img = m_view->image();

    if (!img)
        return;

    KisLayerSP active = activeLayer();

    if (!active)
        return;

    KisLayerSP dup = dynamic_cast<KisLayer*>(active->clone().data());
    dup->setName(i18n("Duplicate of '%1'", active->name()));
    m_commandsAdapter->addNode(dup.data(), active->parent(), active.data());
    if (dup) {
        activateLayer(dup);
        dup->setDirty();
        m_view->canvas()->update();
    } else {
        KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
    }
}

void KisLayerManager::layerRaise()
{
    KisImageWSP img = m_view->image();
    KisLayerSP layer;

    if (!img)
        return;

    layer = activeLayer();

    m_commandsAdapter->raise(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::layerLower()
{
    KisImageWSP img = m_view->image();
    KisLayerSP layer;

    if (!img)
        return;

    layer = activeLayer();

    m_commandsAdapter->lower(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::layerFront()
{
    KisImageWSP img = m_view->image();
    KisLayerSP layer;

    if (!img)
        return;

    layer = activeLayer();
    m_commandsAdapter->toTop(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::layerBack()
{
    KisImageWSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer;

    layer = activeLayer();
    m_commandsAdapter->toBottom(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::rotateLayer180()
{
    rotateLayer(M_PI);
}

void KisLayerManager::rotateLayerLeft90()
{
    rotateLayer(M_PI / 2 - 2*M_PI);
}

void KisLayerManager::rotateLayerRight90()
{
    rotateLayer(M_PI / 2);
}

void KisLayerManager::mirrorLayerX()
{
    KisPaintDeviceSP dev = activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Mirror Layer X"), dev);
        Q_CHECK_PTR(t);
    }

    QRect dirty = KisTransformWorker::mirrorX(dev, m_view->selection());
    m_activeLayer->setDirty(dirty);

    if (t) m_view->undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::mirrorLayerY()
{
    KisPaintDeviceSP dev = activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Mirror Layer Y"), dev);
        Q_CHECK_PTR(t);
    }

    QRect dirty = KisTransformWorker::mirrorY(dev, m_view->selection());
    m_activeLayer->setDirty(dirty);

    if (t) m_view->undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    if (!m_view->image()) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KisSelectedTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisSelectedTransaction(i18n("Scale Layer"), layer);
        Q_CHECK_PTR(t);
    }

    KoProgressUpdater* updater = m_view->createProgressUpdater();
    KoUpdaterPtr u = updater->startSubtask();

    KisTransformWorker worker(layer->paintDevice(), sx, sy, 0, 0, 0.0, 0, 0, u, filterStrategy);
    worker.run();

    if (t) m_view->undoAdapter()->addCommand(t);
    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();

    updater->deleteLater();
}

void KisLayerManager::rotateLayer(double radians)
{
    if (!m_view->image()) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KisSelectedTransaction * t = 0;
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        t = new KisSelectedTransaction(i18n("Rotate Layer"), layer);
        Q_CHECK_PTR(t);
    }

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Triangle");
    QRect r;

    if (KisSelectionSP selection = activeLayer()->selection())
        r = selection->selectedExactRect();
    else
        r = layer->exactBounds();
    double cx = r.x() + r.width() / 2.0;
    double cy = r.y() + r.height() / 2.0;
    qint32 tx = qint32(cx * cos(radians) - cy * sin(radians) - cx + 0.5);
    qint32 ty = qint32(cy * cos(radians) + cx * sin(radians) - cy + 0.5);
    KisTransformWorker tw(layer->paintDevice(), 1.0, 1.0, 0, 0, radians, -tx, -ty, 0, filter);
    tw.run();

    if (t) m_view->undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();
}

void KisLayerManager::shearLayer(double angleX, double angleY)
{
    if (!m_view->image()) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KisUndoAdapter * undo = 0;
    if ((undo = m_view->image()->undoAdapter())) {
        undo->beginMacro(i18n("Shear layer"));
    }

    KoProgressUpdater* updater = m_view->statusBar()->progress()->createUpdater();
    updater->start(100, i18n("Shear layer"));
    KoUpdaterPtr up = updater->startSubtask();

    KisShearVisitor v(angleX, angleY, up);
    v.setUndoAdapter(undo);
    layer->accept(v);

    if (undo) undo->endMacro();

    m_doc->setModified(true);
    layersUpdated();
    m_view->canvas()->update();

    m_view->statusBar()->progress()->detachUpdater(updater);
    updater->deleteLater();
}

void KisLayerManager::flattenImage()
{
    KisImageWSP img = m_view->image();

    if (img) {
        bool doIt = true;

        if (img->nHiddenLayers() > 0) {
            int answer = KMessageBox::warningYesNo(m_view,
                                                   i18n("The image contains hidden layers that will be lost."),
                                                   i18n("Flatten Image"),
                                                   KGuiItem(i18n("&Flatten Image")),
                                                   KStandardGuiItem::cancel());

            if (answer != KMessageBox::Yes) {
                doIt = false;
            }
        }

        if (doIt) {
            img->flatten();
        }
    }
}

void KisLayerManager::mergeLayer()
{
    KisImageWSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    const KisMetaData::MergeStrategy* strategy = KisMetaDataMergeStrategyChooserWidget::showDialog(m_view);
    if (!strategy) return;

    KisLayerSP  newLayer = img->mergeLayer(layer, strategy);
    if (newLayer) {
        newLayer->setDirty();
    }

    m_view->updateGUI();

}

void KisLayerManager::flattenLayer()
{
    KisImageWSP img = m_view->image();
    if (!img) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KisLayerSP newLayer = img->flattenLayer(layer);
    if (newLayer) {
        newLayer->setDirty();
    }
    m_view->updateGUI();
}

void KisLayerManager::layersUpdated()
{
    KisLayerSP layer = activeLayer();
    if (!layer) return;

    m_view->updateGUI();
}

void KisLayerManager::saveLayerAsImage()
{
    QStringList listMimeFilter = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Export);
    QString mimelist = listMimeFilter.join(" ");

    KFileDialog fd(KUrl(QString()), mimelist, m_view);
    fd.setObjectName("Export Layer");
    fd.setCaption(i18n("Export Layer"));
    fd.setMimeFilter(listMimeFilter);
    fd.setOperationMode(KFileDialog::Saving);

    if (!fd.exec()) return;

    KUrl url = fd.selectedUrl();
    QString mimefilter = fd.currentMimeFilter();

    if (mimefilter.isNull()) {
        KMimeType::Ptr mime = KMimeType::findByUrl(url);
        mimefilter = mime->name();
    }

    if (url.isEmpty())
        return;

    KisImageWSP img = m_view->image();
    if (!img) return;

    KisLayerSP l = activeLayer();
    if (!l) return;

    QRect r = img->bounds();

    KisDoc2 d;
    d.prepareForImport();

    KisImageWSP dst = new KisImage(d.undoAdapter(), r.width(), r.height(), img->colorSpace(), l->name());
    dst->setResolution(img->xRes(), img->yRes());
    d.setCurrentImage(dst);
    KisPaintLayer* paintLayer = new KisPaintLayer(dst, "projection", l->opacity());
    KisPainter gc(paintLayer->paintDevice());
    gc.bitBlt(QPoint(0, 0), l->projection(), l->projection()->exactBounds());
    dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));

    dst->resize(paintLayer->exactBounds());
    dst->refreshGraph();

    d.setOutputMimeType(mimefilter.toLatin1());
    d.exportDocument(url);
}

bool KisLayerManager::activeLayerHasSelection()
{
    return (activeLayer()->selection() != 0);
}


#include "kis_layer_manager.moc"

