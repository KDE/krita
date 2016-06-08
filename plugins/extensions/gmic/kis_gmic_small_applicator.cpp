/*
 * Copyright (c) 2014-2015 Lukáš Tvrdý <lukast.dev@gmail.com
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
      m_setting(0)
{
    m_gmicData = KisGmicDataSP(new KisGmicData());
    m_abort = false;
    m_restart = false;
}

KisGmicSmallApplicator::~KisGmicSmallApplicator()
{
    m_mutex.lock();
    m_abort = true;
    m_condition.wakeOne();
    m_mutex.unlock();
    wait();

    dbgPlugins << "Destroying KisGmicSmallApplicator: " << this;
}

void KisGmicSmallApplicator::render(const QRect& canvasRect, const QSize& previewSize, KisNodeListSP layers, KisGmicFilterSetting* settings, const QByteArray& customCommands)
{
    QMutexLocker locker(&m_mutex);

    m_canvasRect = canvasRect;
    m_previewSize = previewSize;
    m_layers = layers;
    m_setting = settings;
    m_gmicCustomCommands = customCommands;

    dbgPlugins << "Rendering " << m_setting->gmicCommand();

    if (!isRunning())
    {
        start();
    }
    else
    {
        m_restart = true;
        m_condition.wakeOne();
    }
}



void KisGmicSmallApplicator::run()
{
    forever
    {
        if (m_abort)
        {
            return;
        }

        m_mutex.lock();
        qreal aspectRatio = (qreal)m_canvasRect.width() / m_canvasRect.height();
        int previewWidth = m_previewSize.width();
        int previewHeight = qRound(previewWidth / aspectRatio);
        QRect previewRect = QRect(QPoint(0,0), QSize(previewWidth, previewHeight));
        KisNodeListSP previewKritaNodes = createPreviewThumbnails(m_layers, previewRect.size(), m_canvasRect);
        QString gmicCommand = m_setting->previewGmicCommand();
        if (gmicCommand.isEmpty())
        {
            gmicCommand = m_setting->gmicCommand();
        }

        QByteArray gmicCustomCommands = m_gmicCustomCommands;
        m_mutex.unlock();


        while(true)
        {
            if (m_abort)
            {
                return;
            }

            QSharedPointer< gmic_list<float> > gmicLayers(new gmic_list<float>);
            gmicLayers->assign(previewKritaNodes->size());

            KisExportGmicProcessingVisitor exportVisitor(previewKritaNodes, gmicLayers, previewRect);
            for (int i = 0; i < previewKritaNodes->size(); i++)
            {
                exportVisitor.visit( (KisPaintLayer *)(*previewKritaNodes)[i].data(), 0);
            }

            if (m_restart)
            {
                dbgPlugins << "Restarting at " << __LINE__;
                break;
            }

            m_mutex.lock();
            m_gmicData->reset();
            m_mutex.unlock();

            KisGmicCommand gmicCmd(gmicCommand, gmicLayers, m_gmicData, gmicCustomCommands);
            connect(&gmicCmd, SIGNAL(gmicFinished(bool, int, QString)), this, SIGNAL(gmicFinished(bool, int, QString)));

            gmicCmd.redo();
            if (!gmicCmd.isSuccessfullyDone())
            {
                dbgPlugins << "G'MIC command for small preview failed!";
                break;
            }

            if (m_restart)
            {
                dbgPlugins << "Restarting at " << __LINE__;
                break;
            }

            KisGmicSynchronizeLayersCommand syncCmd(previewKritaNodes, gmicLayers, 0);
            syncCmd.redo();

            if (m_restart)
            {
                dbgPlugins << "Restarting at " << __LINE__;
                break;
            }

            KisImportGmicProcessingVisitor importVisitor(previewKritaNodes, gmicLayers, previewRect, 0);
            for (int i = 0; i < previewKritaNodes->size(); i++)
            {
                importVisitor.visit( (KisPaintLayer *)(*previewKritaNodes)[i].data(), 0 );
            }

            if (m_restart)
            {
                dbgPlugins << "Restarting at " << __LINE__;
                break;
            }

            if (previewKritaNodes->size() > 0)
            {
                m_mutex.lock();
                m_preview = previewKritaNodes->at(0)->paintDevice();
                m_mutex.unlock();

                dbgPlugins << "Emiting previewFinished(true)";
                emit previewFinished(true);
            }
            else
            {
                dbgPlugins << "Emiting previewFinished(false)";
                emit previewFinished(false);
            }

            break;
        }

        m_mutex.lock();
        if (!m_restart)
        {
            m_condition.wait(&m_mutex);
        }
        m_restart = false;
        m_mutex.unlock();
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

float KisGmicSmallApplicator::progress() const
{
    return m_gmicData->progress();
}

KisPaintDeviceSP KisGmicSmallApplicator::preview()
{   QMutexLocker locker(&m_mutex);
    return m_preview;
}
