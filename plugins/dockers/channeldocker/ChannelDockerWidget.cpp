/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ChannelDockerWidget.h"

#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>

#include "channelmodel.h"
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include "KisChannelsThumbnailsStrokeStrategy.h"
#include <kis_display_color_converter.h>
#include <KisDisplayConfig.h>

ChannelDockerWidget::ChannelDockerWidget(QWidget *parent, const char *name)
    : KisWidgetWithIdleTask<QWidget>(parent)
{
    setObjectName(name);

    m_channelTable = new QTableView();
    m_model = new ChannelModel(this);
    m_channelTable->setModel(m_model);
    m_channelTable->setShowGrid(false);
    m_channelTable->horizontalHeader()->setStretchLastSection(true);
    m_channelTable->verticalHeader()->setVisible(false);
    m_channelTable->horizontalHeader()->setVisible(false);
    m_channelTable->setSelectionBehavior( QAbstractItemView::SelectRows );

    m_channelTable->setMinimumHeight(50);

    // Use a layout so the widget gets resized properly
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(m_channelTable);
    this->setLayout(layout);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(m_channelTable);
    if (scroller){
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    connect(m_channelTable,&QTableView::activated, m_model, &ChannelModel::rowActivated);
}

void ChannelDockerWidget::setCanvas(KisCanvas2 * canvas)
{
    if(m_canvas == canvas)
        return;
    m_canvas = canvas;

    m_model->setCanvas(m_canvas);

    KisWidgetWithIdleTask<QWidget>::setCanvas(canvas);

    if (m_canvas) {
        connect(m_canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()), SLOT(startUpdateCanvasProjection()));
        connect(m_model, SIGNAL(channelFlagsChanged()), m_canvas, SLOT(channelSelectionChanged()));
    }
}

void ChannelDockerWidget::startUpdateCanvasProjection()
{
    triggerCacheUpdate();
}

KisIdleTasksManager::TaskGuard ChannelDockerWidget::registerIdleTask(KisCanvas2 *canvas)
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

void ChannelDockerWidget::updateImageThumbnails(const QVector<QImage> &channels, const KoColorSpace *cs)
{
    m_model->setChannelThumbnails(channels, cs);
    m_channelTable->resizeRowsToContents();
    m_channelTable->resizeColumnsToContents();
}

void ChannelDockerWidget::clearCachedState()
{
    m_model->setChannelThumbnails({}, nullptr);
}
