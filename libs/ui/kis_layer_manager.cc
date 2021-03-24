/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_layer_manager.h"

#include <QRect>
#include <QApplication>
#include <QCursor>
#include <QString>
#include <QDialog>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QStandardPaths>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <QMessageBox>
#include <QUrl>

#include <kis_file_name_requester.h>
#include <kis_icon.h>
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
#include <kis_meta_data_store.h>
#include <kis_meta_data_merge_strategy_registry.h>
#include <kis_psd_layer_style.h>
#include <KisMimeDatabase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "dialogs/kis_dlg_adj_layer_props.h"
#include "dialogs/kis_dlg_adjustment_layer.h"
#include "dialogs/kis_dlg_layer_properties.h"
#include "dialogs/kis_dlg_generator_layer.h"
#include "dialogs/kis_dlg_file_layer.h"
#include "dialogs/kis_dlg_layer_style.h"
#include "dialogs/KisDlgChangeCloneSource.h"
#include "kis_filter_manager.h"
#include "kis_node_visitor.h"
#include "kis_paint_layer.h"
#include "commands/kis_image_commands.h"
#include "commands/kis_node_commands.h"
#include "kis_change_file_layer_command.h"
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
#include "kis_raster_keyframe_channel.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_abstract_projection_plane.h"
#include "commands_new/kis_set_layer_style_command.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_selection_mask.h"
#include "kis_layer_utils.h"
#include "lazybrush/kis_colorize_mask.h"
#include "kis_processing_applicator.h"
#include "kis_projection_leaf.h"
#include "KisGlobalResourcesInterface.h"

#include "KisSaveGroupVisitor.h"

KisLayerManager::KisLayerManager(KisViewManager * view)
    : m_view(view)
    , m_commandsAdapter(new KisNodeCommandsAdapter(m_view))

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
        layersUpdated();
        if (layer) {
            m_view->canvasResourceProvider()->slotNodeActivated(layer.data());
        }
    }
}


void KisLayerManager::setup(KisActionManager* actionManager)
{
    m_imageFlatten = actionManager->createAction("flatten_image");
    connect(m_imageFlatten, SIGNAL(triggered()), this, SLOT(flattenImage()));

    m_imageMergeLayer = actionManager->createAction("merge_layer");
    connect(m_imageMergeLayer, SIGNAL(triggered()), this, SLOT(mergeLayer()));

    m_flattenLayer = actionManager->createAction("flatten_layer");
    connect(m_flattenLayer, SIGNAL(triggered()), this, SLOT(flattenLayer()));

    m_groupLayersSave = actionManager->createAction("save_groups_as_images");
    connect(m_groupLayersSave, SIGNAL(triggered()), this, SLOT(saveGroupLayers()));

    m_convertGroupAnimated = actionManager->createAction("convert_group_to_animated");
    connect(m_convertGroupAnimated, SIGNAL(triggered()), this, SLOT(convertGroupToAnimated()));

    m_imageResizeToLayer = actionManager->createAction("resizeimagetolayer");
    connect(m_imageResizeToLayer, SIGNAL(triggered()), this, SLOT(imageResizeToActiveLayer()));

    KisAction *action = actionManager->createAction("trim_to_image");
    connect(action, SIGNAL(triggered()), this, SLOT(trimToImage()));

    m_layerStyle  = actionManager->createAction("layer_style");
    connect(m_layerStyle, SIGNAL(triggered()), this, SLOT(layerStyle()));

}

void KisLayerManager::updateGUI()
{
    KisImageSP image = m_view->image();
    KisLayerSP layer = activeLayer();

    const bool isGroupLayer = layer && layer->inherits("KisGroupLayer");

    m_imageMergeLayer->setText(
                isGroupLayer ?
                    i18nc("@action:inmenu", "Merge Group") :
                    i18nc("@action:inmenu", "Merge with Layer Below"));
    m_flattenLayer->setVisible(!isGroupLayer);

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

void KisLayerManager::trimToImage()
{
    KisImageWSP image = m_view->image();
    if (image) {
        image->cropImage(image->bounds());
    }
}

void KisLayerManager::layerProperties()
{
    if (!m_view) return;
    if (!m_view->document()) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    QList<KisNodeSP> selectedNodes = m_view->nodeManager()->selectedNodes();
    const bool multipleLayersSelected = selectedNodes.size() > 1;

    if (!m_view->nodeManager()->canModifyLayers(selectedNodes)) return;

    KisAdjustmentLayerSP adjustmentLayer = KisAdjustmentLayerSP(dynamic_cast<KisAdjustmentLayer*>(layer.data()));
    KisGeneratorLayerSP generatorLayer = KisGeneratorLayerSP(dynamic_cast<KisGeneratorLayer*>(layer.data()));
    KisFileLayerSP fileLayer = KisFileLayerSP(dynamic_cast<KisFileLayer*>(layer.data()));

    if (adjustmentLayer && !multipleLayersSelected) {

        KisPaintDeviceSP dev = adjustmentLayer->projection();

        KisDlgAdjLayerProps dlg(adjustmentLayer, adjustmentLayer.data(), dev, m_view, adjustmentLayer->filter().data(), adjustmentLayer->name(), i18n("Filter Layer Properties"), m_view->mainWindow(), "dlgadjlayerprops");
        dlg.resize(dlg.minimumSizeHint());


        KisFilterConfigurationSP configBefore(adjustmentLayer->filter());
        KIS_ASSERT_RECOVER_RETURN(configBefore);
        QString xmlBefore = configBefore->toXML();


        if (dlg.exec() == QDialog::Accepted) {

            adjustmentLayer->setName(dlg.layerName());

            KisFilterConfigurationSP configAfter(dlg.filterConfiguration());
            Q_ASSERT(configAfter);
            QString xmlAfter = configAfter->toXML();

            if(xmlBefore != xmlAfter) {
                KisChangeFilterCmd *cmd
                        = new KisChangeFilterCmd(adjustmentLayer,
                                                 configBefore->cloneWithResourcesSnapshot(),
                                                 configAfter->cloneWithResourcesSnapshot());
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
                adjustmentLayer->setFilter(configBefore->cloneWithResourcesSnapshot());
                adjustmentLayer->setDirty();
            }
        }
    }
    else if (generatorLayer && !multipleLayersSelected) {
        KisFilterConfigurationSP configBefore(generatorLayer->filter());
        Q_ASSERT(configBefore);

        KisDlgGeneratorLayer *dlg = new KisDlgGeneratorLayer(generatorLayer->name(), m_view, m_view->mainWindow(), generatorLayer, configBefore, KisStrokeId());
        dlg->setWindowTitle(i18n("Fill Layer Properties"));
        dlg->setAttribute(Qt::WA_DeleteOnClose);

        dlg->setConfiguration(configBefore.data());

        Qt::WindowFlags flags = dlg->windowFlags();
        dlg->setWindowFlags(flags | Qt::Tool | Qt::Dialog);
        dlg->show();

    }
    else if (fileLayer && !multipleLayersSelected){
        QString basePath = QFileInfo(m_view->document()->path()).absolutePath();
        QString fileNameOld = fileLayer->fileName();
        KisFileLayer::ScalingMethod scalingMethodOld = fileLayer->scalingMethod();
        KisDlgFileLayer dlg(basePath, fileLayer->name(), m_view->mainWindow());
        dlg.setCaption(i18n("File Layer Properties"));
        dlg.setFileName(fileNameOld);
        dlg.setScalingMethod(scalingMethodOld);

        if (dlg.exec() == QDialog::Accepted) {
            const QString fileNameNew = dlg.fileName();
            KisFileLayer::ScalingMethod scalingMethodNew = dlg.scaleToImageResolution();

            if(fileNameNew.isEmpty()){
                QMessageBox::critical(m_view->mainWindow(), i18nc("@title:window", "Krita"), i18n("No file name specified"));
                return;
            }
            fileLayer->setName(dlg.layerName());

            if (fileNameOld!= fileNameNew || scalingMethodOld != scalingMethodNew) {
                KisChangeFileLayerCmd *cmd
                        = new KisChangeFileLayerCmd(fileLayer,
                                                    basePath,
                                                    fileNameOld,
                                                    scalingMethodOld,
                                                    basePath,
                                                    fileNameNew,
                                                    scalingMethodNew);
                m_view->undoAdapter()->addCommand(cmd);
            }
        }
    } else { // If layer == normal painting layer, vector layer, or group layer
        QList<KisNodeSP> selectedNodes = m_view->nodeManager()->selectedNodes();

        KisDlgLayerProperties *dialog = new KisDlgLayerProperties(selectedNodes, m_view);
        dialog->resize(dialog->minimumSizeHint());
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        Qt::WindowFlags flags = dialog->windowFlags();
        dialog->setWindowFlags(flags | Qt::Tool | Qt::Dialog);
        dialog->show();
        dialog->activateWindow();
    }
}

void KisLayerManager::changeCloneSource()
{
    QList<KisNodeSP> selectedNodes = m_view->nodeManager()->selectedNodes();
    if (selectedNodes.isEmpty()) {
        return;
    }

    QList<KisCloneLayerSP> cloneLayers;
    KisNodeSP node;
    Q_FOREACH (node, selectedNodes) {
        KisCloneLayerSP cloneLayer(qobject_cast<KisCloneLayer *>(node.data()));
        if (cloneLayer) {
            cloneLayers << cloneLayer;
        }
    }

    if (cloneLayers.isEmpty()) {
        return;
    }

    if (!m_view->nodeManager()->canModifyLayers(implicitCastList<KisNodeSP>(cloneLayers))) return;

    KisDlgChangeCloneSource *dialog = new KisDlgChangeCloneSource(cloneLayers, m_view);
    dialog->setCaption(i18n("Change Clone Layer"));
    dialog->resize(dialog->minimumSizeHint());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    Qt::WindowFlags flags = dialog->windowFlags();
    dialog->setWindowFlags(flags | Qt::Tool | Qt::Dialog);
    dialog->show();
    dialog->activateWindow();
}

void KisLayerManager::convertNodeToPaintLayer(KisNodeSP source)
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    // this precondition must be checked at higher level
    KIS_SAFE_ASSERT_RECOVER_RETURN(source->isEditable(false));

    KisLayer *srcLayer = qobject_cast<KisLayer*>(source.data());
    if (srcLayer && (srcLayer->inherits("KisGroupLayer") || srcLayer->layerStyle() || srcLayer->childCount() > 0)) {
        image->flattenLayer(srcLayer);
        return;
    }

    KisLayerUtils::convertToPaintLayer(image, source);
}

void KisLayerManager::convertGroupToAnimated()
{
    KisGroupLayerSP group = dynamic_cast<KisGroupLayer*>(activeLayer().data());
    if (group.isNull()) return;

    if (!m_view->nodeManager()->canModifyLayer(group)) return;

    KisPaintLayerSP animatedLayer = new KisPaintLayer(m_view->image(), group->name(), OPACITY_OPAQUE_U8);
    animatedLayer->enableAnimation();
    KisRasterKeyframeChannel *contentChannel = dynamic_cast<KisRasterKeyframeChannel*>(
                animatedLayer->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true));
    KIS_ASSERT_RECOVER_RETURN(contentChannel);

    KisNodeSP child = group->firstChild();
    int time = 0;
    while (child) {
        contentChannel->importFrame(time, child->projection(), NULL);
        time++;

        child = child->nextSibling();
    }

    m_commandsAdapter->beginMacro(kundo2_i18n("Convert to an animated layer"));
    m_commandsAdapter->addNode(animatedLayer, group->parent(), group);
    m_commandsAdapter->removeNode(group);
    m_commandsAdapter->endMacro();
}

void KisLayerManager::convertLayerToFileLayer(KisNodeSP source)
{
    KisImageSP image = m_view->image();
    if (!image) return;

    // this precondition must be checked at higher level
    KIS_SAFE_ASSERT_RECOVER_RETURN(source->isEditable(false));

    QStringList listMimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);

    KoDialog dlg;
    QWidget *page = new QWidget(&dlg);
    dlg.setMainWidget(page);
    QBoxLayout *layout = new QVBoxLayout(page);
    dlg.setWindowTitle(i18n("Save layers to..."));
    QLabel *lbl = new QLabel(i18n("Choose the location where the layer will be saved to. The new file layer will then reference this location."));
    lbl->setWordWrap(true);
    layout->addWidget(lbl);
    KisFileNameRequester *urlRequester = new KisFileNameRequester(page);
    urlRequester->setMode(KoFileDialog::SaveFile);
    urlRequester->setMimeTypeFilters(listMimeFilter);
    urlRequester->setFileName(m_view->document()->path());
    if (!m_view->document()->path().isEmpty()) {
        QFileInfo location = QFileInfo(m_view->document()->path()).completeBaseName();
        location.setFile(location.dir(), location.completeBaseName() + "_" + source->name() + ".png");
        urlRequester->setFileName(location.absoluteFilePath());
    }
    else {
        const QFileInfo location = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        const QString proposedFileName = QDir(location.absoluteFilePath()).absoluteFilePath(source->name() + ".png");
        urlRequester->setFileName(proposedFileName);
    }

    layout->addWidget(urlRequester);
    if (!dlg.exec()) return;

    QString path = urlRequester->fileName();

    if (path.isEmpty()) return;

    QFileInfo f(path);

    QString mimeType= KisMimeDatabase::mimeTypeForFile(f.fileName());
    if (mimeType.isEmpty()) {
        mimeType = "image/png";
    }
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    QRect bounds = source->exactBounds();
    if (bounds.isEmpty()) {
        bounds = image->bounds();
    }
    KisImageSP dst = new KisImage(doc->createUndoStore(),
                                  image->width(),
                                  image->height(),
                                  image->projection()->compositionSourceColorSpace(),
                                  source->name());
    dst->setResolution(image->xRes(), image->yRes());
    doc->setFileBatchMode(false);
    doc->setCurrentImage(dst);
    KisNodeSP node = source->clone();
    dst->addNode(node);
    dst->initialRefreshGraph();
    dst->cropImage(bounds);
    dst->waitForDone();

    bool r = doc->exportDocumentSync(path, mimeType.toLatin1());
    if (!r) {

        qWarning() << "Converting layer to file layer. path:"<< path << "gave errors" << doc->errorMessage();
    } else {
        QString basePath = QFileInfo(m_view->document()->path()).absolutePath();
        QString relativePath = QDir(basePath).relativeFilePath(path);
        KisFileLayer *fileLayer = new KisFileLayer(image, basePath, relativePath, KisFileLayer::None, source->name(), OPACITY_OPAQUE_U8);
        fileLayer->setX(bounds.x());
        fileLayer->setY(bounds.y());
        KisNodeSP dstParent = source->parent();
        KisNodeSP dstAboveThis = source->prevSibling();
        m_commandsAdapter->beginMacro(kundo2_i18n("Convert to a file layer"));
        m_commandsAdapter->removeNode(source);
        m_commandsAdapter->addNode(fileLayer, dstParent, dstAboveThis);
        m_commandsAdapter->endMacro();
    }
    doc->closePath(false);
}

void KisLayerManager::adjustLayerPosition(KisNodeSP node, KisNodeSP activeNode, KisNodeSP &parent, KisNodeSP &above)
{
    Q_ASSERT(activeNode);

    parent = activeNode;
    above = parent->lastChild();

    if (parent->inherits("KisGroupLayer") && parent->collapsed()) {
        above = parent;
        parent = parent->parent();
        return;
    }

    while (parent &&
           (!parent->allowAsChild(node) || !parent->isEditable(false))) {

        above = parent;
        parent = parent->parent();
    }

    if (!parent) {
        warnKrita << "KisLayerManager::adjustLayerPosition:"
                  << "No node accepted newly created node";

        parent = m_view->image()->root();
        above = parent->lastChild();
    }
}

void KisLayerManager::addLayerCommon(KisNodeSP activeNode, KisNodeSP layer, bool updateImage, KisProcessingApplicator *applicator)
{
    KisNodeSP parent;
    KisNodeSP above;
    adjustLayerPosition(layer, activeNode, parent, above);

    KisGroupLayer *group = dynamic_cast<KisGroupLayer*>(parent.data());
    const bool parentForceUpdate = group && !group->projectionIsValid();
    updateImage |= parentForceUpdate;

    m_commandsAdapter->addNodeAsync(layer, parent, above, updateImage, updateImage, applicator);
}

KisLayerSP KisLayerManager::addPaintLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();    
    KisLayerSP layer = new KisPaintLayer(image.data(),  image->nextLayerName( i18n("Paint Layer") ), OPACITY_OPAQUE_U8, image->colorSpace());

    KisConfig cfg(true);
    layer->setPinnedToTimeline(cfg.autoPinLayersToTimeline());

    addLayerCommon(activeNode, layer, false, 0);

    return layer;
}

KisNodeSP KisLayerManager::addGroupLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();
    KisGroupLayerSP group = new KisGroupLayer(image.data(), image->nextLayerName( i18n("Group") ), OPACITY_OPAQUE_U8);
    addLayerCommon(activeNode, group, false, 0);
    return group;
}

KisNodeSP KisLayerManager::addCloneLayer(KisNodeList nodes)
{
    KisImageWSP image = m_view->image();

    KisNodeList filteredNodes = KisLayerUtils::sortAndFilterMergableInternalNodes(nodes, false);
    if (filteredNodes.isEmpty()) return KisNodeSP();

    KisNodeSP newAbove = filteredNodes.last();

    KisNodeSP node, lastClonedNode;
    Q_FOREACH (node, filteredNodes) {
        lastClonedNode = new KisCloneLayer(qobject_cast<KisLayer*>(node.data()), image.data(), image->nextLayerName( i18n("Clone Layer") ), OPACITY_OPAQUE_U8);
        addLayerCommon(newAbove, lastClonedNode, true, 0 );
    }

    return lastClonedNode;
}

KisNodeSP KisLayerManager::addShapeLayer(KisNodeSP activeNode)
{
    if (!m_view) return 0;
    if (!m_view->document()) return 0;

    KisImageWSP image = m_view->image();
    KisShapeLayerSP layer = new KisShapeLayer(m_view->document()->shapeController(), image.data(), image->nextLayerName(i18n("Vector Layer")), OPACITY_OPAQUE_U8);

    addLayerCommon(activeNode, layer, false, 0);

    return layer;
}

KisNodeSP KisLayerManager::addAdjustmentLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();

    KisSelectionSP selection = m_view->selection();

    KisProcessingApplicator applicator(image, 0, KisProcessingApplicator::NONE,
                                       KisImageSignalVector(),
                                       kundo2_i18n("Add Layer"));


    KisAdjustmentLayerSP adjl = addAdjustmentLayer(activeNode, QString(), 0, selection, &applicator);

    KisPaintDeviceSP previewDevice = new KisPaintDevice(*adjl->original());

    KisDlgAdjustmentLayer dlg(adjl, adjl.data(), previewDevice, image->nextLayerName(i18n("Filter Layer")), i18n("New Filter Layer"), m_view, qApp->activeWindow());
    dlg.resize(dlg.minimumSizeHint());

    // ensure that the device may be free'd by the dialog
    // when it is not needed anymore
    previewDevice = 0;

    if (dlg.exec() != QDialog::Accepted || adjl->filter().isNull()) {
        // XXX: add messagebox warning if there's no filter set!
        applicator.cancel();
    } else {
        adjl->setName(dlg.layerName());
        applicator.end();
    }

    return adjl;
}

KisAdjustmentLayerSP KisLayerManager::addAdjustmentLayer(KisNodeSP activeNode, const QString & name,
                                                         KisFilterConfigurationSP  filter,
                                                         KisSelectionSP selection,
                                                         KisProcessingApplicator *applicator)
{
    KisImageWSP image = m_view->image();
    KisAdjustmentLayerSP layer = new KisAdjustmentLayer(image, name, filter ? filter->cloneWithResourcesSnapshot() : 0, selection);
    addLayerCommon(activeNode, layer, true, applicator);

    return layer;
}

KisGeneratorLayerSP KisLayerManager::addGeneratorLayer(KisNodeSP activeNode, const QString &name, KisFilterConfigurationSP filter, KisSelectionSP selection, KisProcessingApplicator *applicator)
{
    KisImageWSP image = m_view->image();
    auto layer = new KisGeneratorLayer(image, name, filter, selection);
    addLayerCommon(activeNode, layer, true, applicator);

    return layer;
}

KisNodeSP KisLayerManager::addGeneratorLayer(KisNodeSP activeNode)
{
    KisImageWSP image = m_view->image();
    KisSelectionSP selection = m_view->selection();
    QColor currentForeground = m_view->canvasResourceProvider()->fgColor().toQColor();

    KisProcessingApplicator applicator(image, 0, KisProcessingApplicator::NONE, KisImageSignalVector(), kundo2_i18n("Add Layer"));

    KisGeneratorLayerSP node = addGeneratorLayer(activeNode, QString(), nullptr, selection, &applicator);

    KisDlgGeneratorLayer dlg(image->nextLayerName(i18n("Fill Layer")), m_view, m_view->mainWindow(), node, nullptr, applicator.getStroke());
    KisFilterConfigurationSP defaultConfig = dlg.configuration();
    defaultConfig->setProperty("color", currentForeground);
    dlg.setConfiguration(defaultConfig);

    if (dlg.exec() == QDialog::Accepted) {
        node->setName(dlg.layerName());
        applicator.end();
        return node;
    }
    else {
        applicator.cancel();
        return nullptr;
    }
}

void KisLayerManager::flattenImage()
{
    KisImageSP image = m_view->image();

    if (!m_view->blockUntilOperationsFinished(image)) return;

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
            image->flatten(m_view->activeNode());
        }
    }
}

inline bool isSelectionMask(KisNodeSP node) {
    return dynamic_cast<KisSelectionMask*>(node.data());
}

bool tryMergeSelectionMasks(KisNodeSP currentNode, KisImageSP image)
{
    bool result = false;

    KisNodeSP prevNode = currentNode->prevSibling();
    if (isSelectionMask(currentNode) &&
            prevNode && isSelectionMask(prevNode)) {

        QList<KisNodeSP> mergedNodes;
        mergedNodes.append(currentNode);
        mergedNodes.append(prevNode);

        image->mergeMultipleLayers(mergedNodes, currentNode);

        result = true;
    }

    return result;
}

bool tryFlattenGroupLayer(KisNodeSP currentNode, KisImageSP image)
{
    bool result = false;

    if (currentNode->inherits("KisGroupLayer")) {
        KisGroupLayer *layer = qobject_cast<KisGroupLayer*>(currentNode.data());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(layer, false);

        image->flattenLayer(layer);
        result = true;
    }

    return result;
}

void KisLayerManager::mergeLayer()
{
    KisImageSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    if (!m_view->blockUntilOperationsFinished(image)) return;

    QList<KisNodeSP> selectedNodes = m_view->nodeManager()->selectedNodes();

    // check if all the layers are a part of a locked group
    bool hasEditableLayer = false;
    Q_FOREACH (KisNodeSP node, selectedNodes) {
        if (node->isEditable(false)) {
            hasEditableLayer = true;
            break;
        }
    }

    if (!hasEditableLayer) {
        m_view->showFloatingMessage(
                    i18ncp("floating message in layer manager",
                          "Layer is locked", "Layers are locked", selectedNodes.size()),
                    QIcon(), 2000, KisFloatingMessage::Low);
        return;
    }

    if (selectedNodes.size() > 1) {
        image->mergeMultipleLayers(selectedNodes, m_view->activeNode());
    }

    else if (tryMergeSelectionMasks(m_view->activeNode(), image)) {
        // already done!
    } else if (tryFlattenGroupLayer(m_view->activeNode(), image)) {
        // already done!
    } else {

        if (!layer->prevSibling()) return;
        KisLayer *prevLayer = qobject_cast<KisLayer*>(layer->prevSibling().data());
        if (!prevLayer) return;
        if (prevLayer->userLocked()) {
            m_view->showFloatingMessage(
                        i18nc("floating message in layer manager when previous layer is locked",
                              "Layer is locked"),
                        QIcon(), 2000, KisFloatingMessage::Low);
        }

        else if (layer->metaData()->isEmpty() && prevLayer->metaData()->isEmpty()) {
            image->mergeDown(layer, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        }
        else {
            const KisMetaData::MergeStrategy* strategy = KisMetaDataMergeStrategyChooserWidget::showDialog(m_view->mainWindow());
            if (!strategy) return;
            image->mergeDown(layer, strategy);
        }
    }

    m_view->updateGUI();
}

void KisLayerManager::flattenLayer()
{
    KisImageSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    if (!m_view->blockUntilOperationsFinished(image)) return;
    if (!m_view->nodeManager()->canModifyLayer(layer)) return;

    convertNodeToPaintLayer(layer);
    m_view->updateGUI();
}

void KisLayerManager::layersUpdated()
{
    KisLayerSP layer = activeLayer();
    if (!layer) return;

    m_view->updateGUI();
}

void KisLayerManager::saveGroupLayers()
{
    QStringList listMimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);

    KoDialog dlg;
    QWidget *page = new QWidget(&dlg);
    dlg.setMainWidget(page);
    QBoxLayout *layout = new QVBoxLayout(page);

    KisFileNameRequester *urlRequester = new KisFileNameRequester(page);
    urlRequester->setMode(KoFileDialog::SaveFile);
    urlRequester->setStartDir(QFileInfo(m_view->document()->path()).absolutePath());
    urlRequester->setMimeTypeFilters(listMimeFilter);
    urlRequester->setFileName(m_view->document()->path());
    layout->addWidget(urlRequester);

    QCheckBox *chkInvisible = new QCheckBox(i18n("Convert Invisible Groups"), page);
    chkInvisible->setChecked(false);
    layout->addWidget(chkInvisible);
    QCheckBox *chkDepth = new QCheckBox(i18n("Export Only Toplevel Groups"), page);
    chkDepth->setChecked(true);
    layout->addWidget(chkDepth);

    if (!dlg.exec()) return;

    QString path = urlRequester->fileName();

    if (path.isEmpty()) return;

    QFileInfo f(path);

    QString mimeType= KisMimeDatabase::mimeTypeForFile(f.fileName(), false);
    if (mimeType.isEmpty()) {
        mimeType = "image/png";
    }
    QString extension = KisMimeDatabase::suffixesForMimeType(mimeType).first();
    QString basename = f.completeBaseName();

    KisImageSP image = m_view->image();
    if (!image) return;

    KisSaveGroupVisitor v(image, chkInvisible->isChecked(), chkDepth->isChecked(), f.absolutePath(), basename, extension, mimeType);
    image->rootLayer()->accept(v);

}

bool KisLayerManager::activeLayerHasSelection()
{
    return (activeLayer()->selection() != 0);
}

KisNodeSP KisLayerManager::addFileLayer(KisNodeSP activeNode)
{
    QString basePath;
    QString path = m_view->document()->path();
    basePath = QFileInfo(path).absolutePath();
    KisImageWSP image = m_view->image();

    KisDlgFileLayer dlg(basePath, image->nextLayerName(i18n("File Layer")), m_view->mainWindow());
    dlg.resize(dlg.minimumSizeHint());

    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.layerName();
        QString fileName = dlg.fileName();

        if(fileName.isEmpty()){
            QMessageBox::critical(m_view->mainWindow(), i18nc("@title:window", "Krita"), i18n("No file name specified"));
            return 0;
        }

        KisFileLayer::ScalingMethod scalingMethod = dlg.scaleToImageResolution();
        KisNodeSP node = new KisFileLayer(image, basePath, fileName, scalingMethod, name, OPACITY_OPAQUE_U8);
        addLayerCommon(activeNode, node, true, 0);
        return node;
    }
    return 0;
}

void updateLayerStyles(KisLayerSP layer, KisDlgLayerStyle *dlg)
{
    KisSetLayerStyleCommand::updateLayerStyle(layer, dlg->style()->cloneWithResourcesSnapshot());
}

void KisLayerManager::layerStyle()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = activeLayer();
    if (!layer) return;

    if (!m_view->blockUntilOperationsFinished(image)) return;
    if (!m_view->nodeManager()->canModifyLayer(layer)) return;

    KisPSDLayerStyleSP oldStyle;
    if (layer->layerStyle()) {
        oldStyle = layer->layerStyle()->clone().dynamicCast<KisPSDLayerStyle>();

    } else {
        oldStyle = toQShared(new KisPSDLayerStyle("", KisGlobalResourcesInterface::instance()));
    }

    KisPSDLayerStyleSP newStyle = oldStyle->clone().dynamicCast<KisPSDLayerStyle>();
    newStyle->setResourcesInterface(KisGlobalResourcesInterface::instance());

    KisDlgLayerStyle dlg(newStyle, m_view->canvasResourceProvider());

    std::function<void ()> updateCall(std::bind(updateLayerStyles, layer, &dlg));
    SignalToFunctionProxy proxy(updateCall);
    connect(&dlg, SIGNAL(configChanged()), &proxy, SLOT(start()));

    if (dlg.exec() == QDialog::Accepted) {
        KisPSDLayerStyleSP newStyle = dlg.style()->cloneWithResourcesSnapshot();

        KUndo2CommandSP command = toQShared(
                    new KisSetLayerStyleCommand(layer, oldStyle, newStyle));

        image->postExecutionUndoAdapter()->addCommand(command);
    }
}

