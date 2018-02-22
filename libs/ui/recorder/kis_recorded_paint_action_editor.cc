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

#include <KoColor.h>

#include "recorder/kis_recorded_paint_action.h"
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_registry.h>
#include <brushengine/kis_paintop_config_widget.h>

#include "ui_wdgpaintactioneditor.h"
#include <KoColorSpaceRegistry.h>
#include <KoResourcePaths.h>
#include "kis_node_query_path_editor.h"
#include <recorder/kis_node_query_path.h>

KisRecordedPaintActionEditor::KisRecordedPaintActionEditor(QWidget* parent, KisRecordedAction* action) : QWidget(parent),
        m_action(dynamic_cast<KisRecordedPaintAction*>(action)),
        m_actionEditor(new Ui_WdgPaintActionEditor),
        m_configWidget(0)
{
    Q_ASSERT(m_action);
    m_actionEditor->setupUi(this);

    // Setup paint color editor
    m_actionEditor->paintColor->setColor(m_action->paintColor());
    connect(m_actionEditor->paintColor, SIGNAL(changed(KoColor)),
            this, SLOT(configurationUpdated()));

    // Setup background color editor
    m_actionEditor->backgroundColor->setColor(m_action->backgroundColor());
    connect(m_actionEditor->backgroundColor, SIGNAL(changed(KoColor)),
            this, SLOT(configurationUpdated()));

    // Setup opacity
    m_actionEditor->opacity->setValue(m_action->opacity() * 100.0);
    connect(m_actionEditor->opacity, SIGNAL(valueChanged(int)), SLOT(configurationUpdated()));

    // Setup paint ops

    QList<KoID> keys = KisPaintOpRegistry::instance()->listKeys();
    Q_FOREACH (const KoID& paintopId, keys) {
        QIcon pm = KisPaintOpRegistry::instance()->icon(paintopId);

        m_actionEditor->paintOps->addItem(pm, paintopId.name());
        m_paintops.append(paintopId.id());
    }
    connect(m_actionEditor->paintOps, SIGNAL(activated(int)), SLOT(paintOpChanged(int)));

    // Setup configuration widget for paint op settings
    m_gridLayout = new QGridLayout(m_actionEditor->frmOptionWidgetContainer);
    setPaintOpPreset();
    m_actionEditor->paintOps->setCurrentIndex(m_paintops.indexOf(m_action->paintOpPreset()->paintOp().id()));
    m_paintOpsToPreset[m_action->paintOpPreset()->paintOp().id()] = m_action->paintOpPreset();
    connect(m_actionEditor->wdgPresetChooser, SIGNAL(resourceSelected(KoResource*)), SLOT(resourceSelected(KoResource*)));

    // Setup the query path editor
    m_actionEditor->nodeQueryPathEditor->setNodeQueryPath(m_action->nodeQueryPath());
    connect(m_actionEditor->nodeQueryPathEditor, SIGNAL(nodeQueryPathChanged()), SLOT(nodeQueryPathChanged()));
    
}

KisRecordedPaintActionEditor::~KisRecordedPaintActionEditor()
{
    delete m_actionEditor;
}

void KisRecordedPaintActionEditor::configurationUpdated()
{
    m_configWidget->writeConfigurationSafe(const_cast<KisPaintOpSettings*>(m_action->paintOpPreset()->settings().data()));

    m_action->setPaintColor(m_actionEditor->paintColor->color());
    m_action->setBackgroundColor(m_actionEditor->backgroundColor->color());
    m_action->setOpacity(m_actionEditor->opacity->value() / qreal(100.0));

    emit(actionEdited());
}

void KisRecordedPaintActionEditor::paintOpChanged(int index)
{
    QString id = m_paintops[index];
    KisPaintOpPresetSP preset = m_paintOpsToPreset[id];
    if (!preset) {
        preset = KisPaintOpRegistry::instance()->defaultPreset(KoID(id, ""));
        m_paintOpsToPreset[id] = preset;
    }
    m_action->setPaintOpPreset(preset);
    setPaintOpPreset();
}

void KisRecordedPaintActionEditor::resourceSelected(KoResource* resource)
{
    KisPaintOpPresetSP preset = static_cast<KisPaintOpPreset*>(resource);

    m_paintOpsToPreset[preset->paintOp().id()] = preset;
    m_action->setPaintOpPreset(preset);
    setPaintOpPreset();
}

void KisRecordedPaintActionEditor::nodeQueryPathChanged()
{
    m_action->setNodeQueryPath(m_actionEditor->nodeQueryPathEditor->nodeQueryPath());
    emit(actionEdited());
}


void KisRecordedPaintActionEditor::setPaintOpPreset()
{
    delete m_configWidget;
    m_configWidget = KisPaintOpRegistry::instance()->get(m_action->paintOpPreset()->paintOp().id())->createConfigWidget(m_actionEditor->frmOptionWidgetContainer);
    if (m_configWidget) {
        m_gridLayout->addWidget(m_configWidget);
        //TODO use default configuration instead?
        //m_configWidget->setConfiguration(m_action->paintOpPreset()->settings());
        connect(m_configWidget, SIGNAL(sigConfigurationUpdated()), SLOT(configurationUpdated()));
    } else {
        m_gridLayout->addWidget(new QLabel(i18n("No configuration option."), this));
    }
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

