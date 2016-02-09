/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#include "DualBrushOption.h"

#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QVariant>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>

#include <kis_slider_spin_box.h>
#include <kis_paintop_presets_chooser_popup.h>
#include <kis_resource_server_provider.h>

#define ICON_SIZE 48

KisDualBrushOpOptionsWidget::KisDualBrushOpOptionsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    doubleVOffset->setRange(-100, 100, 1);
    connect(doubleVOffset, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    doubleHOffset->setRange(-100, 100, 1);
    connect(doubleHOffset, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    doubleFuzziness->setRange(0, 100, 1);
    connect(doubleFuzziness, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    doubleOpacity->setValue(100);
    doubleOpacity->setRange(0, 100, 0);
    doubleOpacity->setPrefix(QString("%1:  ").arg(i18n("Opacity")));
    doubleOpacity->setSuffix("%");
    connect(doubleOpacity, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    tableSelected->setIconSize(QSize(ICON_SIZE, ICON_SIZE));

    connect(bnAdd, SIGNAL(clicked()), SLOT(presetAdded()));
    connect(bnRemove, SIGNAL(clicked()), SLOT(presetRemoved()));
    connect(bnUp, SIGNAL(clicked()), SLOT(movePresetUp()));
    connect(bnDown, SIGNAL(clicked()), SLOT(movePresetDown()));
}

KisDualBrushOpOptionsWidget::~KisDualBrushOpOptionsWidget()
{
}

// XXX: copy-pasted from dlg_create_bundle, refactor!
QPixmap imageToIcon(const QImage &img) {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill();
    QImage scaled = img.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    int x = (ICON_SIZE - scaled.width()) / 2;
    int y = (ICON_SIZE - scaled.height()) / 2;
    QPainter gc(&pixmap);
    gc.drawImage(x, y, scaled);
    gc.end();
    return pixmap;
}


void KisDualBrushOpOptionsWidget::addPreset()
{
    StackedPreset preset;
    preset.presetName = wdgPresetChooser->currentPaintOp();
    preset.compositeOp = cmbComposite->selectedCompositeOp().id();
    preset.opacitiy = doubleOpacity->value();
    preset.verticalOffset = doubleVOffset->value();
    preset.horizontalOffset = doubleHOffset->value();

    KisPaintOpPresetResourceServer* server = KisResourceServerProvider::instance()->paintOpPresetServer();
    preset.paintop = server->resourceByName(preset.presetName);

    QListWidgetItem *item = new QListWidgetItem(imageToIcon(preset.paintop->image()), preset.presetName);
    item->setData(Qt::UserRole, QVariant::fromValue<StackedPreset>(preset));

    tableSelected->addItem(item);
}

void KisDualBrushOpOptionsWidget::removePreset()
{

}

void KisDualBrushOpOptionsWidget::movePresetUp()
{

}

void KisDualBrushOpOptionsWidget::movePresetDown()
{

}

KisDualBrushOpOption::KisDualBrushOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    m_checkable = false;
    m_dualBrushOptionsWidget = new KisDualBrushOpOptionsWidget();
    m_dualBrushOptionsWidget->hide();
    setObjectName("KisDualBrushOpOption");
    setConfigurationPage(m_dualBrushOptionsWidget);
}

KisDualBrushOpOption::~KisDualBrushOpOption()
{
    // delete m_options;
}

void KisDualBrushOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    //    setting->setProperty(DUALBRUSH_RADIUS, radius());
}

void KisDualBrushOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    //    m_options->radiusSpinBox->setValue(setting->getInt(DUALBRUSH_RADIUS));
}



void DualBrushProperties::readOptionSetting(const KisPropertiesConfiguration *settings) {
    //radius = settings->getInt(DUALBRUSH_RADIUS);
}
