/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_recorded_paint_action_editor.h"

#include <QLabel>
#include <QGridLayout>

#include "recorder/kis_recorded_paint_action.h"
#include <kis_paintop_preset.h>
#include <kis_paintop_registry.h>
#include <kis_paintop_settings_widget.h>

KisRecordedPaintActionEditor::KisRecordedPaintActionEditor(QWidget* parent, KisRecordedAction* action) : QWidget(parent),
        m_action(dynamic_cast<KisRecordedPaintAction*>(action)),
        m_gridLayout(new QGridLayout(this))
{
    Q_ASSERT(m_action);
    m_configWidget = KisPaintOpRegistry::instance()->get(m_action->paintOpPreset()->paintOp().id())->createSettingsWidget(this);
    if (m_configWidget) {
        m_gridLayout->addWidget(m_configWidget);
        m_configWidget->setConfiguration(m_action->paintOpPreset()->settings());
        connect(m_configWidget, SIGNAL(sigConfigurationUpdated()), SLOT(configurationUpdated()));
    } else {
        m_gridLayout->addWidget(new QLabel("No configuration option.", this));
    }
}

KisRecordedPaintActionEditor::~KisRecordedPaintActionEditor()
{
}

void KisRecordedPaintActionEditor::configurationUpdated()
{
    m_configWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_action->paintOpPreset()->settings().data()));
    emit(actionEdited());
}

KisRecordedPaintActionEditorFactory::KisRecordedPaintActionEditorFactory()
{
}

KisRecordedPaintActionEditorFactory::~KisRecordedPaintActionEditorFactory()
{
}

QWidget* KisRecordedPaintActionEditorFactory::createEditor(QWidget* parent, KisRecordedAction* action) const
{
    return new KisRecordedPaintActionEditor(parent, action);
}

bool KisRecordedPaintActionEditorFactory::canEdit(const KisRecordedAction* action) const
{
    return dynamic_cast<const KisRecordedPaintAction*>(action);
}

#include "kis_recorded_paint_action_editor.moc"
