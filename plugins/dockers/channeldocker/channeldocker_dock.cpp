/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "channeldocker_dock.h"

#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <klocalizedstring.h>

#include <KoCanvasBase.h>
#include "channelmodel.h"
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_layer.h>
#include <kis_node_manager.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <KisView.h>
#include <kis_idle_watcher.h>
#include "KisChannelsThumbnailsStrokeStrategy.h"
#include <kis_display_color_converter.h>
#include <KisDisplayConfig.h>

ChannelDockerDock::ChannelDockerDock()
{
    setWindowTitle(i18nc("Channel as in Color Channels", "Channels"));

    m_channelTable = new QTableView(this);
    m_model = new ChannelModel(this);
    m_channelTable->setModel(m_model);
    m_channelTable->setShowGrid(false);
    m_channelTable->horizontalHeader()->setStretchLastSection(true);
    m_channelTable->verticalHeader()->setVisible(false);
    m_channelTable->horizontalHeader()->setVisible(false);
    m_channelTable->setSelectionBehavior( QAbstractItemView::SelectRows );

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(m_channelTable);
    if (scroller){
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    setWidget(m_channelTable);

    connect(m_channelTable,&QTableView::activated, m_model, &ChannelModel::rowActivated);
}

void ChannelDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    KisCanvas2 *newCanvas = canvas ? dynamic_cast<KisCanvas2*>(canvas) : nullptr;

    KisWidgetWithIdleTask<QDockWidget>::setCanvas(newCanvas);
    m_model->setCanvas(newCanvas);

    if (m_canvas) {
        connect(m_canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()), SLOT(startUpdateCanvasProjection()));
        connect(m_model, SIGNAL(channelFlagsChanged()), m_canvas, SLOT(channelSelectionChanged()));
    }

    setEnabled(bool(canvas));
}

void ChannelDockerDock::unsetCanvas()
{
    setCanvas(0);
}

void ChannelDockerDock::startUpdateCanvasProjection()
{
    triggerCacheUpdate();
}

KisIdleTasksManager::TaskGuard ChannelDockerDock::registerIdleTask(KisCanvas2 *canvas)
{
    return
        canvas->viewManager()->idleTasksManager()->
        addIdleTaskWithGuard([this](KisImageSP image) {
            const KisDisplayConfig config = m_canvas->displayColorConverter()->displayConfig();
            const QSize previewSize = m_model->thumbnailSizeLimit();

            KisChannelsThumbnailsStrokeStrategy *strategy =
                new KisChannelsThumbnailsStrokeStrategy(image->projection(), image->bounds(),
                                                        previewSize, false, config.profile,
                                                        config.intent, config.conversionFlags);

            connect(strategy, SIGNAL(thumbnailsUpdated(QVector<QImage>, const KoColorSpace*)), this, SLOT(updateImageThumbnails(QVector<QImage>, const KoColorSpace*)));

            return strategy;
        });
}

void ChannelDockerDock::updateImageThumbnails(const QVector<QImage> &channels, const KoColorSpace *cs)
{
    m_model->setChannelThumbnails(channels, cs);
    m_channelTable->resizeRowsToContents();
    m_channelTable->resizeColumnsToContents();
}

void ChannelDockerDock::clearCachedState()
{
    m_model->setChannelThumbnails({}, nullptr);
}




