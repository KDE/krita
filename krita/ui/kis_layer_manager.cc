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
#include <QDesktopServices>
#include <QScopedPointer>

#include <kactioncollection.h>
#include <klocale.h>
#include <QMessageBox>
#include <kfilewidget.h>
#include <kurl.h>
#include <kdiroperator.h>
#include <kurlcombobox.h>

#include <KoIcon.h>
#include <KisImportExportManager.h>
#include <KisDocument.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KoPointerEvent.h>
#include <KoColorProfile.h>
#include <KoSelection.h>
#include <KisPart.h>
#include <KisMainWindow.h>

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
#include <kis_selection.h>
#include <flake/kis_shape_layer.h>
#include <kis_undo_adapter.h>
#include <kis_painter.h>
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_merge_strategy_registry.h>
#include <kis_psd_layer_style.h>

#include "KisView.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "dialogs/kis_dlg_adj_layer_props.h"
#include "dialogs/kis_dlg_adjustment_layer.h"
#include "dialogs/kis_dlg_layer_properties.h"
#include "dialogs/kis_dlg_generator_layer.h"
#include "dialogs/kis_dlg_file_layer.h"
#include "dialogs/kis_dlg_layer_style.h"
#include "KisDocument.h"
#include "kis_filter_manager.h"
#include "kis_node_visitor.h"
#include "kis_paint_layer.h"
#include "commands/kis_image_commands.h"
#include "commands/kis_layer_commands.h"
#include "commands/kis_node_commands.h"
#include "kis_canvas_resource_provider.h"
#include "kis_selection_manager.h"
#include "kis_statusbar.h"
#include "KisViewManager.h"
#include "kis_zoom_manager.h"
#include "canvas/kis_canvas2.h"
#include "widgets/kis_meta_data_merge_strategy_chooser_widget.h"
#include "widgets/kis_wdg_generator.h"
#include "kis_progress_widget.h"
#include "kis_node_commands_adapter.h"
#include "kis_node_manager.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include "KisPart.h"

#include "kis_signal_compressor_with_param.h"
#include "kis_abstract_projection_plane.h"
#include "commands_new/kis_set_layer_style_command.h"
#include "kis_post_execution_undo_adapter.h"



class KisSaveGroupVisitor : public KisNodeVisitor
{
public:
    KisSaveGroupVisitor(KisImageWSP image,
                        bool saveInvisible,
                        bool saveTopLevelOnly,
                        const KUrl &url,
                        const QString &baseName,
                        const QString &extension,
                        const QString &mimeFilter)
        : m_image(image)
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

    bool visit(KisTransformMask *) {
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

            KisDocument *d = KisPart::instance()->createDocument();

            d->prepareForImport();

            KisImageWSP dst = new KisImage(d->createUndoStore(), r.width(), r.height(), m_image->colorSpace(), layer->name());
            dst->setResolution(m_image->xRes(), m_image->yRes());
            d->setCurrentImage(dst);
            KisPaintLayer* paintLayer = new KisPaintLayer(dst, "projection", layer->opacity());
            KisPainter gc(paintLayer->paintDevice());
            gc.bitBlt(QPoint(0, 0), layer->projection(), r);
            dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));

            dst->refreshGraph();

            d->setOutputMimeType(m_mimeFilter.toLatin1());
            d->setSaveInBatchMode(true);


            KUrl url = m_url;
            url.adjustPath(KUrl::AddTrailingSlash);

            url.setFileName(m_baseName + '_' + layer->name().replace(' ', '_') + '.' + m_extension);

            d->exportDocument(url);

            if (!m_saveTopLevelOnly) {
                KisGroupLayerSP child = dynamic_cast<KisGroupLayer*>(layer->firstChild().data());
                while (child) {
                    child->accept(*this);
                    child = dynamic_cast<KisGroupLayer*>(child->nextSibling().data());
                }
            }
            delete d;
        }

        return true;
    }

private:

    KisImageWSP m_image;
    bool m_saveInvisible;
    bool m_saveTopLevelOnly;
    KUrl m_url;
    QString m_baseName;
    QString m_extension;
    QString m_mimeFilter;
};

KisLayerManager::KisLayerManager(KisViewManager * view)
    : m_view(view)
    , m_imageView(0)
    , m_imageFlatten(0)
    , m_imageMergeLayer(0)
    , m_groupLayersSave(0)
    , m_imageResizeToLayer(0)
    , m_flattenLayer(0)
    , m_rasterizeLayer(0)
    , m_commandsAdapter(new KisNodeCommandsAdapter(m_view))
    , m_layerStyle(0)
{
}

KisLayerManager::~KisLayerManager()
{
    delete m_commandsAdapter;
}

void KisLayerManager::setView(QPointer<KisView>view)
{
    m_imageView = view;
}

KisLayerSP KisLayerManager::activeLayer()
{
    if (m_imageView) {
        return m_imageView->currentLayer();
    }
    return 0;
}

KisPaintDeviceSP KisLayerManager::activeDevice()
{
    if (activeLayer()) {
        return activeLayer()->paintDevice();
    }
    return 0;
}

void KisLayerManager::activateLayer(KisLayerSP layer)
{
    if (m_imageView) {
        emit sigLayerActivated(layer);
        layersUpdated();
        if (layer) {
            m_view->resourceProvider()->slotNodeActivated(layer.data());
        }
    }
}


void KisLayerManager::setup(KisActionManager* actionManager)
{
    m_imageFlatten = new KisAction(i18n("&Flatten image"), this);
    m_imageFlatten->setActivationFlags(KisAction::ACTIVE_LAYER);
    actionManager->addAction("flatten_image", m_imageFlatten);
    m_imageFlatten->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E));
    connect(m_imageFlatten, SIGNAL(triggered()), this, SLOT(flattenImage()));

    m_imageMergeLayer = new KisAction(i18n("&Merge with Layer Below"), this);
    m_imageMergeLayer->setActivationFlags(KisAction::ACTIVE_LAYER);
    actionManager->addAction("merge_layer", m_imageMergeLayer);
    m_imageMergeLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(m_imageMergeLayer, SIGNAL(triggered()), this, SLOT(mergeLayer()));

    m_flattenLayer = new KisAction(i18n("&Flatten Layer"), this);
    m_flattenLayer->setActivationFlags(KisAction::ACTIVE_LAYER);
    actionManager->addAction("flatten_layer", m_flattenLayer);
    connect(m_flattenLayer, SIGNAL(triggered()), this, SLOT(flattenLayer()));

    KisAction * action = new KisAction(i18n("Rename current layer"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    actionManager->addAction("RenameCurrentLayer", action);
    action->setShortcut(KShortcut(Qt::Key_F2));
    connect(action, SIGNAL(triggered()), this, SLOT(layerProperties()));

    m_rasterizeLayer = new KisAction(i18n("Rasterize Layer"), this);
    m_rasterizeLayer->setActivationFlags(KisAction::ACTIVE_SHAPE_LAYER);
    m_rasterizeLayer->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    actionManager->addAction("rasterize_layer", m_rasterizeLayer);
    connect(m_rasterizeLayer, SIGNAL(triggered()), this, SLOT(rasterizeLayer()));

    m_groupLayersSave = new KisAction(koIcon("document-save"), i18n("Save Group Layers..."), this);
    m_groupLayersSave->setActivationFlags(KisAction::ACTIVE_LAYER);
    actionManager->addAction("save_groups_as_images", m_groupLayersSave);
    connect(m_groupLayersSave, SIGNAL(triggered()), this, SLOT(saveGroupLayers()));

    m_imageResizeToLayer = new KisAction(i18n("Trim to Current Layer"), this);
    m_imageResizeToLayer->setActivationFlags(KisAction::ACTIVE_LAYER);
    actionManager->addAction("resizeimagetolayer", m_imageResizeToLayer);
    connect(m_imageResizeToLayer, SIGNAL(triggered()), this, SLOT(imageResizeToActiveLayer()));

    m_layerStyle  = new KisAction(i18n("Layer Style..."), this);
    m_layerStyle->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_layerStyle->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    actionManager->addAction("layer_style", m_layerStyle);
    connect(m_layerStyle, SIGNAL(triggered()), this, SLOT(layerStyle()));

}

void KisLayerManager::updateGUI()
{
    KisImageWSP image = m_view->image();

    KisLayerSP layer;
    qint32 nlayers = 0;

    if (image) {
        layer = activeLayer();
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
        QRect cropRect = layer->projection()->nonDefaultPixelArea();
        if (!cropRect.isEmpty()) {
            image->cropImage(cropRect);
        } else {
            m_view->showFloatingMessage(
                i18nc("floating message in layer manager",
                      "Layer is empty "),
                QIcon(), 2000, KisFloatingMessage::Low);
        }
    }
}

void KisLayerManager::layerProperties()
{
    if (!m_view) return;
    if (!m_view->document()) return;

    KisLayerSP layer = activeLayer();

    if (!layer) return;


    if (KisAdjustmentLayerSP alayer = KisAdjustmentLayerSP(dynamic_cast<KisAdjustmentLayer*>(layer.data()))) {
        KisPaintDeviceSP dev = alayer->projection();

        KisDlgAdjLayerProps dlg(alayer, alayer.data(), dev, m_view, alayer->filter().data(), alayer->name(), i18n("Filter Layer Properties"), m_view->mainWindow(), "dlgadjlayerprops");
        dlg.resize(dlg.minimumSizeHint());


        KisSafeFilterConfigurationSP configBefore(alayer->filter());
        KIS_ASSERT_RECOVER_RETURN(configBefore);
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
                m_view->document()->setModified(true);
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

        KisDlgGeneratorLayer dlg(alayer->name(), m_view, m_view->mainWindow());
        dlg.setCaption(i18n("Fill Layer Properties"));

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
                m_view->document()->setModified(true);
            }

        }
    } else { // If layer == normal painting layer, vector layer, or group layer
        KisDlgLayerProperties *dialog = new KisDlgLayerProperties(layer, m_view, m_view->document());
        dialog->resize(dialog->minimumSizeHint());
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        Qt::WindowFlags flags = dialog->windowFlags();
        dialog->setWindowFlags(flags | Qt::WindowStaysOnTopHint | Qt::Dialog);
        dialog->show();

    }
}

void KisLayerManager::convertNodeToPaintLayer(KisNodeSP source)
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisPaintDeviceSP srcDevice =
        source->paintDevice() ? source->projection() : source->original();

    if (!srcDevice) return;

    KisPaintDeviceSP clone;

    if (!(*srcDevice->colorSpace() ==
          *srcDevice->compositionSourceColorSpace())) {

        clone = new KisPaintDevice(srcDevice->compositionSourceColorSpace());
        KisPainter gc(clone);
        gc.setCompositeOp(COMPOSITE_COPY);
        QRect rc(srcDevice->extent());
        gc.bitBlt(rc.topLeft(), srcDevice, rc);
    } else {
        clone = new KisPaintDevice(*srcDevice);
    }

    KisLayerSP layer = new KisPaintLayer(image,
                                         source->name(),
                                         source->opacity(),
                                         clone);
    layer->setCompositeOp(source->compositeOpId());

    KisNodeSP parent = source->parent();
    KisNodeSP above = source;

    while (parent && !parent->allowAsChild(layer)) {
        above = above->parent();
        parent = above ? above->parent() : 0;
    }

    m_commandsAdapter->beginMacro(kundo2_i18n("Convert to a Paint Layer"));
    m_commandsAdapter->addNode(layer, parent, above);
    m_commandsAdapter->removeNode(source);
    m_commandsAdapter->endMacro();

}

void KisLayerManager::adjustLayerPosition(KisNodeSP node, KisNodeSP activeNode, KisNodeSP &parent, KisNodeSP &above)
{
    Q_ASSERT(activeNode);

    parent = activeNode;
    above = parent->lastChild();

    while (parent && !parent->allowAsChild(node)) {
        above = parent;
        parent = parent->parent();
    }

    if (!parent) {
        qWarning() << "KisLayerManager::adjustLayerPosition:"
                   << "No node accepted newly created node";

        parent = m_view->image()->root();
        above = parent->lastChild();
    }
}

void KisLayerManager::addLayerCommon(KisNodeSP activeNode, KisLayerSP layer)
{
    KisNodeSP parent;
    KisNodeSP above;
    adjustLayerPosition(layer, activeNode, parent, above);

    m_commandsAdapter->addNode(layer, parent, above);
}

void KisLayerManager::addLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();
    addLayerCommon(activeNode,
                   new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, image->colorSpace()));
}

void KisLayerManager::addGroupLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();
    addLayerCommon(activeNode,
                   new KisGroupLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8));
}

void KisLayerManager::addCloneLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();
    addLayerCommon(activeNode,
                   new KisCloneLayer(activeLayer(), image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8));
}

void KisLayerManager::addShapeLayer(KisNodeSP activeNode)
{
    if (!m_view) return;
    if (!m_view->document()) return;

    KisImageWSP image = m_view->image();
    KisShapeLayerSP layer = new KisShapeLayer(m_view->document()->shapeController(), image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8);

    addLayerCommon(activeNode, layer);
}

void KisLayerManager::addAdjustmentLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();

    KisSelectionSP selection = m_view->selection();
    KisAdjustmentLayerSP adjl = addAdjustmentLayer(activeNode, QString(), 0, selection);
    image->refreshGraph();

    KisPaintDeviceSP previewDevice = new KisPaintDevice(*adjl->original());

    KisDlgAdjustmentLayer dlg(adjl, adjl.data(), previewDevice, image->nextLayerName(), i18n("New Filter Layer"), m_view);
    dlg.resize(dlg.minimumSizeHint());

    // ensure that the device may be free'd by the dialog
    // when it is not needed anymore
    previewDevice = 0;

    if (dlg.exec() != QDialog::Accepted || adjl->filter().isNull()) {
        // XXX: add messagebox warning if there's no filter set!
        m_commandsAdapter->undoLastCommand();
    } else {
        adjl->setName(dlg.layerName());
    }

}

KisAdjustmentLayerSP KisLayerManager::addAdjustmentLayer(KisNodeSP activeNode, const QString & name,
                                                         KisFilterConfiguration * filter, KisSelectionSP selection)
{
    KisImageWSP image = m_view->image();
    KisAdjustmentLayerSP layer = new KisAdjustmentLayer(image, name, filter, selection);
    addLayerCommon(activeNode, layer);

    return layer;
}

void KisLayerManager::addGeneratorLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();

    KisDlgGeneratorLayer dlg(image->nextLayerName(), m_view, m_view->mainWindow());
    dlg.resize(dlg.minimumSizeHint());

    if (dlg.exec() == QDialog::Accepted) {
        KisSelectionSP selection = m_view->selection();
        KisFilterConfiguration * generator = dlg.configuration();
        QString name = dlg.layerName();

        addLayerCommon(activeNode,
            new KisGeneratorLayer(image, name, generator, selection));
    }

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
        QMessageBox::critical(m_view->mainWindow(),
                              i18nc("@title:window", "Krita"),
                              i18n("Could not add layer to image."));
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
            int answer = QMessageBox::warning(m_view->mainWindow(),
                                              i18nc("@title:window", "Flatten Image"),
                                              i18n("The image contains hidden layers that will be lost. Do you want to flatten the image?"),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);

            if (answer != QMessageBox::Yes) {
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
        const KisMetaData::MergeStrategy* strategy = KisMetaDataMergeStrategyChooserWidget::showDialog(m_view->mainWindow());
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

    m_commandsAdapter->beginMacro(kundo2_i18n("Rasterize Layer"));
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
    QStringList listMimeFilter = KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Export);

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

    KisSaveGroupVisitor v(image, chkInvisible->isChecked(), chkDepth->isChecked(), url, basename, extension, mimefilter);
    image->rootLayer()->accept(v);

}

bool KisLayerManager::activeLayerHasSelection()
{
    return (activeLayer()->selection() != 0);
}

void KisLayerManager::addFileLayer(KisNodeSP activeNode)
{

    QString basePath;
    KUrl url = m_view->document()->url();
    if (url.isLocalFile()) {
        basePath = QFileInfo(url.toLocalFile()).absolutePath();
    }
    KisImageWSP image = m_view->image();

    KisDlgFileLayer dlg(basePath, image->nextLayerName(), m_view->mainWindow());
    dlg.resize(dlg.minimumSizeHint());

    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.layerName();
        QString fileName = dlg.fileName();

        if(fileName.isEmpty()){
            QMessageBox::critical(m_view->mainWindow(), i18nc("@title:window", "Krita"), i18n("No file name specified"));
            return;
        }

        KisFileLayer::ScalingMethod scalingMethod = dlg.scaleToImageResolution();

        addLayerCommon(activeNode,
                       new KisFileLayer(image, basePath, fileName, scalingMethod, name, OPACITY_OPAQUE_U8));
    }

}

void updateLayerStyles(KisLayerSP layer, KisDlgLayerStyle *dlg)
{
    KisSetLayerStyleCommand::updateLayerStyle(layer, dlg->style()->clone());
}

void KisLayerManager::layerStyle()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    KisPSDLayerStyleSP oldStyle;
    if (layer->layerStyle()) {
        oldStyle = layer->layerStyle()->clone();
    }
    else {
        oldStyle = toQShared(new KisPSDLayerStyle());
    }

    KisDlgLayerStyle dlg(oldStyle->clone(), m_view->resourceProvider());

    boost::function<void ()> updateCall(boost::bind(updateLayerStyles, layer, &dlg));
    SignalToFunctionProxy proxy(updateCall);
    connect(&dlg, SIGNAL(configChanged()), &proxy, SLOT(start()));

    if (dlg.exec() == QDialog::Accepted) {
        KisPSDLayerStyleSP newStyle = dlg.style();

        KUndo2CommandSP command = toQShared(
            new KisSetLayerStyleCommand(layer, oldStyle, newStyle));

        image->postExecutionUndoAdapter()->addCommand(command);
    }
}

#include "kis_layer_manager.moc"

