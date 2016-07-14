/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#include <QTreeView>
#include <QSplitter>

#include "kis_animation_curve_docker.h"
#include "kis_animation_curves_model.h"
#include "kis_animation_curves_view.h"
#include "kis_animation_curve_channel_list_model.h"
#include "kis_animation_curve_channel_list_delegate.h"

#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_shape_controller.h"
#include "kis_signal_auto_connection.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_animation_frame_cache.h"
#include "klocalizedstring.h"

struct KisAnimationCurveDocker::Private
{
    Private(QWidget *parent)
        : curvesModel(new KisAnimationCurvesModel(parent))
        , curvesView(new KisAnimationCurvesView(parent))
        , channelListView(new QTreeView(parent))
    {
        channelListModel = new KisAnimationCurveChannelListModel(curvesModel, parent);
        curvesView->setModel(curvesModel);
        channelListView->setModel(channelListModel);

        KisAnimationCurveChannelListDelegate *listDelegate = new KisAnimationCurveChannelListDelegate(channelListView);
        channelListView->setItemDelegate(listDelegate);
        channelListView->setHeaderHidden(true);
    }

    KisAnimationCurvesModel *curvesModel;
    KisAnimationCurvesView *curvesView;

    KisAnimationCurveChannelListModel *channelListModel;
    QTreeView *channelListView;

    QPointer<KisCanvas2> canvas;

    KisSignalAutoConnectionsStore canvasConnections;
};

KisAnimationCurveDocker::KisAnimationCurveDocker()
    : QDockWidget(i18n("Animation curves"))
    , m_d(new Private(this))
{
    QSplitter *splitter = new QSplitter(this);

    splitter->addWidget(m_d->channelListView);
    splitter->addWidget(m_d->curvesView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);
    setWidget(splitter);

    connect(m_d->channelListModel, &KisAnimationCurveChannelListModel::rowsInserted,
            this, &KisAnimationCurveDocker::slotListRowsInserted);
}

KisAnimationCurveDocker::~KisAnimationCurveDocker()
{}

void KisAnimationCurveDocker::setCanvas(KoCanvasBase * canvas)
{
    if (canvas && m_d->canvas == canvas) return;

    if (m_d->canvas) {
        m_d->canvasConnections.clear();
        m_d->canvas->disconnectCanvasObserver(this);
    }

    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if(m_d->canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_d->canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        m_d->channelListModel->setDummiesFacade(kritaShapeController);

        m_d->curvesModel->setImage(m_d->canvas->image());
        m_d->curvesModel->setFrameCache(m_d->canvas->frameCache());
        m_d->curvesModel->setAnimationPlayer(m_d->canvas->animationPlayer());

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigUiNeedChangeSelectedNodes(KisNodeList)),
            m_d->channelListModel, SLOT(selectedNodesChanged(KisNodeList))
        );

        m_d->channelListModel->selectedNodesChanged(m_d->canvas->viewManager()->nodeManager()->selectedNodes());
    }
}

void KisAnimationCurveDocker::unsetCanvas()
{
    setCanvas(0);
}

void KisAnimationCurveDocker::setMainWindow(KisViewManager *kisview)
{

}

void KisAnimationCurveDocker::slotListRowsInserted(const QModelIndex &parentIndex, int first, int last)
{
    // Auto-expand nodes on the tree

    for (int r=first; r<=last; r++) {
        QModelIndex index = m_d->channelListModel->index(r, 0, parentIndex);
        m_d->channelListView->expand(index);
    }
}
