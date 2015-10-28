/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "timeline_node_list_keeper.h"

#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"
#include "timeline_frames_index_converter.h"


struct TimelineNodeListKeeper::Private
{
    Private(ModelWithExternalNotifications *_model,
            KisDummiesFacadeBase *_dummiesFacade)
        : model(_model),
          dummiesFacade(_dummiesFacade),
          converter(dummiesFacade)
    {
    }

    ModelWithExternalNotifications *model;
    KisDummiesFacadeBase *dummiesFacade;

    TimelineFramesIndexConverter converter;

    QVector<KisNodeDummy*> dummiesList;

    void populateDummiesList() {
        const int rowCount = converter.rowCount();
        for (int i = 0; i < rowCount; ++i) {
            dummiesList.append(converter.dummyFromRow(i));
        }
    }
};

TimelineNodeListKeeper::TimelineNodeListKeeper(ModelWithExternalNotifications *model, KisDummiesFacadeBase *dummiesFacade)
    : m_d(new Private(model, dummiesFacade))
{
    KIS_ASSERT_RECOVER_RETURN(m_d->dummiesFacade);

    connect(m_d->dummiesFacade, SIGNAL(sigEndInsertDummy(KisNodeDummy*)),
            SLOT(slotEndInsertDummy(KisNodeDummy*)));
    connect(m_d->dummiesFacade, SIGNAL(sigBeginRemoveDummy(KisNodeDummy*)),
            SLOT(slotBeginRemoveDummy(KisNodeDummy*)));
    connect(m_d->dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
            SLOT(slotDummyChanged(KisNodeDummy*)));

    m_d->populateDummiesList();
}

TimelineNodeListKeeper::~TimelineNodeListKeeper()
{
}

KisNodeDummy* TimelineNodeListKeeper::dummyFromRow(int row)
{
    if (row >= 0 && row < m_d->dummiesList.size()) {
        return m_d->dummiesList[row];
    }

    return 0;
}

int TimelineNodeListKeeper::rowForDummy(KisNodeDummy *dummy)
{
    return m_d->dummiesList.indexOf(dummy);
}

int TimelineNodeListKeeper::rowCount()
{
    return m_d->dummiesList.size();
}

void TimelineNodeListKeeper::updateActiveDummy(KisNodeDummy *dummy)
{
    bool oldRemoved = false;
    bool newAdded = false;

    KisNodeDummy *oldActiveDummy = m_d->converter.activeDummy();
    m_d->converter.updateActiveDummy(dummy, &oldRemoved, &newAdded);


    if (oldRemoved) {
        slotBeginRemoveDummy(oldActiveDummy);
    }

    if (newAdded) {
        slotEndInsertDummy(dummy);
    }
}

void TimelineNodeListKeeper::slotEndInsertDummy(KisNodeDummy *dummy)
{
    KIS_ASSERT_RECOVER_RETURN(!m_d->dummiesList.contains(dummy));

    if (m_d->converter.isDummyVisible(dummy)) {
        int pos = m_d->converter.rowForDummy(dummy);

        m_d->model->callBeginInsertRows(QModelIndex(), pos, pos);
        m_d->dummiesList.insert(pos, 1, dummy);
        m_d->model->callEndInsertRows();
    }
}

void TimelineNodeListKeeper::slotBeginRemoveDummy(KisNodeDummy *dummy)
{
    if (m_d->dummiesList.contains(dummy)) {
        int pos = m_d->dummiesList.indexOf(dummy);

        m_d->model->callBeginRemoveRows(QModelIndex(), pos, pos);
        m_d->dummiesList.remove(pos);
        m_d->model->callEndRemoveRows();
    }

    m_d->converter.notifyDummyRemoved(dummy);
}

void TimelineNodeListKeeper::slotDummyChanged(KisNodeDummy *dummy)
{
    const bool present = m_d->dummiesList.contains(dummy);
    const bool shouldBe = m_d->converter.isDummyVisible(dummy);

    if (!present && shouldBe) {
        slotEndInsertDummy(dummy);
    } else if (present && !shouldBe) {
        slotBeginRemoveDummy(dummy);
    }
}

void findOtherLayers(KisNodeDummy *root,
                     TimelineNodeListKeeper::OtherLayersList *list,
                     const QString &prefix)
{
    KisNodeSP node = root->node();

    if (root->parent() && !node->useInTimeline()) {
        *list <<
            TimelineNodeListKeeper::OtherLayer(
                QString(prefix + node->name()),
                root);
    }

    KisNodeDummy *dummy = root->lastChild();
    while(dummy) {
        findOtherLayers(dummy, list, prefix + " ");
        dummy = dummy->prevSibling();
    }
}

TimelineNodeListKeeper::OtherLayersList
TimelineNodeListKeeper::otherLayersList() const
{
    OtherLayersList list;
    findOtherLayers(m_d->dummiesFacade->rootDummy(), &list, "");
    return list;
}
