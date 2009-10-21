/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "widgets/kis_paintop_presets_popup.h"

#include <QList>
#include <QComboBox>
#include <QHBoxLayout>
#include <QToolButton>
#include <QGridLayout>
#include <QFont>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kis_paintop_preset.h>
#include <widgets/kis_preset_widget.h>

#include <ui_wdgpaintoppresets.h>


class KisPaintOpPresetsPopup::Private
{

public:

    Ui_WdgPaintOpPresets uiWdgPaintOpPresets;
    QGridLayout * layout;
    QWidget * settingsWidget;
    QFont smallFont;

};

KisPaintOpPresetsPopup::KisPaintOpPresetsPopup(QWidget * parent)
        : QWidget(parent)
        , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsPopup");
    KConfigGroup group(KGlobal::config(), "GUI");
    m_d->smallFont  = KGlobalSettings::generalFont();
    qreal pointSize = group.readEntry("palettefontsize", m_d->smallFont.pointSize() * 0.75);
    pointSize = qMax(pointSize, KGlobalSettings::smallestReadableFont().pointSizeF());
    m_d->smallFont.setPointSizeF(pointSize);
    setFont(m_d->smallFont);

    m_d->uiWdgPaintOpPresets.setupUi(this);

    // TODO 2.2 Disables preset collections
    QWidget * collectionWidget = m_d->uiWdgPaintOpPresets.tabPresets->widget(1);
    m_d->uiWdgPaintOpPresets.tabPresets->removeTab(1);
    delete collectionWidget;
    m_d->uiWdgPaintOpPresets.bnSave->setEnabled(false);

    m_d->layout = new QGridLayout(m_d->uiWdgPaintOpPresets.frmOptionWidgetContainer);
    m_d->layout->setSizeConstraint(QLayout::SetFixedSize);

    m_d->settingsWidget = 0;
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    connect(m_d->uiWdgPaintOpPresets.bnSave, SIGNAL(clicked()),
            this, SIGNAL(savePresetClicked()));
}


KisPaintOpPresetsPopup::~KisPaintOpPresetsPopup()
{
    m_d->layout->removeWidget(m_d->settingsWidget);
    m_d->settingsWidget->hide();
    m_d->settingsWidget->setParent(0);
    m_d->settingsWidget = 0;
    delete m_d;
}

void KisPaintOpPresetsPopup::setPaintOpSettingsWidget(QWidget * widget)
{
    if (m_d->settingsWidget) {
        m_d->layout->removeWidget(m_d->settingsWidget);
        m_d->uiWdgPaintOpPresets.frmOptionWidgetContainer->updateGeometry();
    }
    m_d->layout->update();
    updateGeometry();

    if (widget) {

        widget->setFont(m_d->smallFont);

        m_d->settingsWidget = widget;
        m_d->settingsWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        m_d->layout->addWidget(widget);

        m_d->layout->update();
        widget->show();
    }
}

KisPresetWidget* KisPaintOpPresetsPopup::presetPreview()
{
    return m_d->uiWdgPaintOpPresets.presetPreview;
}

#include "kis_paintop_presets_popup.moc"
