/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2013-2014 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_gmic_plugin.h"

#include <QApplication>
#include <QTimer>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kglobal.h>

#include <QMessageBox>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <QTime>

#include <KisViewManager.h>
#include <kis_action.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_paint_layer.h>
#include "kis_statusbar.h"
#include "widgets/kis_progress_widget.h"


#include "kis_gmic_parser.h"
#include "Component.h"
#include "kis_gmic_filter_model.h"
#include "kis_gmic_widget.h"

#include "kis_gmic_blacklister.h"
#include "kis_input_output_mapper.h"
#include "kis_gmic_simple_convertor.h"
#include "kis_gmic_applicator.h"
#include "kis_export_gmic_processing_visitor.h"
#include "kis_gmic_command.h"
#include "kis_import_gmic_processing_visitor.h"
#include "kis_gmic_synchronize_layers_command.h"

#include "KoColorSpaceConstants.h"
#include "kis_gmic_progress_manager.h"

#include <kis_processing_visitor.h>
#include "gmic.h"

K_PLUGIN_FACTORY(KisGmicPluginFactory, registerPlugin<KisGmicPlugin>();)
K_EXPORT_PLUGIN(KisGmicPluginFactory("krita"))

const QString STANDARD_GMIC_DEFINITION = "gmic_def.gmic";

KisGmicPlugin::KisGmicPlugin(QObject *parent, const QVariantList &)
    :   KisViewPlugin(parent),
      m_gmicWidget(0),
      m_requestFinishAndClose(false)
{
    KisAction *action  = new KisAction(i18n("G'MIC"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    connect(action, SIGNAL(triggered()), this, SLOT(slotShowGmicDialog()));
    addAction("gmic", action);

    KGlobal::dirs()->addResourceType("gmic_definitions", "data", "krita/gmic/");
    m_blacklistPath = KGlobal::mainComponent().dirs()->findResource("gmic_definitions", STANDARD_GMIC_DEFINITION + ".blacklist");

}

KisGmicPlugin::~KisGmicPlugin()
{
    delete m_gmicWidget;
}

void KisGmicPlugin::setupDefinitionPaths()
{
    m_definitionFilePaths = KGlobal::dirs()->findAllResources("gmic_definitions", "*.gmic");
    QMutableStringListIterator it(m_definitionFilePaths);
    // remove all instances of gmic_def.gmic
    while (it.hasNext())
    {
        if ( it.next().endsWith(STANDARD_GMIC_DEFINITION) )
        {
            it.remove();
        }
    }

    // if we don't have updateXXXX.gmic, prepend standard gmic_def.gmic
    int gmicVersion = gmic_version;
    QString updateFileName = "update" + QString::number(gmicVersion) + ".gmic";
    QString updatedGmicDefinitionFilePath = KGlobal::mainComponent().dirs()->findResource("gmic_definitions", updateFileName);
    if (updatedGmicDefinitionFilePath.isEmpty())
    {
        QString standardGmicDefinitionFilePath = KGlobal::mainComponent().dirs()->findResource("gmic_definitions", STANDARD_GMIC_DEFINITION);
        m_definitionFilePaths.prepend(standardGmicDefinitionFilePath);
    }

    dbgPlugins << m_definitionFilePaths;
}

void KisGmicPlugin::slotShowGmicDialog()
{
    if (m_gmicWidget)
    {
        // restart here?
        slotCloseGmicDialog();
    }

    // init everything here
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    m_progressManager = new KisGmicProgressManager(m_view);
    connect(m_progressManager, SIGNAL(sigProgress()), this, SLOT(slotUpdateProgress()));

    m_gmicApplicator = new KisGmicApplicator();
    connect(m_gmicApplicator, SIGNAL(gmicFinished(int)), this, SLOT(slotGmicFinished(int)));
    connect(m_gmicApplicator, SIGNAL(gmicFailed(const QString&)), this, SLOT(slotGmicFailed(const QString&)));

    setupDefinitionPaths();
    parseGmicCommandDefinitions(m_definitionFilePaths);

    KisGmicParser parser(m_definitionFilePaths);
    Component * root = parser.createFilterTree();
    KisGmicFilterModel * model = new KisGmicFilterModel(root); // filter mode takes owner ship

    KisGmicBlacklister * blacklister = new KisGmicBlacklister(m_blacklistPath);
    model->setBlacklister(blacklister);

    QString updateUrl = "http://gmic.sourceforge.net/" + QString("update") + QString::number(gmic_version) + ".gmic";
    m_gmicWidget = new KisGmicWidget(model, updateUrl);

    // preview filter
    connect(m_gmicWidget, SIGNAL(sigPreviewFilterCommand(KisGmicFilterSetting*)),this, SLOT(slotPreviewGmicCommand(KisGmicFilterSetting*)));
    // finish filter
    connect(m_gmicWidget, SIGNAL(sigAcceptOnCanvasPreview()), this, SLOT(slotAcceptOnCanvasPreview()));
    // cancel filter
    connect(m_gmicWidget, SIGNAL(sigCancelOnCanvasPreview()), this, SLOT(slotCancelOnCanvasPreview()));
    // cancel
    connect(m_gmicWidget, SIGNAL(sigClose()),this, SLOT(slotCloseGmicDialog()));
    // filter current image with current settings
    connect(m_gmicWidget, SIGNAL(sigFilterCurrentImage(KisGmicFilterSetting*)), this, SLOT(slotFilterCurrentImage(KisGmicFilterSetting*)));
    // show active layer in preview viewport
    connect(m_gmicWidget, SIGNAL(sigPreviewActiveLayer()), this, SLOT(slotPreviewActiveLayer()));

    connect(m_gmicWidget, SIGNAL(sigRequestFinishAndClose()), this, SLOT(slotRequestFinishAndClose()));

    QString version = QString("%0.%1.%2.%3").arg(gmic_version/1000).arg((gmic_version/100)%10).arg((gmic_version/10)%10).arg(gmic_version%10);
    QString pluginName = i18n("G'MIC for Krita");
    m_gmicWidget->setWindowTitle(QString("%0 %1").arg(pluginName).arg(version));
    m_gmicWidget->show();
    slotPreviewActiveLayer();
}

void KisGmicPlugin::slotCloseGmicDialog()
{
    m_gmicWidget = 0;
    if (m_gmicApplicator)
    {
        m_gmicApplicator->cancel();
    }

    delete m_gmicApplicator;
    m_gmicApplicator = 0;

    delete m_progressManager;
}

void KisGmicPlugin::slotPreviewGmicCommand(KisGmicFilterSetting* setting)
{
    dbgPlugins << "Preview Request, type : " << setting->previewSize();
    KisInputOutputMapper mapper(m_view->image(), m_view->activeNode());
    KisNodeListSP layers = mapper.inputNodes(setting->inputLayerMode());
    if (!checkSettingsValidity(layers, setting))
    {
        dbgPlugins << "Failed, some feature not implemented";
        return;
    }

    if (setting->previewSize() !=  ON_CANVAS)
    {
        createViewportPreview(layers, setting);
    }
    else
    {
        startOnCanvasPreview(layers, setting, PREVIEWING);
    }
}


void KisGmicPlugin::slotPreviewActiveLayer()
{
    showInPreviewViewport(m_view->activeNode()->paintDevice());
}


void KisGmicPlugin::showInPreviewViewport(KisPaintDeviceSP device)
{
    QRect deviceRect = device->exactBounds();
    qreal aspectRatio = (qreal)deviceRect.width()/deviceRect.height();

    int dstWidth = m_gmicWidget->previewWidget()->size().width();
    int dstHeight = dstWidth / aspectRatio;

    QImage previewImage = device->createThumbnail(dstWidth, dstHeight, deviceRect);
    m_gmicWidget->previewWidget()->setImage(previewImage);
}


void KisGmicPlugin::slotAcceptOnCanvasPreview()
{
    m_gmicApplicator->finish();
}

void KisGmicPlugin::slotCancelOnCanvasPreview()
{
    m_gmicApplicator->cancel();
}

void KisGmicPlugin::slotFilterCurrentImage(KisGmicFilterSetting* setting)
{
    dbgPlugins << "Filtering image on canvas!";

    KisInputOutputMapper mapper(m_view->image(), m_view->activeNode());
    KisNodeListSP layers = mapper.inputNodes(setting->inputLayerMode());
    if (checkSettingsValidity(layers, setting))
    {
        startOnCanvasPreview(layers, setting, FILTERING);
        // wait, so that next request to strokes for on-canvas preview is possible
        // m_view->image()->waitForDone();
    }
    else
    {
        dbgPlugins << "Failed to filter image, some feature not implemented";
    }
}

void KisGmicPlugin::parseGmicCommandDefinitions(const QStringList& gmicDefinitionFilePaths)
{
    foreach (const QString filePath, gmicDefinitionFilePaths)
    {
        QByteArray gmicCommands = KisGmicParser::extractGmicCommandsOnly(filePath);
        m_gmicCustomCommands.append(gmicCommands);
    }
}

KisNodeListSP KisGmicPlugin::createPreviewThumbnails(KisNodeListSP layers,const QSize &dstSize,const QRect &srcRect)
{
    KisNodeListSP previewKritaNodes(new QList< KisNodeSP >());
    for (int i = 0; i < layers->size(); i++)
    {
        KisPaintDeviceSP thumbnail = layers->at(i)->paintDevice()->createThumbnailDevice(dstSize.width(), dstSize.height(), srcRect);
        KisNodeSP node(new KisPaintLayer(0, "", OPACITY_OPAQUE_U8, thumbnail));
        previewKritaNodes->append(node);
    }
    return previewKritaNodes;
}


void KisGmicPlugin::createViewportPreview(KisNodeListSP layers, KisGmicFilterSetting* setting)
{
    QRect canvasRect = m_view->image()->bounds();
    qreal aspectRatio = (qreal)canvasRect.width() / canvasRect.height();

    int previewWidth = m_gmicWidget->previewWidget()->size().width();
    int previewHeight = qRound(previewWidth / aspectRatio);
    QRect previewRect = QRect(QPoint(0,0), QSize(previewWidth, previewHeight));

    KisNodeListSP previewKritaNodes = KisGmicPlugin::createPreviewThumbnails(layers, previewRect.size(), canvasRect);

    QSharedPointer< gmic_list<float> > gmicLayers(new gmic_list<float>);
    gmicLayers->assign(previewKritaNodes->size());

    KisExportGmicProcessingVisitor exportVisitor(previewKritaNodes, gmicLayers, previewRect);
    for (int i = 0; i < previewKritaNodes->size(); i++)
    {
        exportVisitor.visit( (KisPaintLayer *)(*previewKritaNodes)[i].data(), 0);
    }

    QString gmicCommand = setting->previewGmicCommand();
    if (gmicCommand.isEmpty())
    {
        gmicCommand = setting->gmicCommand();
    }

    KisGmicCommand gmicCmd(gmicCommand, gmicLayers, m_gmicCustomCommands);
    gmicCmd.redo();

    KisGmicSynchronizeLayersCommand syncCmd(previewKritaNodes, gmicLayers, 0);
    syncCmd.redo();

    KisImportGmicProcessingVisitor importVisitor(previewKritaNodes, gmicLayers, previewRect, 0);
    for (int i = 0; i < previewKritaNodes->size(); i++)
    {
        importVisitor.visit( (KisPaintLayer *)(*previewKritaNodes)[i].data(), 0 );
    }

    if (previewKritaNodes->size() > 0)
    {
        showInPreviewViewport(previewKritaNodes->at(0)->paintDevice());
    }
    else
    {
        // TODO: show error preview
    }
}


void KisGmicPlugin::startOnCanvasPreview(KisNodeListSP layers, KisGmicFilterSetting* setting, Activity activity)
{
    KUndo2MagicString actionName = kundo2_i18n("Gmic filter");
    KisNodeSP rootNode = m_view->image()->root();
    m_gmicApplicator->setProperties(m_view->image(), rootNode, actionName, layers, setting->gmicCommand(), m_gmicCustomCommands);
    m_gmicApplicator->preview();
    // Note: do not call KisImage::waitForDone(): strokes are not finished or cancelled, it's just preview!
    // waitForDone would cause infinite hang
    m_currentActivity = activity;
    m_progressManager->initProgress();
}


bool KisGmicPlugin::checkSettingsValidity(KisNodeListSP layers, const KisGmicFilterSetting* setting)
{
    if (setting->isBlacklisted())
    {
        QMessageBox::warning(m_gmicWidget, i18nc("@title:window", "Krita"), i18n("Sorry, this filter is crashing Krita and is turned off."));
        return false;
    }

    if (setting->outputMode() != IN_PLACE) {
        QMessageBox::warning(m_gmicWidget, i18nc("@title:window", "Krita"), i18n("Sorry, this output mode is not implemented"));
        return false;
    }

    if (layers->isEmpty()) {
        QMessageBox::warning(m_gmicWidget, i18nc("@title:window", "Krita"), i18n("Sorry, this input mode is not implemented"));
        return false;
    }

    return true;
}



void KisGmicPlugin::slotUpdateProgress()
{
    float progress = m_gmicApplicator->getProgress();
    m_progressManager->updateProgress(progress);
}

void KisGmicPlugin::slotGmicFinished(int miliseconds)
{
    if (m_currentActivity == FILTERING)
    {
        slotAcceptOnCanvasPreview();
    }

    double seconds = miliseconds * 0.001;
    m_gmicWidget->setWindowTitle(QString("Filtering took ") + QString::number(seconds) + QString(" seconds"));

    m_progressManager->finishProgress();

    if (m_requestFinishAndClose)
    {
        slotRequestFinishAndClose();
    }
}

void KisGmicPlugin::slotGmicFailed(const QString& msg)
{
    dbgPlugins << msg;
    slotCancelOnCanvasPreview();
    m_progressManager->finishProgress();

    QMessageBox::warning(m_gmicWidget, i18nc("@title:window", "Krita"), i18n("Sorry! G'Mic failed, reason:") + msg);

    if (m_requestFinishAndClose)
    {
        slotRequestFinishAndClose();
    }
}

void KisGmicPlugin::slotRequestFinishAndClose()
{
    if (m_progressManager->inProgress())
    {
        m_requestFinishAndClose = true;
    }
    else
    {
        m_gmicWidget->close();
    }
}


#include "kis_gmic_plugin.moc"

