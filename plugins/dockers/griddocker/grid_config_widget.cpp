/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "grid_config_widget.h"
#include "ui_grid_config_widget.h"

#include "kis_grid_config.h"
#include "kis_guides_config.h"
#include "kis_debug.h"
#include "kis_aspect_ratio_locker.h"
#include "kis_int_parse_spin_box.h"

#include <kis_config.h>
#include <kis_config_notifier.h>

#include <QStandardItem>
#include <QStandardItemModel>

struct GridConfigWidget::Private
{
    Private() : guiSignalsBlocked(false) {}

    KisGridConfig gridConfig;
    KisGuidesConfig guidesConfig;
    bool guiSignalsBlocked {false};
};

GridConfigWidget::GridConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridConfigWidget),
    m_d(new Private)
{
    ui->setupUi(this);

    ui->colorMain->setAlphaChannelEnabled(true);
    ui->colorSubdivision->setAlphaChannelEnabled(true);
    ui->colorGuides->setAlphaChannelEnabled(true);


    ui->angleLeftSpinbox->setSuffix(QChar(Qt::Key_degree));
    ui->angleRightSpinbox->setSuffix(QChar(Qt::Key_degree));
    ui->cellSpacingSpinbox->setSuffix(i18n(" px"));


    ui->gridTypeCombobox->addItem(i18n("Rectangle"));
    ui->gridTypeCombobox->addItem(i18n("Isometric"));

    ui->gridTypeCombobox->setCurrentIndex(0); // set to rectangle by default
    slotGridTypeChanged(); // update the UI to hide any elements we don't need


    connect(ui->gridTypeCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotGridTypeChanged()));

    setGridConfig(m_d->gridConfig);
    setGuidesConfig(m_d->guidesConfig);

    // hide offset UI elements if offset is disabled
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->lblXOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->lblYOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->intXOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->intYOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->offsetAspectButton, SLOT(setVisible(bool)));


    ui->lblXOffset->setVisible(false);
    ui->lblYOffset->setVisible(false);
    ui->intXOffset->setVisible(false);
    ui->intYOffset->setVisible(false);
    ui->offsetAspectButton->setVisible(false);

    connect(ui->chkShowGrid, SIGNAL(stateChanged(int)), SLOT(slotGridGuiChanged()));
    connect(ui->chkSnapToGrid, SIGNAL(stateChanged(int)), SLOT(slotGridGuiChanged()));

    connect(ui->chkShowGuides, SIGNAL(stateChanged(int)), SLOT(slotGuidesGuiChanged()));
    connect(ui->chkSnapToGuides, SIGNAL(stateChanged(int)), SLOT(slotGuidesGuiChanged()));
    connect(ui->chkLockGuides, SIGNAL(stateChanged(int)), SLOT(slotGuidesGuiChanged()));

    connect(ui->intSubdivision, SIGNAL(valueChanged(int)), SLOT(slotGridGuiChanged()));
    connect(ui->angleLeftSpinbox, SIGNAL(valueChanged(int)), SLOT(slotGridGuiChanged()));
    connect(ui->angleRightSpinbox, SIGNAL(valueChanged(int)), SLOT(slotGridGuiChanged()));
    connect(ui->cellSpacingSpinbox, SIGNAL(valueChanged(int)), SLOT(slotGridGuiChanged()));

    connect(ui->selectMainStyle, SIGNAL(currentIndexChanged(int)), SLOT(slotGridGuiChanged()));
    connect(ui->colorMain, SIGNAL(changed(QColor)), SLOT(slotGridGuiChanged()));
    connect(ui->selectSubdivisionStyle, SIGNAL(currentIndexChanged(int)), SLOT(slotGridGuiChanged()));
    connect(ui->colorSubdivision, SIGNAL(changed(QColor)), SLOT(slotGridGuiChanged()));
    connect(ui->selectGuidesStyle, SIGNAL(currentIndexChanged(int)), SLOT(slotGuidesGuiChanged()));
    connect(ui->colorGuides, SIGNAL(changed(QColor)), SLOT(slotGuidesGuiChanged()));

    ui->chkOffset->setChecked(false);

    KisAspectRatioLocker *offsetLocker = new KisAspectRatioLocker(this);
    offsetLocker->connectSpinBoxes(ui->intXOffset, ui->intYOffset, ui->offsetAspectButton);

    KisAspectRatioLocker *spacingLocker = new KisAspectRatioLocker(this);
    spacingLocker->connectSpinBoxes(ui->intHSpacing, ui->intVSpacing, ui->spacingAspectButton);

    connect(offsetLocker, SIGNAL(sliderValueChanged()), SLOT(slotGridGuiChanged()));
    connect(offsetLocker, SIGNAL(aspectButtonChanged()), SLOT(slotGridGuiChanged()));

    connect(spacingLocker, SIGNAL(sliderValueChanged()), SLOT(slotGridGuiChanged()));
    connect(spacingLocker, SIGNAL(aspectButtonChanged()), SLOT(slotGridGuiChanged()));

    connect(ui->chkShowRulers,SIGNAL(toggled(bool)),SIGNAL(showRulersChanged(bool)));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotPreferencesUpdated()));
}

GridConfigWidget::~GridConfigWidget()
{
    delete ui;
}

void GridConfigWidget::setGridConfig(const KisGridConfig &value)
{
    KisGridConfig currentConfig = fetchGuiGridConfig();
    if (currentConfig == value) return;

    setGridConfigImpl(value);
}

void GridConfigWidget::setGuidesConfig(const KisGuidesConfig &value)
{
    KisGuidesConfig currentConfig = fetchGuiGuidesConfig();
    if (currentConfig == value) return;

    setGuidesConfigImpl(value);
}

void GridConfigWidget::setGridConfigImpl(const KisGridConfig &value)
{
    m_d->gridConfig = value;
    m_d->guiSignalsBlocked = true;

    if (!m_d->gridConfig.offset().isNull()) {
        ui->chkOffset->setChecked(true);
    }

    ui->offsetAspectButton->setKeepAspectRatio(m_d->gridConfig.offsetAspectLocked());
    ui->spacingAspectButton->setKeepAspectRatio(m_d->gridConfig.spacingAspectLocked());
    ui->chkShowGrid->setChecked(m_d->gridConfig.showGrid());
    ui->intHSpacing->setMaximum(std::numeric_limits<int>::max());
    ui->intVSpacing->setMaximum(std::numeric_limits<int>::max());
    ui->intHSpacing->setValue(m_d->gridConfig.spacing().x());
    ui->intVSpacing->setValue(m_d->gridConfig.spacing().y());
    ui->intXOffset->setValue(m_d->gridConfig.offset().x());
    ui->intYOffset->setValue(m_d->gridConfig.offset().y());
    ui->intSubdivision->setValue(m_d->gridConfig.subdivision());
    ui->chkSnapToGrid->setChecked(m_d->gridConfig.snapToGrid());
    ui->angleLeftSpinbox->setValue(m_d->gridConfig.angleLeft());
    ui->angleRightSpinbox->setValue(m_d->gridConfig.angleRight());
    ui->cellSpacingSpinbox->setValue(m_d->gridConfig.cellSpacing());

    ui->selectMainStyle->setCurrentIndex(int(m_d->gridConfig.lineTypeMain()));
    ui->selectSubdivisionStyle->setCurrentIndex(int(m_d->gridConfig.lineTypeSubdivision()));
    ui->gridTypeCombobox->setCurrentIndex(m_d->gridConfig.gridType());

    ui->colorMain->setColor(m_d->gridConfig.colorMain());
    ui->colorSubdivision->setColor(m_d->gridConfig.colorSubdivision());

    m_d->guiSignalsBlocked = false;

    emit gridValueChanged();
}

void GridConfigWidget::setGuidesConfigImpl(const KisGuidesConfig &value)
{
    m_d->guidesConfig = value;
    m_d->guiSignalsBlocked = true;

    ui->chkShowGuides->setChecked(m_d->guidesConfig.showGuides());
    ui->chkSnapToGuides->setChecked(m_d->guidesConfig.snapToGuides());
    ui->chkLockGuides->setChecked(m_d->guidesConfig.lockGuides());

    ui->selectGuidesStyle->setCurrentIndex(int(m_d->guidesConfig.guidesLineType()));
    ui->colorGuides->setColor(m_d->guidesConfig.guidesColor());

    m_d->guiSignalsBlocked = false;

    emit guidesValueChanged();
}

KisGridConfig GridConfigWidget::gridConfig() const
{
    return m_d->gridConfig;
}

KisGuidesConfig GridConfigWidget::guidesConfig() const
{
    return m_d->guidesConfig;
}

KisGridConfig GridConfigWidget::fetchGuiGridConfig() const
{
    KisGridConfig config;

    config.setShowGrid(ui->chkShowGrid->isChecked());
    config.setSnapToGrid(ui->chkSnapToGrid->isChecked());

    QPoint pt;

    pt.rx() = ui->intHSpacing->value();
    pt.ry() = ui->intVSpacing->value();
    config.setSpacing(pt);

    pt.rx() = ui->intXOffset->value();
    pt.ry() = ui->intYOffset->value();
    config.setOffset(pt);

    config.setSubdivision(ui->intSubdivision->value());
    config.setAngleLeft(ui->angleLeftSpinbox->value());
    config.setAngleRight(ui->angleRightSpinbox->value());
    config.setCellSpacing(ui->cellSpacingSpinbox->value());
    config.setGridType(ui->gridTypeCombobox->currentIndex());

    config.setOffsetAspectLocked(ui->offsetAspectButton->keepAspectRatio());
    config.setSpacingAspectLocked(ui->spacingAspectButton->keepAspectRatio());

    config.setLineTypeMain(KisGridConfig::LineTypeInternal(ui->selectMainStyle->currentIndex()));
    config.setLineTypeSubdivision(KisGridConfig::LineTypeInternal(ui->selectSubdivisionStyle->currentIndex()));

    config.setColorMain(ui->colorMain->color());
    config.setColorSubdivision(ui->colorSubdivision->color());

    return config;
}

KisGuidesConfig GridConfigWidget::fetchGuiGuidesConfig() const
{
    KisGuidesConfig config = m_d->guidesConfig;

    config.setShowGuides(ui->chkShowGuides->isChecked());
    config.setSnapToGuides(ui->chkSnapToGuides->isChecked());
    config.setLockGuides(ui->chkLockGuides->isChecked());

    config.setGuidesLineType(KisGuidesConfig::LineTypeInternal(ui->selectGuidesStyle->currentIndex()));
    config.setGuidesColor(ui->colorGuides->color());

    return config;
}

void GridConfigWidget::slotGridGuiChanged()
{
    if (m_d->guiSignalsBlocked) return;

    KisGridConfig currentConfig = fetchGuiGridConfig();
    if (currentConfig == m_d->gridConfig) return;

    setGridConfigImpl(currentConfig);
}

void GridConfigWidget::slotPreferencesUpdated()
{
    KisConfig cfg(true);
    enableIsometricGrid(cfg.useOpenGL()); // Isometric view needs OpenGL
}

void GridConfigWidget::slotGuidesGuiChanged()
{
    if (m_d->guiSignalsBlocked) return;

    KisGuidesConfig currentConfig = fetchGuiGuidesConfig();
    if (currentConfig == m_d->guidesConfig) return;

    setGuidesConfigImpl(currentConfig);
}

void GridConfigWidget::slotGridTypeChanged() {


    bool showRectangleControls = ui->gridTypeCombobox->currentIndex() == 0;


    // specific rectangle UI controls
    ui->lblXSpacing->setVisible(showRectangleControls);
    ui->lblYSpacing->setVisible(showRectangleControls);
    ui->intHSpacing->setVisible(showRectangleControls);
    ui->intVSpacing->setVisible(showRectangleControls);
    ui->spacingAspectButton->setVisible(showRectangleControls);

    ui->lblSubdivision->setVisible(showRectangleControls);
    ui->intSubdivision->setVisible(showRectangleControls);
    ui->lblSubdivisionStyle->setVisible(showRectangleControls);
    ui->selectSubdivisionStyle->setVisible(showRectangleControls);
    ui->colorSubdivision->setVisible(showRectangleControls);


    // specific isometric UI controls
    ui->leftAngleLabel->setVisible(!showRectangleControls);
    ui->rightAngleLabel->setVisible(!showRectangleControls);
    ui->angleLeftSpinbox->setVisible(!showRectangleControls);
    ui->angleRightSpinbox->setVisible(!showRectangleControls);
    ui->cellSpacingLabel->setVisible(!showRectangleControls);
    ui->cellSpacingSpinbox->setVisible(!showRectangleControls);



    // disable snapping for isometric grid type for now
    // remember if we had snapping enabled if it was on the rectangule mode
    if (!showRectangleControls) {
        m_isGridEnabled = ui->chkSnapToGrid->isChecked();
        ui->chkSnapToGrid->setEnabled(false);
        ui->chkSnapToGrid->setChecked(false);
    } else {
        ui->chkSnapToGrid->setEnabled(true);
        ui->chkSnapToGrid->setChecked(m_isGridEnabled);
    }



    slotGridGuiChanged();
}

bool GridConfigWidget::showRulers() const
{
    return ui->chkShowRulers->isChecked();
}

void GridConfigWidget::enableIsometricGrid(bool value)
{
    m_isIsometricGridEnabled = value;

    // Isometric grids disabled if OpenGL is disabled
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->gridTypeCombobox->model());
    QStandardItem *item = model->item(1); // isometric option

   // item->setFlags(m_isIsometricGridEnabled ? item->flags() & ~Qt::ItemIsEnabled:
     //                                         item->flags() | Qt::ItemIsEnabled);


     item->setEnabled(m_isIsometricGridEnabled);

    if (m_isIsometricGridEnabled) {
        item->setText(i18n("Isometric"));
    } else {
        item->setText(i18n("Isometric (requires OpenGL)"));

        // change drop down index to Rectangular in case it was previously set to isometric
        ui->gridTypeCombobox->setCurrentIndex(0);
    }
}

void GridConfigWidget::setShowRulers(bool value)
{
    ui->chkShowRulers->setChecked(value);
}

