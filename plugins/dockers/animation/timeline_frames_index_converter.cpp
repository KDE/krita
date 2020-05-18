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

#include "timeline_frames_index_converter.h"

#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"


TimelineFramesIndexConverter::TimelineFramesIndexConverter(KisDummiesFacadeBase *dummiesFacade)
    : m_dummiesFacade(dummiesFacade),
      m_activeDummy(0),
      m_showGlobalSelectionMask(false)
{
}

TimelineFramesIndexConverter::~TimelineFramesIndexConverter()
{
}

bool TimelineFramesIndexConverter::calcNodesInPath(KisNodeDummy *root, int &startCount, KisNodeDummy *endDummy)
{
    if (isDummyVisible(root)) {
        if (endDummy && root == endDummy) {
            return true;
        }

        startCount++;
    }

    KisNodeDummy *dummy = root->lastChild();
    while (dummy) {
        if (calcNodesInPath(dummy, startCount, endDummy)) {
            return true;
        }

        dummy = dummy->prevSibling();
    }

    return false;
}

KisNodeDummy* TimelineFramesIndexConverter::findNodeFromRow(KisNodeDummy *root, int &startCount)
{
    if (isDummyVisible(root)) {
        if (!startCount) {
            return root;
        }

        startCount--;
    }

    KisNodeDummy *dummy = root->lastChild();
    while (dummy) {
        KisNodeDummy *found = findNodeFromRow(dummy, startCount);
        if (found) return found;

        dummy = dummy->prevSibling();
    }

    return 0;
}

KisNodeDummy* TimelineFramesIndexConverter::dummyFromRow(int row)
{
    KisNodeDummy *root = m_dummiesFacade->rootDummy();
    if(!root) return 0;

    return findNodeFromRow(root, row);
}

int TimelineFramesIndexConverter::rowForDummy(KisNodeDummy *dummy)
{
    if (!dummy) return -1;

    KisNodeDummy *root = m_dummiesFacade->rootDummy();
    if(!root) return -1;

    int count = 0;
    return calcNodesInPath(root, count, dummy) ? count : -1;
}

int TimelineFramesIndexConverter::rowCount()
{
    KisNodeDummy *root = m_dummiesFacade->rootDummy();
    if(!root) return 0;

    int count = 0;
    calcNodesInPath(root, count, 0);
    return count;
}

KisNodeDummy* TimelineFramesIndexConverter::activeDummy() const
{
    return m_activeDummy;
}

void TimelineFramesIndexConverter::updateActiveDummy(KisNodeDummy *dummy,
                                                     bool *oldRemoved,
                                                     bool *newAdded)
{
    if (m_activeDummy == dummy) return;

    if (m_activeDummy && !m_activeDummy->node()->useInTimeline()) {
        *oldRemoved = true;
    }

    m_activeDummy = dummy;

    if (m_activeDummy && !m_activeDummy->node()->useInTimeline()) {
        *newAdded = true;
    }
}

void TimelineFramesIndexConverter::notifyDummyRemoved(KisNodeDummy *dummy)
{
    if (m_activeDummy && m_activeDummy == dummy) {
        m_activeDummy = 0;
    }
}

void TimelineFramesIndexConverter::setShowGlobalSelectionMask(bool value)
{
    m_showGlobalSelectionMask = value;
}

bool TimelineFramesIndexConverter::isDummyAvailableForTimeline(KisNodeDummy *dummy) const
{
    return dummy->isGUIVisible(m_showGlobalSelectionMask);
}

bool TimelineFramesIndexConverter::isDummyVisible(KisNodeDummy *dummy) const
{
    return (isDummyAvailableForTimeline(dummy) &&
            dummy->node()->useInTimeline()) ||
            dummy == m_activeDummy;
}
