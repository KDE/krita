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

#include "kis_recorded_filter_action_creator.h"

#include <QGridLayout>
#include <QHeaderView>

#include <klocalizedstring.h>

#include <filter/kis_filter.h>
#include <kis_paint_device.h>
#include <kis_filter_selector_widget.h>
#include <kis_filters_model.h>
#include <recorder/kis_recorded_filter_action.h>
#include <recorder/kis_node_query_path.h>
#include <kis_filter_configuration.h>

KisRecordedFilterActionCreator::KisRecordedFilterActionCreator(QWidget* parent , Qt::WindowFlags f)
    : KisRecordedActionCreator(parent, f)
{
    m_filterModel = new KisFiltersModel(true, 0);
    m_filterTree = new QTreeView(this);
    m_filterTree->setModel(m_filterModel);
    m_filterTree->header()->setVisible(false);
    QGridLayout* layout = new QGridLayout();
    setLayout(layout);
    layout->addWidget(m_filterTree, 0, 0, 1, 1);
}

KisRecordedFilterActionCreator::~KisRecordedFilterActionCreator()
{
    delete m_filterTree;
    delete m_filterModel;
}

KisRecordedAction* KisRecordedFilterActionCreator::createAction() const
{
    const KisFilter* filter = m_filterModel->indexToFilter(m_filterTree->currentIndex());
    if(!filter) return 0;
    return new KisRecordedFilterAction(filter->name(), KisNodeQueryPath::fromString(""), filter, filter->defaultConfiguration());
}


KisRecordedFilterActionCreatorFactory::KisRecordedFilterActionCreatorFactory()
    : KisRecordedActionCreatorFactory("filter", i18nc("recorded filter action", "Apply Filter"))
{
}

KisRecordedFilterActionCreatorFactory::~KisRecordedFilterActionCreatorFactory()
{
}

bool KisRecordedFilterActionCreatorFactory::requireCreator() const
{
    return true;
}

KisRecordedActionCreator* KisRecordedFilterActionCreatorFactory::createCreator(QWidget* parent) const
{
    return new KisRecordedFilterActionCreator(parent);
}
