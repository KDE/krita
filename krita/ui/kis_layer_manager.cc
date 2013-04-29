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
#include <QVBoxLayout>
#include <QFileInfo>

#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kfilewidget.h>
#include <kurl.h>
#include <kdiroperator.h>
#include <kurlcombobox.h>

#include <KoIcon.h>
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
#include <KoDocument.h>
#include <KoPart.h>

#include <filter/kis_filter_configuration.h>
#include <filter/kis_filter.h>
#include <kis_filter_strategy.h>
#include <generator/kis_generator_layer.h>
#include <kis_file_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_mask.h>
#include <kis_clone_layer.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <flake/kis_shape_layer.h>
#include <kis_transform_visitor.h>
#include <kis_undo_adapter.h>
#include <kis_painter.h>
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_merge_strategy_registry.h>
#include <kis_file_layer.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "dialogs/kis_dlg_adj_layer_props.h"
#include "dialogs/kis_dlg_adjustment_layer.h"
#include "dialogs/kis_dlg_layer_properties.h"
#include "dialogs/kis_dlg_generator_layer.h"
#include "dialogs/kis_dlg_file_layer.h"
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
#include "kis_action.h"
#include "kis_action_manager.h"

class KisSaveGroupVisitor : public KisNodeVisitor
{
public:
    KisSaveGroupVisitor(KisView2 *view,
                        KisImageWSP image,
                        bool saveInvisible,
                        bool saveTopLevelOnly,
                        const KUrl &url,
                        const QString &baseName,
                        const QString &extension,
                        const QString &mimeFilter)
        : m_view(view)
        , m_image(image)
        , m_saveInvisible(saveInvisible)
        , m_saveTopLevelOnly(saveTopLevelOnly)
        , m_url(url)
        , m_baseName(baseName)
        , m_extension(extension)
        , m_mimeFilter(mimeFilter)
    {
    }

    virtual ~KisSaveGroupVisitor()
    {
    }

public:

    bool visit(KisNode* ) {
        return true;
    }

    bool visit(KisPaintLayer *) {
        return true;
    }

    bool visit(KisAdjustmentLayer *) {
        return true;
    }


    bool visit(KisExternalLayer *) {
        return true;
    }


    bool visit(KisCloneLayer *) {
        return true;
    }


    bool visit(KisFilterMask *) {
        return true;
    }


    bool visit(KisTransparencyMask *) {
        return true;
    }


    bool visit(KisGeneratorLayer * ) {
        return true;
    }

    bool visit(KisSelectionMask* ) {
        return true;
    }

    bool visit(KisGroupLayer *layer)
    {
        if (layer == m_image->rootLayer()) {
            KisLayerSP child = dynamic_cast<KisLayer*>(layer->firstChild().data());
            while (child) {
                child->accept(*this);
                child = dynamic_cast<KisLayer*>(child->nextSibling().data());
            }

        }
        else if (layer->visible() || m_saveInvisible) {

            QRect r = m_image->bounds();

            KisDoc2 d;

            d.prepareForImport();

            KisImageWSP dst = new KisImage(d.createUndoStore(), r.width(), r.height(), m_image->colorSpace(), layer->name());
            dst->setResolution(m_image->xRes(), m_image->yRes());
            d.setCurrentImage(dst);
            KisPaintLayer* paintLayer = new KisPaintLayer(dst, "projection", layer->opacity());
            KisPainter gc(paintLayer->paintDevice());
            gc.bitBlt(QPoint(0, 0), layer->projection(), r);
            dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));

            dst->refreshGraph();

            d.setOutputMimeType(m_mimeFilter.toLatin1());
            d.setSaveInBatchMode(true);


            KUrl url = m_url;
            url.adjustPath(KUrl::AddTrailingSlash);

            url.setFileName(m_baseName + "_" + layer->name().replace(" ", "_") + "." + m_extension);

            d.exportDocument(url);

            if (!m_saveTopLevelOnly) {
                KisGroupLayerSP child = dynamic_cast<KisGroupLayer*>(layer->firstChild().data());
                while (child) {
                    child->accept(*this);
                    child = dynamic_cast<KisGroupLayer*>(child->nextSibling().data());
                }
            }
        }
        return true;
    }

private:

    KisView2 *m_view;
    KisImageWSP m_image;
    bool m_saveInvisible;
    bool m_saveTopLevelOnly;
    KUrl m_url;
    QString m_baseName;
    QString m_extension;
    QString m_mimeFilter;
};



KisLayerManager::KisLayerManager(KisView2 * view, KisDoc2 * doc)
    : m_view(view)
    , m_doc(doc)
    , m_imageFlatten(0)
    , m_imageMergeLayer(0)
    , m_groupLayersSave(0)
    , m_actLayerVis(false)
    , m_imageResizeToLayer(0)
    , m_flattenLayer(0)
    , m_rasterizeLayer(0)
    , m_addPaintLayer(0)
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
    m_imageFlatten  = new KAction(i18n("&Flatten image"), this);
    actionCollection->addAction("flatten_image", m_imageFlatten);
    m_imageFlatten->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E));
    connect(m_imageFlatten, SIGNAL(triggered()), this, SLOT(flattenImage()));

    m_imageMergeLayer  = new KAction(i18n("&Merge with Layer Below"), this);
    actionCollection->addAction("merge_layer", m_imageMergeLayer);
    m_imageMergeLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(m_imageMergeLayer, SIGNAL(triggered()), this, SLOT(mergeLayer()));

    m_flattenLayer  = new KAction(i18n("&Flatten Layer"), this);
    actionCollection->addAction("flatten_layer", m_flattenLayer);
    connect(m_flattenLayer, SIGNAL(triggered()), this, SLOT(flattenLayer()));

    m_rasterizeLayer  = new KisAction(i18n("Rasterize Layer"), this);
    m_rasterizeLayer->setActivationFlags(KisAction::ACTIVE_SHAPE_LAYER);
    m_rasterizeLayer->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    m_view->actionManager()->addAction("rasterize_layer", m_rasterizeLayer, actionCollection);
    connect(m_rasterizeLayer, SIGNAL(triggered()), this, SLOT(rasterizeLayer()));

    m_groupLayersSave = new KAction(koIcon("document-save"), i18n("Save Group Layers..."), this);
    actionCollection->addAction("save_groups_as_images", m_groupLayersSave);
    connect(m_groupLayersSave, SIGNAL(triggered()), this, SLOT(saveGroupLayers()));

    m_imageResizeToLayer  = new KisAction(i18n("Size Canvas to Size of Current Layer"), this);
    m_imageResizeToLayer->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_view->actionManager()->addAction("resizeimagetolayer", m_imageResizeToLayer, actionCollection);
    connect(m_imageResizeToLayer, SIGNAL(triggered()), this, SLOT(imageResizeToActiveLayer()));

    m_addPaintLayer = new KAction(i18n("Add new paint layer"), this);
    m_addPaintLayer->setShortcut(KShortcut(Qt::Key_Insert));
    actionCollection->addAction("add_new_paint_layer", m_addPaintLayer);
    connect(m_addPaintLayer, SIGNAL(triggered()), this, SLOT(layerAdd()));
}

void KisLayerManager::updateGUI()
{
    KisImageWSP image = m_view->image();

    KisLayerSP layer;
    qint32 nlayers = 0;


    if (image) {
        layer = m_activeLayer;
        nlayers = image->nlayers();
    }

    // XXX these should be named layer instead of image
    m_imageFlatten->setEnabled(nlayers > 1);
    m_imageMergeLayer->setEnabled(nlayers > 1 && layer && layer->prevSibling());
    m_flattenLayer->setEnabled(nlayers > 1 && layer && layer->firstChild());

    if (m_view->statusBar())
        m_view->statusBar()->setProfile(image);
}

void KisLayerManager::imageResizeToActiveLayer()
{
    KisLayerSP layer;
    KisImageWSP image = m_view->image();

    if (image && (layer = activeLayer())) {
        image->cropImage(layer->exactBounds());
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


    if (KisAdjustmentLayerSP alayer = KisAdjustmentLayerSP(dynamic_cast<KisAdjustmentLayer*>(layer.data()))) {
        KisPaintDeviceSP dev = alayer->projection();
        KisLayerSP prev = dynamic_cast<KisLayer*>(alayer->prevSibling().data());
        if (prev) dev = prev->projection();

        KisDlgAdjLayerProps dlg(alayer, alayer.data(), dev, m_view, alayer->filter().data(), alayer->name(), i18n("Filter Layer Properties"), m_view, "dlgadjlayerprops");
        dlg.resize(dlg.minimumSizeHint());


        KisSafeFilterConfigurationSP configBefore(alayer->filter());
        Q_ASSERT(configBefore);
        QString xmlBefore = configBefore->toXML();


        if (dlg.exec() == QDialog::Accepted) {

            alayer->setName(dlg.layerName());

            KisSafeFilterConfigurationSP configAfter(dlg.filterConfiguration());
            Q_ASSERT(configAfter);
            QString xmlAfter = configAfter->toXML();

            if(xmlBefore != xmlAfter) {
                KisChangeFilterCmd *cmd
                    = new KisChangeFilterCmd(alayer,
                                             configBefore->name(),
                                             xmlBefore,
                                             configAfter->name(),
                                             xmlAfter,
                                             false);
                // FIXME: check whether is needed
                cmd->redo();
                m_view->undoAdapter()->addCommand(cmd);
                m_doc->setModified(true);
            }
        }
        else {
            KisSafeFilterConfigurationSP configAfter(dlg.filterConfiguration());
            Q_ASSERT(configAfter);
            QString xmlAfter = configAfter->toXML();

            if(xmlBefore != xmlAfter) {
                alayer->setFilter(KisFilterRegistry::instance()->cloneConfiguration(configBefore.data()));
                alayer->setDirty();
            }
        }
    }
    else if (KisGeneratorLayerSP alayer = KisGeneratorLayerSP(dynamic_cast<KisGeneratorLayer*>(layer.data()))) {

        KisDlgGeneratorLayer dlg(alayer->name(), m_view);
        dlg.setCaption(i18n("Generator Layer Properties"));

        KisSafeFilterConfigurationSP configBefore(alayer->filter());
        Q_ASSERT(configBefore);
        QString xmlBefore = configBefore->toXML();

        dlg.setConfiguration(configBefore.data());
        dlg.resize(dlg.minimumSizeHint());

        if (dlg.exec() == QDialog::Accepted) {

            KisSafeFilterConfigurationSP configAfter(dlg.configuration());
            Q_ASSERT(configAfter);
            QString xmlAfter = configAfter->toXML();

            if(xmlBefore != xmlAfter) {
                KisChangeFilterCmd *cmd
                    = new KisChangeFilterCmd(alayer,
                                             configBefore->name(),
                                             xmlBefore,
                                             configAfter->name(),
                                             xmlAfter,
                                             true);
                // FIXME: check whether is needed
                cmd->redo();
                m_view->undoAdapter()->addCommand(cmd);
                m_doc->setModified(true);
            }

        }
    } else { // If layer == normal painting layer, vector layer, or group layer
        KisDlgLayerProperties *dialog = new KisDlgLayerProperties(layer, m_view, m_doc);
        dialog->resize(dialog->minimumSizeHint());
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}

void KisLayerManager::layerAdd()
{
    KisImageWSP image = m_view->image();
    if (image && activeLayer()) {
        addLayer(activeLayer()->parent(), activeLayer());
    } else if (image)
        addLayer(image->rootLayer(), KisLayerSP(0));
}

void KisLayerManager::addLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP image = m_view->image();
    if (image) {
        KisConfig cfg;
        QString profilename;
        KisLayerSP layer = new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, image->colorSpace());
        if (layer) {
            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent.data(), above.data());
        } else {
            KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
        }
    }
}

void KisLayerManager::addGroupLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP image = m_view->image();
    if (image) {
        KisLayerSP layer = KisLayerSP(new KisGroupLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8));
        if (layer) {
            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent.data(), above.data());
        } else {
            KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
        }
    }
}


void KisLayerManager::addCloneLayer()
{
    KisImageWSP image = m_view->image();
    if (image && activeLayer()) {
        addCloneLayer(activeLayer()->parent(), activeLayer());
    } else if (image)
        addCloneLayer(image->rootLayer(), KisLayerSP(0));
}

void KisLayerManager::addCloneLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP image = m_view->image();
    if (image) {
        // Check whether we are not cloning a parent layer
        if (KisGroupLayer * from = dynamic_cast<KisGroupLayer*>(m_activeLayer.data())) {
            KisNodeSP parent1 = parent;
            while (parent1 && parent1 != image->root()) {
                if (parent1.data() == from) {
                    // The chosen layer is one of our own parents -- this will
                    // lead to cyclic behaviour when updating. So we need to change parent
                    parent = parent1->parent();
                    above = parent1;
                }
                parent1 = parent1->parent();
            }
        }

        KisLayerSP layer = new KisCloneLayer(m_activeLayer, image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8);

        if (layer) {

            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent.data(), above.data());

        } else {
            KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
        }
    }
}


void KisLayerManager::addShapeLayer()
{
    KisImageWSP image = m_view->image();
    if (image && activeLayer()) {
        addShapeLayer(activeLayer()->parent(), activeLayer());
    } else if (image)
        addShapeLayer(image->rootLayer(), KisLayerSP(0));
}


void KisLayerManager::addShapeLayer(KisNodeSP parent, KisNodeSP above)
{
    KisImageWSP image = m_view->image();
    if (image) {
        // XXX: Make work with nodes!
        KisLayer * parentLayer = dynamic_cast<KisLayer*>(parent.data());
        KoShapeContainer * parentContainer =
                dynamic_cast<KoShapeContainer*>(m_doc->shapeForNode(parentLayer));
        if (!parentContainer) return;

        KisLayerSP layer = new KisShapeLayer(parentContainer, m_doc->shapeController(), image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8);
        if (layer) {
            layer->setCompositeOp(COMPOSITE_OVER);
            m_commandsAdapter->addNode(layer.data(), parent, above.data());
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

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP l = activeLayer();

    KisSelectionSP selection = l->selection();
    KisAdjustmentLayerSP adjl = addAdjustmentLayer(parent, above, QString(), 0, selection);

    KisPaintDeviceSP previewDevice = new KisPaintDevice(*adjl->original());

    KisDlgAdjustmentLayer dlg(adjl, adjl.data(), previewDevice, image->nextLayerName(), i18n("New Filter Layer"), m_view);
    dlg.resize(dlg.minimumSizeHint());

    // ensure that the device may be free'd by the dialog
    // when it is not needed anymore
    previewDevice = 0;

    if (dlg.exec() != QDialog::Accepted) {
        m_commandsAdapter->undoLastCommand();
    } else {
        adjl->setName(dlg.layerName());
    }
}

KisAdjustmentLayerSP KisLayerManager::addAdjustmentLayer(KisNodeSP parent, KisNodeSP above, const QString & name,
                                                         KisFilterConfiguration * filter, KisSelectionSP selection)
{
    Q_ASSERT(parent);

    KisImageWSP image = m_view->image();
    if (!image) return 0;

    KisAdjustmentLayerSP l = new KisAdjustmentLayer(image, name, filter, selection);
    m_commandsAdapter->addNode(l.data(), parent, above);
    l->setDirty(image->bounds());
    image->waitForDone();
    return l;
}

void KisLayerManager::addGeneratorLayer()
{
    addGeneratorLayer(activeLayer()->parent(), activeLayer());
}

void KisLayerManager::addGeneratorLayer(KisNodeSP parent, KisNodeSP above)
{
    Q_ASSERT(parent);

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisDlgGeneratorLayer dlg(image->nextLayerName(), m_view);
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

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisGeneratorLayerSP l = new KisGeneratorLayer(image, name, generator, selection);
    m_commandsAdapter->addNode(l.data(), parent, above.data());
}

void KisLayerManager::layerDuplicate()
{
    KisImageWSP image = m_view->image();

    if (!image)
        return;

    KisLayerSP active = activeLayer();

    if (!active)
        return;

    KisLayerSP dup = dynamic_cast<KisLayer*>(active->clone().data());
    m_commandsAdapter->addNode(dup.data(), active->parent(), active.data());
    if (dup) {
        activateLayer(dup);
    } else {
        KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
    }
}

void KisLayerManager::layerRaise()
{
    KisImageWSP image = m_view->image();
    KisLayerSP layer;

    if (!image)
        return;

    layer = activeLayer();

    m_commandsAdapter->raise(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::layerLower()
{
    KisImageWSP image = m_view->image();
    KisLayerSP layer;

    if (!image)
        return;

    layer = activeLayer();

    m_commandsAdapter->lower(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::layerFront()
{
    KisImageWSP image = m_view->image();
    KisLayerSP layer;

    if (!image)
        return;

    layer = activeLayer();
    m_commandsAdapter->toTop(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::layerBack()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer;

    layer = activeLayer();
    m_commandsAdapter->toBottom(layer);
    layer->parent()->setDirty();
}

void KisLayerManager::scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    if (!m_view->image()) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KoProgressUpdater* updater = m_view->createProgressUpdater();
    KoUpdaterPtr u = updater->startSubtask();

    m_view->image()->undoAdapter()->beginMacro(i18n("Scale Layer"));

    KisTransformVisitor visitor(m_view->image(), sx, sy, 0.0, 0.0, 0.0, 0, 0, u, filterStrategy);
    layer->accept(visitor);

    m_view->image()->undoAdapter()->endMacro();
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

    m_view->image()->rotateNode(layer, radians);
}

void KisLayerManager::shearLayer(double angleX, double angleY)
{
    if (!m_view->image()) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    m_view->image()->shearNode(layer, angleX, angleY);
}

void KisLayerManager::flattenImage()
{
    KisImageWSP image = m_view->image();

    if (image) {
        bool doIt = true;

        if (image->nHiddenLayers() > 0) {
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
            image->flatten();
        }
    }
}

void KisLayerManager::mergeLayer()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    if (!layer->prevSibling()) return;
    KisLayer *prevLayer = dynamic_cast<KisLayer*>(layer->prevSibling().data());
    if (!prevLayer) return;

    if (layer->metaData()->isEmpty() && prevLayer->metaData()->isEmpty()) {
        image->mergeDown(layer, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
    }
    else {
        const KisMetaData::MergeStrategy* strategy = KisMetaDataMergeStrategyChooserWidget::showDialog(m_view);
        if (!strategy) return;
        image->mergeDown(layer, strategy);

    }


    m_view->updateGUI();
}

void KisLayerManager::flattenLayer()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KisLayerSP newLayer = image->flattenLayer(layer);
    if (newLayer) {
        newLayer->setDirty();
    }
    m_view->updateGUI();
}

void KisLayerManager::rasterizeLayer()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, layer->name(), layer->opacity());
    KisPainter gc(paintLayer->paintDevice());
    QRect rc = layer->projection()->exactBounds();
    gc.bitBlt(rc.topLeft(), layer->projection(), rc);

    m_commandsAdapter->beginMacro(i18n("Rasterize Layer"));
    m_commandsAdapter->addNode(paintLayer.data(), layer->parent().data(), layer.data());

    int childCount = layer->childCount();
    for (int i = 0; i < childCount; i++) {
        m_commandsAdapter->moveNode(layer->firstChild(), paintLayer, paintLayer->lastChild());
    }
    m_commandsAdapter->removeNode(layer);
    m_commandsAdapter->endMacro();
    updateGUI();
}


void KisLayerManager::layersUpdated()
{
    KisLayerSP layer = activeLayer();
    if (!layer) return;

    m_view->updateGUI();
}

void KisLayerManager::saveGroupLayers()
{
    QStringList listMimeFilter = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Export);

    KDialog dlg;
    QWidget *page = new QWidget(&dlg);
    dlg.setMainWidget(page);
    QBoxLayout *layout = new QVBoxLayout(page);

    KFileWidget *fd = new KFileWidget(m_view->document()->url().path(), page);
    fd->setUrl(m_view->document()->url());
    fd->setMimeFilter(listMimeFilter);
    fd->setOperationMode(KAbstractFileWidget::Saving);
    layout->addWidget(fd);

    QCheckBox *chkInvisible = new QCheckBox(i18n("Convert Invisible Groups"), page);
    chkInvisible->setChecked(false);
    layout->addWidget(chkInvisible);
    QCheckBox *chkDepth = new QCheckBox(i18n("Export Only Toplevel Groups"), page);
    chkDepth->setChecked(true);
    layout->addWidget(chkDepth);

    if (!dlg.exec()) return;

    // selectedUrl()( does not return the expected result. So, build up the KUrl the more complicated way
    //return m_fileWidget->selectedUrl();
    KUrl url = fd->dirOperator()->url();
    url.adjustPath(KUrl::AddTrailingSlash);
    QString path = fd->locationEdit()->currentText();
    QFileInfo f(path);
    QString extension = f.completeSuffix();
    QString basename = f.baseName();

    QString mimefilter = fd->currentMimeFilter();

    if (mimefilter.isEmpty()) {
        KMimeType::Ptr mime = KMimeType::findByUrl(url);
        mimefilter = mime->name();
        extension = mime->extractKnownExtension(path);
    }

    if (url.isEmpty())
        return;

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisSaveGroupVisitor v(m_view, image, chkInvisible->isChecked(), chkDepth->isChecked(), url, basename, extension, mimefilter);
    image->rootLayer()->accept(v);

}

bool KisLayerManager::activeLayerHasSelection()
{
    return (activeLayer()->selection() != 0);
}


void KisLayerManager::addFileLayer()
{
    addFileLayer(activeLayer()->parent(), activeLayer());
}

void KisLayerManager::addFileLayer(KisNodeSP parent, KisNodeSP above)
{
    Q_ASSERT(parent);

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisDlgFileLayer dlg(image->nextLayerName(), m_view);
    dlg.resize(dlg.minimumSizeHint());

    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.layerName();
        QString fileName = dlg.fileName();
        bool scaleToImageResolution = dlg.scaleToImageResolution();

        addFileLayer(parent, above, name, fileName, scaleToImageResolution);
    }

}

void KisLayerManager::addFileLayer(KisNodeSP parent, KisNodeSP above, const QString &name, const QString &fileName, bool scaleToImageResolution)
{
    Q_ASSERT(parent);
    Q_ASSERT(!fileName.isEmpty());

    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = new KisFileLayer(image, fileName, scaleToImageResolution, name, OPACITY_OPAQUE_U8);
    if (layer) {
        layer->setCompositeOp(COMPOSITE_OVER);
        m_commandsAdapter->addNode(layer.data(), parent, above.data());
    } else {
        KMessageBox::error(m_view, i18n("Could not add layer to image."), i18n("Layer Error"));
    }


}


#include "kis_layer_manager.moc"

