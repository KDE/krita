/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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


#include "kis_recorded_filter_action_editor.h"
#include <recorder/kis_recorded_filter_action.h>
#include <filter/kis_filter.h>
#include <qgridlayout.h>
#include <kis_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <QLabel>
#include <kis_paint_device.h>

KisRecordedFilterActionEditor::KisRecordedFilterActionEditor(QWidget* parent, KisRecordedAction* action) :
        m_action(dynamic_cast<KisRecordedFilterAction*>(action)),
        m_gridLayout(new QGridLayout(this))
{
    Q_UNUSED(parent);
    Q_ASSERT(m_action);
    m_configWidget = m_action->filter()->createConfigurationWidget(this, 0 , 0);
    if (m_configWidget) {
        m_gridLayout->addWidget(m_configWidget);
        m_configWidget->setConfiguration(m_action->filterConfiguration());
        connect(m_configWidget, SIGNAL(sigConfigurationItemChanged()), SLOT(configurationUpdated()));
    } else {
        m_gridLayout->addWidget(new QLabel("No configuration option.", this));
    }
}

KisRecordedFilterActionEditor::~KisRecordedFilterActionEditor()
{
}

void KisRecordedFilterActionEditor::configurationUpdated()
{
    KisFilterConfiguration* config = dynamic_cast<KisFilterConfiguration*>(m_configWidget->configuration());
    if (config) {
        m_action->setFilterConfiguration(config);
        emit(actionEdited());
    }
}

KisRecordedFilterActionEditorFactory::KisRecordedFilterActionEditorFactory()
{
}

KisRecordedFilterActionEditorFactory::~KisRecordedFilterActionEditorFactory()
{
}

QWidget* KisRecordedFilterActionEditorFactory::createEditor(QWidget* parent, KisRecordedAction* action) const
{
    return new KisRecordedFilterActionEditor(parent, action);
}

bool KisRecordedFilterActionEditorFactory::canEdit(const KisRecordedAction* action) const
{
    return action->id() == "FilterAction";
}

