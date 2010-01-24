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
#include <KoColorPopupAction.h>

#include "recorder/kis_recorded_paint_action.h"
#include <kis_paintop_preset.h>
#include <kis_paintop_registry.h>
#include <kis_paintop_settings_widget.h>

#include "ui_wdgpaintactioneditor.h"
#include <KoColorSpaceRegistry.h>
#include <kis_factory2.h>
#include <kstandarddirs.h>

KisRecordedPaintActionEditor::KisRecordedPaintActionEditor(QWidget* parent, KisRecordedAction* action) : QWidget(parent),
        m_action(dynamic_cast<KisRecordedPaintAction*>(action)),
        m_actionEditor(new Ui_WdgPaintActionEditor),
        m_configWidget(0)
{
    Q_ASSERT(m_action);
    m_actionEditor->setupUi(this);

    // Setup paint color editor
    m_paintColorPopup = new KoColorPopupAction(this);
    m_paintColorPopup->setCurrentColor(m_action->paintColor());
    m_actionEditor->paintColor->setDefaultAction(m_paintColorPopup);
    connect(m_paintColorPopup, SIGNAL(colorChanged(const KoColor &)),
            this, SLOT(configurationUpdated()));

    // Setup background color editor
    m_backgroundColorPopup = new KoColorPopupAction(this);
    m_backgroundColorPopup->setCurrentColor(m_action->backgroundColor());
    m_actionEditor->backgroundColor->setDefaultAction(m_backgroundColorPopup);
    connect(m_backgroundColorPopup, SIGNAL(colorChanged(const KoColor &)),
            this, SLOT(configurationUpdated()));

    // Setup opacity
    m_actionEditor->opacity->setValue(m_action->opacity() / 2.55);
    connect(m_actionEditor->opacity, SIGNAL(valueChanged(int)), SLOT(configurationUpdated()));

    // Setup paint ops

    QList<KoID> keys = KisPaintOpRegistry::instance()->listKeys();
    foreach(const KoID& paintopId, keys) {
        if (KisPaintOpRegistry::instance()->userVisible(paintopId, KoColorSpaceRegistry::instance()->rgb8())) {
            QString pixmapName = KisPaintOpRegistry::instance()->pixmap(paintopId);

            QPixmap pm;
            if (!pixmapName.isEmpty()) {
                QString fname = KisFactory2::componentData().dirs()->findResource("kis_images", pixmapName);
                pm = QPixmap(fname);
            }


            if (pm.isNull()) {
                pm = QPixmap(16, 16);
                pm.fill();
            }

            m_actionEditor->paintOps->addItem(QIcon(pm), paintopId.name());
            m_paintops.append(paintopId.id());
        }
    }
    connect(m_actionEditor->paintOps, SIGNAL(activated(int)), SLOT(paintOpChanged(int)));

    // Setup configuration widget for paint op settings
    m_gridLayout = new QGridLayout(m_actionEditor->frmOptionWidgetContainer);
    setPaintOpPreset();
    m_actionEditor->paintOps->setCurrentIndex(m_paintops.indexOf(m_action->paintOpPreset()->paintOp().id()));
    m_paintOpsToPreset[m_action->paintOpPreset()->paintOp().id()] = m_action->paintOpPreset();
    connect(m_actionEditor->wdgPresetChooser, SIGNAL(resourceSelected(KoResource*)), SLOT(resourceSelected(KoResource*)));
}

KisRecordedPaintActionEditor::~KisRecordedPaintActionEditor()
{
    delete m_actionEditor;
}

void KisRecordedPaintActionEditor::configurationUpdated()
{
    m_configWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_action->paintOpPreset()->settings().data()));

    m_action->setPaintColor(m_paintColorPopup->currentKoColor());
    m_action->setBackgroundColor(m_backgroundColorPopup->currentKoColor());
    m_action->setOpacity(m_actionEditor->opacity->value() * 2.55);

    emit(actionEdited());
}

void KisRecordedPaintActionEditor::paintOpChanged(int index)
{
    QString id = m_paintops[index];
    KisPaintOpPresetSP preset = m_paintOpsToPreset[id];
    if (!preset) {
        preset = KisPaintOpRegistry::instance()->defaultPreset(KoID(id, ""), 0);
        m_paintOpsToPreset[id] = preset;
    }
    m_action->setPaintOpPreset(preset);
    setPaintOpPreset();
}

void KisRecordedPaintActionEditor::resourceSelected(KoResource* resource)
{
    KisPaintOpPresetSP preset = static_cast<KisPaintOpPreset*>(resource)->clone();

    m_paintOpsToPreset[preset->paintOp().id()] = preset;
    m_action->setPaintOpPreset(preset);
    setPaintOpPreset();
}

void KisRecordedPaintActionEditor::setPaintOpPreset()
{
    delete m_configWidget;
    m_configWidget = KisPaintOpRegistry::instance()->get(m_action->paintOpPreset()->paintOp().id())->createSettingsWidget(m_actionEditor->frmOptionWidgetContainer);
    if (m_configWidget) {
        m_gridLayout->addWidget(m_configWidget);
        m_configWidget->setConfiguration(m_action->paintOpPreset()->settings());
        connect(m_configWidget, SIGNAL(sigConfigurationUpdated()), SLOT(configurationUpdated()));
    } else {
        m_gridLayout->addWidget(new QLabel("No configuration option.", this));
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

#include "kis_recorded_paint_action_editor.moc"
