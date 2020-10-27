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

#ifndef __TIMELINE_FRAMES_INDEX_CONVERTER_H
#define __TIMELINE_FRAMES_INDEX_CONVERTER_H

#include "kritaanimationdocker_export.h"

class KisNodeDummy;

class KisDummiesFacadeBase;

class KRITAANIMATIONDOCKER_EXPORT TimelineFramesIndexConverter
{
public:
    TimelineFramesIndexConverter(KisDummiesFacadeBase *dummiesFacade);
    ~TimelineFramesIndexConverter();

    KisNodeDummy* dummyFromRow(int row);
    int rowForDummy(KisNodeDummy *dummy);
    int rowCount();

    KisNodeDummy* activeDummy() const;
    void updateActiveDummy(KisNodeDummy *dummy, bool *oldRemoved, bool *newAdded);
    void notifyDummyRemoved(KisNodeDummy *dummy);

    void setShowGlobalSelectionMask(bool value);

    bool isDummyAvailableForTimeline(KisNodeDummy *dummy) const;
    bool isDummyVisible(KisNodeDummy *dummy) const;

private:
    KisNodeDummy* findNodeFromRow(KisNodeDummy *root, int &startCount);
    bool calcNodesInPath(KisNodeDummy *root, int &startCount, KisNodeDummy *endDummy);

private:
    KisDummiesFacadeBase *m_dummiesFacade;
    KisNodeDummy *m_activeDummy;
    bool m_showGlobalSelectionMask;
};

#endif /* __TIMELINE_FRAMES_INDEX_CONVERTER_H */
