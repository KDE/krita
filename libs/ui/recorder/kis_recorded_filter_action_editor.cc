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


#include <kis_filter_configuration.h>
#include "kis_recorded_filter_action_editor.h"
#include <recorder/kis_recorded_filter_action.h>
#include <filter/kis_filter.h>
#include <QGridLayout>
#include <kis_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <QLabel>
#include <kis_paint_device.h>
#include "kis_node_query_path_editor.h"
#include <KoColorSpaceRegistry.h>
#include <recorder/kis_node_query_path.h>

KisRecordedFilterActionEditor::KisRecordedFilterActionEditor(QWidget* parent, KisRecordedAction* action)
    : QWidget(parent)
    , m_action(dynamic_cast<KisRecordedFilterAction*>(action))
    , m_gridLayout(new QGridLayout(this))
{
    Q_ASSERT(m_action);

    // Create the node query path editor
    m_nodeQueryPathEditor = new KisNodeQueryPathEditor(this);
    m_nodeQueryPathEditor->setNodeQueryPath(m_action->nodeQueryPath());
    connect(m_nodeQueryPathEditor, SIGNAL(nodeQueryPathChanged()), SLOT(nodeQueryPathChanged()));
    m_gridLayout->addWidget(m_nodeQueryPathEditor, 1, 0);

    // Create the filter editor
    m_configWidget = m_action->filter()->createConfigurationWidget(this, new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8()));
    if (m_configWidget) {
        m_gridLayout->addWidget(m_configWidget);

        // FIXME: pass the view object to the config widget
        //m_configWidget->setView(view);

        m_configWidget->setConfiguration(m_action->filterConfiguration());
        connect(m_configWidget, SIGNAL(sigConfigurationItemChanged()), SLOT(configurationUpdated()));
    } else {
        m_gridLayout->addWidget(new QLabel(i18n("No configuration option."), this));
    }
}

KisRecordedFilterActionEditor::~KisRecordedFilterActionEditor()
{
}

void KisRecordedFilterActionEditor::configurationUpdated()
{
    KisFilterConfigurationSP config = dynamic_cast<KisFilterConfiguration*>(m_configWidget->configuration().data());
    if (config) {
        m_action->setFilterConfiguration(config);
        emit(actionEdited());
    }
}

void KisRecordedFilterActionEditor::nodeQueryPathChanged()
{
    m_action->setNodeQueryPath(m_nodeQueryPathEditor->nodeQueryPath());
    emit(actionEdited());
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

