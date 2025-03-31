/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolKnifeOptionsWidget.h"

#include "ui_KisToolKnifeOptionsWidget.h"

#include <KoColorSpaceRegistry.h>
#include "KisPaletteModel.h"

#include "kis_config.h"
#include <resources/KoColorSet.h>
#include "kis_canvas_resource_provider.h"
#include <KisOptionButtonStrip.h>
#include <kis_icon_utils.h>
#include <KoGroupButton.h>
#include <KoUnit.h>


Q_DECLARE_METATYPE(KisToolKnifeOptionsWidget::GapWidthType)

#include <KoUnit.h>

struct KisToolKnifeOptionsWidget::Private {
    Private()
    {
    }

    Ui_KisToolKnifeOptionsWidget *ui {nullptr};
    KoGroupButton* buttonModeAddGutter {nullptr};
    KoGroupButton* buttonModeRemoveGutter {nullptr};
    KoGroupButton* buttonModeMoveGutterEndPoint {nullptr};
    qreal resolution {1.0};

    QString toolId {""};
    //KoGroupButton* m_buttonModeAddGutter {nullptr};



    qreal getThickGapWidth(void)
    {
        return ui->thickGapWidth->value();
    }

    qreal getThinGapWidth(void)
    {
        return ui->thinGapWidth->value();
    }

    qreal getSpecialGapWidth(void)
    {
        return ui->customGapWidth->value();
    }

    KisToolKnifeOptionsWidget::GapWidthType getWidthType()
    {
        if (ui->thickGapWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Thick;
        } else if (ui->thinGapWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Thin;
        } else if (ui->customWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Special;
        } else { // if (ui->automaticGapWidthRadioButton->isChecked())
            return KisToolKnifeOptionsWidget::Automatic;
        }
    }

    qreal getWidthForType(KisToolKnifeOptionsWidget::GapWidthType type) {
        switch(type) {
            case KisToolKnifeOptionsWidget::Thick:
                return getThickGapWidth();
            case KisToolKnifeOptionsWidget::Thin:
                return getThinGapWidth();
            case KisToolKnifeOptionsWidget::Special:
                return getSpecialGapWidth();
            default: // this handles automatic, too...
                return getSpecialGapWidth();
        }
    }

    qreal getWidthHorizontal() {
        return getWidthForType(ui->automaticHorizontalCombobox->currentData().value<KisToolKnifeOptionsWidget::GapWidthType>());
    }

    qreal getWidthVertical() {
        return getWidthForType(ui->automaticVerticalCombobox->currentData().value<KisToolKnifeOptionsWidget::GapWidthType>());
    }

    qreal getWidthDiagonal() {
        return getWidthForType(ui->automaticDiagonalCombobox->currentData().value<KisToolKnifeOptionsWidget::GapWidthType>());
    }



    GutterWidthsConfig getCurrentWidthsConfig(KisToolKnifeOptionsWidget::GapWidthType type) {
        switch(type) {
            case KisToolKnifeOptionsWidget::Thick:
                return GutterWidthsConfig(currentUnit(), resolution, getThickGapWidth(), ui->gutterWidthAngleSpinBox->value());
            case KisToolKnifeOptionsWidget::Thin:
                return GutterWidthsConfig(currentUnit(), resolution, getThinGapWidth(), ui->gutterWidthAngleSpinBox->value());
            case KisToolKnifeOptionsWidget::Special:
                return GutterWidthsConfig(currentUnit(), resolution, getSpecialGapWidth(), ui->gutterWidthAngleSpinBox->value());
            case KisToolKnifeOptionsWidget::Automatic:
                return GutterWidthsConfig(currentUnit(), resolution, getWidthHorizontal(), getWidthVertical(), getWidthDiagonal(), ui->gutterWidthAngleSpinBox->value());
            default:
                return GutterWidthsConfig(currentUnit(), resolution, getSpecialGapWidth(), ui->gutterWidthAngleSpinBox->value());
        }
    }



    KisToolKnifeOptionsWidget::ToolMode getToolMode() {
        if (buttonModeAddGutter && buttonModeAddGutter->isChecked()) {
            return KisToolKnifeOptionsWidget::AddGutter;
        } else if (buttonModeRemoveGutter && buttonModeRemoveGutter->isChecked()) {
            return KisToolKnifeOptionsWidget::RemoveGutter;
        } else if (buttonModeMoveGutterEndPoint && buttonModeMoveGutterEndPoint->isChecked()) {
            return KisToolKnifeOptionsWidget::MoveGutterEndPoint;
        } else {
            // default
            return KisToolKnifeOptionsWidget::AddGutter;
        }
    }

    KoUnit currentUnit() {
        return KoUnit::fromSymbol(ui->thickGapWidth->suffix().trimmed());
    }

    void setUnit(QString unit) {

        KoUnit unitBefore = KoUnit::fromSymbol(ui->thickGapWidth->suffix().trimmed());
        KoUnit unitNow = KoUnit::fromSymbol(unit.trimmed());

        if (unitNow.type() == unitBefore.type()) {
            return;
        }

        ui->thickGapWidth->setSuffix(" " + unitNow.symbol());
        ui->thinGapWidth->setSuffix(" " + unitNow.symbol());
        ui->customGapWidth->setSuffix(" " + unitNow.symbol());

        ui->thickGapWidth->setValue(KoUnit::convertFromUnitToUnit(ui->thickGapWidth->value(), unitBefore, unitNow, resolution));
        ui->thinGapWidth->setValue(KoUnit::convertFromUnitToUnit(ui->thinGapWidth->value(), unitBefore, unitNow, resolution));
        ui->customGapWidth->setValue(KoUnit::convertFromUnitToUnit(ui->customGapWidth->value(), unitBefore, unitNow, resolution));

    }

    void readFromConfig(QString toolId) {
        //
    }
};

KisToolKnifeOptionsWidget::KisToolKnifeOptionsWidget(KisCanvasResourceProvider */*provider*/, QWidget *parent, QString toolId, qreal resolution)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->ui = new Ui_KisToolKnifeOptionsWidget();
    m_d->ui->setupUi(this);

    m_d->toolId = toolId;
    m_d->resolution = resolution;


    //m_d->ui->optionButtonStripToolMode->addButton(KisIconUtils::loadIcon("tool_comic_panel_scissors"));
    //KisOptionButtonStrip *optionButtonStripToolMode =
    //    new KisOptionButtonStrip;
    m_d->buttonModeAddGutter = m_d->ui->optionButtonStripToolMode->addButton(
        KisIconUtils::loadIcon("tool_comic_panel_scissors"));
    m_d->buttonModeRemoveGutter = m_d->ui->optionButtonStripToolMode->addButton(
        KisIconUtils::loadIcon("tool_comic_panel_zipper"));
    m_d->buttonModeMoveGutterEndPoint = m_d->ui->optionButtonStripToolMode->addButton(
        KisIconUtils::loadIcon("tool_comic_panel_move_point"));

    m_d->buttonModeAddGutter->setChecked(true);
    m_d->buttonModeAddGutter->setMinimumSize(QSize(36, 36));
    m_d->buttonModeRemoveGutter->setMinimumSize(QSize(36, 36));
    m_d->buttonModeAddGutter->setIconSize(QSize(28, 28));
    m_d->buttonModeRemoveGutter->setIconSize(QSize(28, 28));
    m_d->buttonModeMoveGutterEndPoint->setMinimumSize(QSize(36, 36));
    m_d->buttonModeMoveGutterEndPoint->setIconSize(QSize(28, 28));
    m_d->ui->unitsCombobox->addItem(i18n("Pixels"), QVariant(" px"));
    m_d->ui->unitsCombobox->addItem(i18n("Millimeters"), QVariant(" mm"));

    m_d->ui->automaticHorizontalCombobox->addItem(i18nc("Thick type of gutter (in comics); keep consistent with the label in the GUI", "Thick"), QVariant(KisToolKnifeOptionsWidget::Thick));
    m_d->ui->automaticHorizontalCombobox->addItem(i18nc("Thin type of gutter (in comics); keep consistent with the label in the GUI", "Thin"), QVariant(KisToolKnifeOptionsWidget::Thin));
    m_d->ui->automaticHorizontalCombobox->addItem(i18nc("Special type of gutter (in comics); keep consistent with the label in the GUI", "Special"), QVariant(KisToolKnifeOptionsWidget::Special));


    m_d->ui->automaticVerticalCombobox->addItem(i18nc("Thick type of gutter (in comics); keep consistent with the label in the GUI", "Thick"), QVariant(KisToolKnifeOptionsWidget::Thick));
    m_d->ui->automaticVerticalCombobox->addItem(i18nc("Thin type of gutter (in comics); keep consistent with the label in the GUI", "Thin"), QVariant(KisToolKnifeOptionsWidget::Thin));
    m_d->ui->automaticVerticalCombobox->addItem(i18nc("Special type of gutter (in comics); keep consistent with the label in the GUI", "Special"), QVariant(KisToolKnifeOptionsWidget::Special));


    m_d->ui->automaticDiagonalCombobox->addItem(i18nc("Thick type of gutter (in comics); keep consistent with the label in the GUI", "Thick"), QVariant(KisToolKnifeOptionsWidget::Thick));
    m_d->ui->automaticDiagonalCombobox->addItem(i18nc("Thin type of gutter (in comics); keep consistent with the label in the GUI", "Thin"), QVariant(KisToolKnifeOptionsWidget::Thin));
    m_d->ui->automaticDiagonalCombobox->addItem(i18nc("Special type of gutter (in comics); keep consistent with the label in the GUI", "Special"), QVariant(KisToolKnifeOptionsWidget::Special));


    m_d->readFromConfig(toolId);
    connect(m_d->ui->unitsCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(unitForWidthChanged(int)));

}



KisToolKnifeOptionsWidget::~KisToolKnifeOptionsWidget()
{
    delete m_d->ui;
    m_d->ui = nullptr;
}

GutterWidthsConfig KisToolKnifeOptionsWidget::getCurrentWidthsConfig()
{
    return m_d->getCurrentWidthsConfig(m_d->getWidthType());
}

KisToolKnifeOptionsWidget::ToolMode KisToolKnifeOptionsWidget::getToolMode()
{
    return m_d->getToolMode();
}

void KisToolKnifeOptionsWidget::unitForWidthChanged(int index)
{
    QString selected = m_d->ui->unitsCombobox->itemData(index).toString();
    m_d->setUnit(selected);
}

