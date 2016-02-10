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

#include <kis_icon_utils.h>
#include <kis_slider_spin_box.h>
#include <kis_paintop_presets_chooser_popup.h>
#include <kis_resource_server_provider.h>
#include <KoCompositeOpRegistry.h>

#define ICON_SIZE 48

DualBrushOpOptionsWidget::DualBrushOpOptionsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    doubleVOffset->setRange(-100, 100, 1);
    connect(doubleVOffset, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    doubleHOffset->setRange(-100, 100, 1);
    connect(doubleHOffset, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    doubleFuzziness->setRange(0, 100, 1);
    connect(doubleFuzziness, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    doubleOpacity->setRange(0, 100, 0);
    doubleOpacity->setValue(100);
    doubleOpacity->setPrefix(QString("%1:  ").arg(i18n("Opacity")));
    doubleOpacity->setSuffix("%");
    connect(doubleOpacity, SIGNAL(valueChanged(qreal)), SIGNAL(configurationChanged()));

    tableSelected->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    tableSelected->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(tableSelected, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(itemSelected(QListWidgetItem*, QListWidgetItem*)));

    connect(bnAdd, SIGNAL(clicked()), SLOT(addPreset()));
    bnAdd->setIcon(KisIconUtils::loadIcon("arrow-right"));

    connect(bnRemove, SIGNAL(clicked()), SLOT(removePreset()));
    bnRemove->setIcon(KisIconUtils::loadIcon("edit-delete"));

    connect(bnUp, SIGNAL(clicked()), SLOT(movePresetUp()));
    bnUp->setIcon(KisIconUtils::loadIcon("arrow-up"));

    connect(bnDown, SIGNAL(clicked()), SLOT(movePresetDown()));
    bnDown->setIcon(KisIconUtils::loadIcon("arrow-down"));

    connect(this, SIGNAL(configurationChanged()), SLOT(updatePreset()));

}

DualBrushOpOptionsWidget::~DualBrushOpOptionsWidget()
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


void DualBrushOpOptionsWidget::setPresetStack(const QVector<StackedPreset> &presetStack)
{
    blockSignals(true);
    tableSelected->clear();
    for(int i = 0; i < presetStack.count(); ++i){
        QImage image;
        if (presetStack[i].paintopPreset) {
            image = presetStack[i].paintopPreset->image();
        }
        QListWidgetItem *item = new QListWidgetItem(imageToIcon(image), presetStack[i].presetName);
        item->setData(Qt::UserRole, QVariant::fromValue<StackedPreset>(presetStack[i]));
        tableSelected->addItem(item);
    }
    tableSelected->setCurrentRow(0);
    itemSelected(tableSelected->currentItem(), 0);
    blockSignals(false);
}

QVector<StackedPreset> DualBrushOpOptionsWidget::presetStack() const
{
    QVector<StackedPreset> stack;
    for(int i = 0; i < tableSelected->count(); ++i) {
        stack << tableSelected->item(i)->data(Qt::UserRole).value<StackedPreset>();
    }
    return stack;
}

void DualBrushOpOptionsWidget::updatePreset()
{
    if (tableSelected->currentItem()) {
        QListWidgetItem *item = tableSelected->currentItem();
        StackedPreset preset = item->data(Qt::UserRole).value<StackedPreset>();
        preset.fuzziness = doubleFuzziness->value();
        preset.compositeOp = cmbComposite->selectedCompositeOp().id();
        preset.opacitiy = doubleOpacity->value();
        preset.verticalOffset = doubleVOffset->value();
        preset.horizontalOffset = doubleHOffset->value();
        item->setData(Qt::UserRole, QVariant::fromValue<StackedPreset>(preset));
    }
}

void DualBrushOpOptionsWidget::addPreset()
{
    doubleFuzziness->setValue(0.0);
    doubleOpacity->setValue(100.0);
    doubleVOffset->setValue(0.0);
    doubleHOffset->setValue(0.0);
    cmbComposite->selectCompositeOp(KoID(COMPOSITE_OVER));

    StackedPreset preset;
    preset.presetName = wdgPresetChooser->currentPaintOp();
    preset.fuzziness = doubleFuzziness->value();
    preset.compositeOp = cmbComposite->selectedCompositeOp().id();
    preset.opacitiy = doubleOpacity->value();
    preset.verticalOffset = doubleVOffset->value();
    preset.horizontalOffset = doubleHOffset->value();

    KisPaintOpPresetResourceServer* server = KisResourceServerProvider::instance()->paintOpPresetServer();

    preset.paintopPreset = server->resourceByName(preset.presetName);
    Q_ASSERT(preset.paintopPreset);
    QListWidgetItem *item = new QListWidgetItem(imageToIcon(preset.paintopPreset->image()), preset.presetName);
    item->setData(Qt::UserRole, QVariant::fromValue<StackedPreset>(preset));

    tableSelected->addItem(item);
    tableSelected->setCurrentRow(tableSelected->count() -1);
}

void DualBrushOpOptionsWidget::removePreset()
{
    delete tableSelected->takeItem(tableSelected->currentRow());
}

void DualBrushOpOptionsWidget::movePresetUp()
{
    int currentRow = tableSelected->currentRow();
    if (currentRow > 0) {
        QListWidgetItem *item = tableSelected->takeItem(currentRow);
        tableSelected->insertItem(currentRow - 1, item);
        tableSelected->setCurrentRow(currentRow - 1);
    }
}

void DualBrushOpOptionsWidget::movePresetDown()
{
    int currentRow = tableSelected->currentRow();
    if (currentRow < tableSelected->count() - 1) {
        QListWidgetItem *item = tableSelected->takeItem(currentRow);
        tableSelected->insertItem(currentRow + 1, item);
        tableSelected->setCurrentRow(currentRow + 1);
    }
}

void DualBrushOpOptionsWidget::itemSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous) {
        StackedPreset psPrevious = previous->data(Qt::UserRole).value<StackedPreset>();
        psPrevious.fuzziness = doubleFuzziness->value();
        psPrevious.compositeOp = cmbComposite->selectedCompositeOp().id();
        psPrevious.opacitiy = doubleOpacity->value();
        psPrevious.verticalOffset = doubleVOffset->value();
        psPrevious.horizontalOffset = doubleHOffset->value();
        previous->setData(Qt::UserRole, QVariant::fromValue<StackedPreset>(psPrevious));
    }
    if (current) {
        StackedPreset psCurrent = current->data(Qt::UserRole).value<StackedPreset>();
        doubleFuzziness->setValue(psCurrent.fuzziness);
        doubleOpacity->setValue(psCurrent.opacitiy);
        doubleVOffset->setValue(psCurrent.verticalOffset);
        doubleHOffset->setValue(psCurrent.horizontalOffset);
        cmbComposite->selectCompositeOp(KoID(psCurrent.compositeOp));
    }
}

DualBrushOpOption::DualBrushOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    m_checkable = false;
    m_dualBrushOptionsWidget = new DualBrushOpOptionsWidget();
    m_dualBrushOptionsWidget->hide();
    setObjectName("DualBrushOpOption");
    setConfigurationPage(m_dualBrushOptionsWidget);
    connect(m_dualBrushOptionsWidget, SIGNAL(configurationChanged()), SLOT(emitSettingChanged()));
}

DualBrushOpOption::~DualBrushOpOption()
{
}

void DualBrushOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    QVector<StackedPreset> stack = m_dualBrushOptionsWidget->presetStack();
    setting->setProperty("dualbrush/preset_count", stack.count());
    for(int i = 0; i < stack.count(); ++i) {
        setting->setProperty(QString("dualbrush/preset_%1_compositeop").arg(i), stack[i].compositeOp);
        setting->setProperty(QString("dualbrush/preset_%1_fuzziness").arg(i), stack[i].fuzziness);
        setting->setProperty(QString("dualbrush/preset_%1_horizontal_offset").arg(i), stack[i].horizontalOffset);
        setting->setProperty(QString("dualbrush/preset_%1_name").arg(i), stack[i].presetName);
        setting->setProperty(QString("dualbrush/preset_%1_opacity").arg(i), stack[i].opacitiy);
        setting->setProperty(QString("dualbrush/preset_%1_vertical_offset").arg(i), stack[i].verticalOffset);
    }
}

void DualBrushOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{

    QVector<StackedPreset> stack;
    int count = setting->getInt("dualbrush/preset_count");
    for (int i = 0; i < count; ++i) {
        StackedPreset ps;
        ps.compositeOp = setting->getString(QString("dualbrush/preset_%1_compositeop").arg(i));
        ps.fuzziness = setting->getDouble(QString("dualbrush/preset_%1_fuzziness").arg(i));
        ps.horizontalOffset = setting->getDouble(QString("dualbrush/preset_%1_horizontal_offset").arg(i));
        ps.opacitiy = setting->getDouble(QString("dualbrush/preset_%1_opacity").arg(i));
        ps.presetName = setting->getString(QString("dualbrush/preset_%1_name").arg(i));
        KisPaintOpPresetResourceServer* server = KisResourceServerProvider::instance()->paintOpPresetServer();
        ps.paintopPreset = server->resourceByName(ps.presetName);
        ps.verticalOffset = setting->getDouble(QString("dualbrush/preset_%1_vertical_offset").arg(i));
        stack << ps;
    }
    m_dualBrushOptionsWidget->setPresetStack(stack);
}

QVector<StackedPreset> DualBrushOpOption::presetStack() const
{
    return m_dualBrushOptionsWidget->presetStack();
}
