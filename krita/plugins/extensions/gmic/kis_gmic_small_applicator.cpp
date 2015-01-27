/*
 * Copyright (c) 2014 Lukáš Tvrdý <lukast.dev@gmail.com
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

#include <QRect>
#include "kis_gmic_small_applicator.h"

#include "kis_gmic_command.h"
#include "kis_import_gmic_processing_visitor.h"
#include <kis_paint_layer.h>
#include <KoColorSpaceConstants.h>

#include "kis_gmic_filter_settings.h"
#include "kis_gmic_synchronize_layers_command.h"
#include "kis_export_gmic_processing_visitor.h"

KisGmicSmallApplicator::KisGmicSmallApplicator(QObject *parent)
    : QThread(parent),
      m_setting(0),
      m_progress(0)
{
}

KisGmicSmallApplicator::~KisGmicSmallApplicator()
{
    wait();
    qDebug() << "Destroying KisGmicSmallApplicator : " << this;
}

void KisGmicSmallApplicator::setProperties(const QRect& canvasRect, const QSize& previewSize, KisNodeListSP layers, KisGmicFilterSetting* settings, const QByteArray& customCommands)
{
    m_canvasRect = canvasRect;
    m_previewSize = previewSize;
    m_layers = layers;
    m_setting = settings;
    m_gmicCustomCommands = customCommands;
}


void KisGmicSmallApplicator::run()
{
    qreal aspectRatio = (qreal)m_canvasRect.width() / m_canvasRect.height();

    int previewWidth = m_previewSize.width();
    int previewHeight = qRound(previewWidth / aspectRatio);
    QRect previewRect = QRect(QPoint(0,0), QSize(previewWidth, previewHeight));

    KisNodeListSP previewKritaNodes = createPreviewThumbnails(m_layers, previewRect.size(), m_canvasRect);

    QSharedPointer< gmic_list<float> > gmicLayers(new gmic_list<float>);
    gmicLayers->assign(previewKritaNodes->size());

    KisExportGmicProcessingVisitor exportVisitor(previewKritaNodes, gmicLayers, previewRect);
    for (int i = 0; i < previewKritaNodes->size(); i++)
    {
        exportVisitor.visit( (KisPaintLayer *)(*previewKritaNodes)[i].data(), 0);
    }

    QString gmicCommand = m_setting->previewGmicCommand();
    if (gmicCommand.isEmpty())
    {
        gmicCommand = m_setting->gmicCommand();
    }

    KisGmicCommand gmicCmd(gmicCommand, gmicLayers, m_gmicCustomCommands);
    connect(&gmicCmd, SIGNAL(gmicFinished(bool, int, QString)), this, SIGNAL(gmicFinished(bool, int, QString)));
    m_progress = gmicCmd.progressPtr();

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
        m_preview = previewKritaNodes->at(0)->paintDevice();
        emit previewReady();
    }
    else
    {
        // TODO: show error preview
    }
}

KisNodeListSP KisGmicSmallApplicator::createPreviewThumbnails(KisNodeListSP layers,const QSize &dstSize,const QRect &srcRect)
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

float KisGmicSmallApplicator::getProgress() const
{
    return (m_progress == 0) ? -2.0f : *m_progress;
}

KisPaintDeviceSP KisGmicSmallApplicator::preview()
{
    return m_preview;
}
