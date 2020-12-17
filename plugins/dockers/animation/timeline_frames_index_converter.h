/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
