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

#include <kis_node.h>
#include <kis_shape_layer.h>


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

    KisToolKnifeOptionsWidget::GapWidthType getCurrentWidthType()
    {
        if (ui->thickGapWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Thick;
        } else if (ui->thinGapWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Thin;
        } else if (ui->specialGapWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Special;
        } else { // if (ui->automaticGapWidthRadioButton->isChecked())
            return KisToolKnifeOptionsWidget::Automatic;
        }
    }

    KisToolKnifeOptionsWidget::GapWidthType widthTypeFromString(QString type) {
        if (type == "thick") {
            return KisToolKnifeOptionsWidget::Thick;
        } else if (type == "thin") {
            return KisToolKnifeOptionsWidget::Thin;
        } else if (type == "special") {
            return KisToolKnifeOptionsWidget::Special;
        } else if (type == "automatic") {
            return KisToolKnifeOptionsWidget::Automatic;
        } else {
            // default
            return KisToolKnifeOptionsWidget::Thick;
        }
    }

    QString widthTypeToString(KisToolKnifeOptionsWidget::GapWidthType type) {
        switch(type) {
            case KisToolKnifeOptionsWidget::Thick:
                return "thick";
                break;
            case KisToolKnifeOptionsWidget::Thin:
                return "thin";
                break;
            case KisToolKnifeOptionsWidget::Special:
                return "special";
                break;
            case KisToolKnifeOptionsWidget::Automatic:
                return "automatic";
                break;
            default:
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false, "thick");
        }
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false, "thick");
        return "thick";
    }



    void setWidthTypeFromString(QString typeStr) {
        KisToolKnifeOptionsWidget::GapWidthType type = widthTypeFromString(typeStr);
        if (type == KisToolKnifeOptionsWidget::Thick) {
            ui->thickGapWidthRadioButton->setChecked(true);
        } else if (type == KisToolKnifeOptionsWidget::Thin) {
            ui->thinGapWidthRadioButton->setChecked(true);
        } else if (type == KisToolKnifeOptionsWidget::Special) {
            ui->specialGapWidthRadioButton->setChecked(true);
        } else if (type == KisToolKnifeOptionsWidget::Automatic) {
            ui->automaticGapWidthRadioButton->setChecked(true);
        } else {
            // default
            ui->thickGapWidthRadioButton->setChecked(true);
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
                return GutterWidthsConfig(getCurrentUnit(), resolution, getThickGapWidth(), ui->gutterWidthAngleSpinBox->value());
            case KisToolKnifeOptionsWidget::Thin:
                return GutterWidthsConfig(getCurrentUnit(), resolution, getThinGapWidth(), ui->gutterWidthAngleSpinBox->value());
            case KisToolKnifeOptionsWidget::Special:
                return GutterWidthsConfig(getCurrentUnit(), resolution, getSpecialGapWidth(), ui->gutterWidthAngleSpinBox->value());
            case KisToolKnifeOptionsWidget::Automatic:
                return GutterWidthsConfig(getCurrentUnit(), resolution, getWidthHorizontal(), getWidthVertical(), getWidthDiagonal(), ui->gutterWidthAngleSpinBox->value());
            default:
                return GutterWidthsConfig(getCurrentUnit(), resolution, getSpecialGapWidth(), ui->gutterWidthAngleSpinBox->value());
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

    KoUnit getCurrentUnit() {
        return KoUnit::fromSymbol(ui->thickGapWidth->suffix().trimmed());
    }

    void setUnit(QString unit) {
        if (ui->unitsCombobox->currentData() != unit) {
            int index = ui->unitsCombobox->findData(unit);
            if (index != -1) {
                ui->unitsCombobox->setCurrentIndex(index);
            }
        }

        KoUnit unitBefore = KoUnit::fromSymbol(ui->thickGapWidth->suffix().trimmed());
        bool success;
        KoUnit unitNow = KoUnit::fromSymbol(unit.trimmed(), &success);

        if (!success || unit.trimmed() == "" || unitNow.type() == unitBefore.type()) {
            return;
        }

        ui->thickGapWidth->setSuffix(" " + unitNow.symbol());
        ui->thinGapWidth->setSuffix(" " + unitNow.symbol());
        ui->customGapWidth->setSuffix(" " + unitNow.symbol());

        ui->thickGapWidth->setValue(KoUnit::convertFromUnitToUnit(ui->thickGapWidth->value(), unitBefore, unitNow, resolution));
        ui->thinGapWidth->setValue(KoUnit::convertFromUnitToUnit(ui->thinGapWidth->value(), unitBefore, unitNow, resolution));
        ui->customGapWidth->setValue(KoUnit::convertFromUnitToUnit(ui->customGapWidth->value(), unitBefore, unitNow, resolution));

    }

    void setTypeToCombobox(KisToolKnifeOptionsWidget::GapWidthType type, QComboBox* combobox) {
        int index = combobox->findData(type);
        if (index >= 0) {
            combobox->setCurrentIndex(index);
        }
    }

    void readFromConfig(QString groupId) {

        KConfigGroup configGroup = KSharedConfig::openConfig()->group(groupId);

        QString unitSymbol = configGroup.readEntry("gutter_unit_symbol", "px");
        bool conversionSuccess;
        KoUnit::fromSymbol(unitSymbol, &conversionSuccess);
        if (!conversionSuccess) {
            unitSymbol = "px";
        }
        setUnit(unitSymbol);

        setWidthTypeFromString(configGroup.readEntry("current_gutter_width_type", "thick"));

        ui->thickGapWidth->setValue(configGroup.readEntry("thick_gutter_width", 40.0f));
        ui->thinGapWidth->setValue(configGroup.readEntry("thin_gutter_width", 15.0f));
        ui->customGapWidth->setValue(configGroup.readEntry("special_gutter_width", 70.0f));

        ui->gutterWidthAngleSpinBox->setValue(configGroup.readEntry("automatic_gutter_angle", 2.0f));

        KisToolKnifeOptionsWidget::GapWidthType horizontalType = widthTypeFromString(configGroup.readEntry("automatic_horizontal_type", "thick"));
        KisToolKnifeOptionsWidget::GapWidthType verticalType = widthTypeFromString(configGroup.readEntry("automatic_vertical_type", "thin"));
        KisToolKnifeOptionsWidget::GapWidthType diagonalType = widthTypeFromString(configGroup.readEntry("automatic_diagonal_type", "thin"));

        setTypeToCombobox(horizontalType, ui->automaticHorizontalCombobox);
        setTypeToCombobox(verticalType, ui->automaticVerticalCombobox);
        setTypeToCombobox(diagonalType, ui->automaticDiagonalCombobox);

    }

    void writeToConfig(QString groupId) {

        KConfigGroup configGroup = KSharedConfig::openConfig()->group(groupId);

        configGroup.writeEntry("gutter_unit_symbol", getCurrentUnit().symbol());
        configGroup.writeEntry("current_gutter_width_type", widthTypeToString(getCurrentWidthType()));

        configGroup.writeEntry("thick_gutter_width", ui->thickGapWidth->value());
        configGroup.writeEntry("thin_gutter_width", ui->thinGapWidth->value());
        configGroup.writeEntry("special_gutter_width", ui->customGapWidth->value());

        configGroup.writeEntry("automatic_gutter_angle", ui->gutterWidthAngleSpinBox->value());

        configGroup.writeEntry("automatic_horizontal_type", widthTypeToString(ui->automaticHorizontalCombobox->currentData().value<KisToolKnifeOptionsWidget::GapWidthType>()));
        configGroup.writeEntry("automatic_vertical_type", widthTypeToString(ui->automaticVerticalCombobox->currentData().value<KisToolKnifeOptionsWidget::GapWidthType>()));
        configGroup.writeEntry("automatic_diagonal_type", widthTypeToString(ui->automaticDiagonalCombobox->currentData().value<KisToolKnifeOptionsWidget::GapWidthType>()));
    }

};

KisToolKnifeOptionsWidget::KisToolKnifeOptionsWidget(KisCanvasResourceProvider *provider, QWidget *parent, QString toolId, qreal resolution)
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
        KisIconUtils::loadIcon("tool_comic_panel_cut"));
    m_d->buttonModeRemoveGutter = m_d->ui->optionButtonStripToolMode->addButton(
        KisIconUtils::loadIcon("tool_comic_panel_merge"));
    //m_d->buttonModeMoveGutterEndPoint = m_d->ui->optionButtonStripToolMode->addButton(
    //    KisIconUtils::loadIcon("tool_comic_panel_move_point"));

    m_d->buttonModeAddGutter->setChecked(true);
    m_d->buttonModeAddGutter->setMinimumSize(QSize(20, 20));
    m_d->buttonModeRemoveGutter->setMinimumSize(QSize(20, 20));
    m_d->buttonModeAddGutter->setIconSize(QSize(16, 16));
    m_d->buttonModeRemoveGutter->setIconSize(QSize(16, 16));

    //m_d->buttonModeMoveGutterEndPoint->setMinimumSize(QSize(36, 36));
    //m_d->buttonModeMoveGutterEndPoint->setIconSize(QSize(28, 28));


    m_d->ui->unitsCombobox->addItem(i18n("Pixels"), QVariant("px"));
    m_d->ui->unitsCombobox->addItem(i18n("Millimeters"), QVariant("mm"));

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

    connect(provider, SIGNAL(sigNodeChanged(const KisNodeSP)), this, SLOT(currentNodeChanged(const KisNodeSP)));
    connect(m_d->buttonModeAddGutter, SIGNAL(clicked()), this, SLOT(modeChanged()));
    connect(m_d->buttonModeRemoveGutter, SIGNAL(clicked()), this, SLOT(modeChanged()));

    connect(m_d->ui->automaticGapWidthRadioButton, SIGNAL(toggled(bool)), this, SLOT(currentWidthSystemChanged()));

    modeChanged();
    currentNodeChanged(provider->currentNode());
    currentWidthSystemChanged();

}



KisToolKnifeOptionsWidget::~KisToolKnifeOptionsWidget()
{
    m_d->writeToConfig(m_d->toolId);
    delete m_d->ui;
    m_d->ui = nullptr;
}

GutterWidthsConfig KisToolKnifeOptionsWidget::getCurrentWidthsConfig()
{
    return m_d->getCurrentWidthsConfig(m_d->getCurrentWidthType());
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

void KisToolKnifeOptionsWidget::currentNodeChanged(const KisNodeSP node)
{
    ENTER_FUNCTION();
    const KisShapeLayer *shapeLayer = qobject_cast<const KisShapeLayer*>(node.data());
    if (!shapeLayer) {
        m_d->ui->gapOptionsWidget->setVisible(false);
        m_d->ui->warningLabel->setVisible(true);

    } else {
        m_d->ui->gapOptionsWidget->setVisible(true);
        m_d->ui->warningLabel->setVisible(false);
    }
}

void KisToolKnifeOptionsWidget::modeChanged()
{
    KisToolKnifeOptionsWidget::ToolMode mode = getToolMode();
    switch(mode) {
        case AddGutter:
            m_d->ui->gapOptionsWidget->setEnabled(true);
        break;
        case RemoveGutter:
            m_d->ui->gapOptionsWidget->setEnabled(false);
        break;

    }
}

void KisToolKnifeOptionsWidget::currentWidthSystemChanged()
{

    ENTER_FUNCTION();

    GapWidthType gapConfig = m_d->getCurrentWidthType();
    switch(gapConfig) {
    case Thin:
    case Thick:
    case Special:
        m_d->ui->automaticDiagonalCombobox->setEnabled(false);
        m_d->ui->automaticVerticalCombobox->setEnabled(false);
        m_d->ui->automaticHorizontalCombobox->setEnabled(false);
        m_d->ui->gutterWidthAngleSpinBox->setEnabled(false);
        m_d->ui->angleLabel->setEnabled(false);
        m_d->ui->horizontalLabel->setEnabled(false);
        m_d->ui->verticalLabel->setEnabled(false);
        m_d->ui->diagonalLabel->setEnabled(false);

        break;
    case Automatic:
        m_d->ui->automaticDiagonalCombobox->setEnabled(true);
        m_d->ui->automaticVerticalCombobox->setEnabled(true);
        m_d->ui->automaticHorizontalCombobox->setEnabled(true);
        m_d->ui->gutterWidthAngleSpinBox->setEnabled(true);
        m_d->ui->horizontalLabel->setEnabled(true);
        m_d->ui->verticalLabel->setEnabled(true);
        m_d->ui->diagonalLabel->setEnabled(true);
        m_d->ui->angleLabel->setEnabled(true);
        break;
    }
}

