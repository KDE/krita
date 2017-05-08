/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_RECORDED_FILTER_ACTION_CREATOR_FACTORY_H_
#define _KIS_RECORDED_FILTER_ACTION_CREATOR_FACTORY_H_

#include "kis_recorded_action_creator.h"
#include "kis_recorded_action_creator_factory.h"

class KisFiltersModel;
class QTreeView;

class KisRecordedFilterActionCreator : public KisRecordedActionCreator {
public:
    explicit KisRecordedFilterActionCreator(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~KisRecordedFilterActionCreator() override;
    KisRecordedAction* createAction() const override;
private:
    KisFiltersModel* m_filterModel;
    QTreeView* m_filterTree;
};

/**
 * This class allows to create widgets that are used to create new actions.
 */
class KisRecordedFilterActionCreatorFactory : public KisRecordedActionCreatorFactory
{
public:
    KisRecordedFilterActionCreatorFactory();
    ~KisRecordedFilterActionCreatorFactory() override;
    bool requireCreator() const override;
    KisRecordedActionCreator* createCreator(QWidget* parent) const override;
};

#endif
