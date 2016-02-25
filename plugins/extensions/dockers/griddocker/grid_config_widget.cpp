/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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
#include "kis_debug.h"
#include "kis_aspect_ratio_locker.h"


struct GridConfigWidget::Private
{
    Private() : guiSignalsBlocked(false) {}

    KisGridConfig config;
    bool guiSignalsBlocked;
};

GridConfigWidget::GridConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridConfigWidget),
    m_d(new Private)
{
    ui->setupUi(this);

    ui->colorMain->setAlphaChannelEnabled(true);
    ui->colorSubdivision->setAlphaChannelEnabled(true);

    setGridConfig(m_d->config);

    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->lblXOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->lblYOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->intXOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->intYOffset, SLOT(setVisible(bool)));
    connect(ui->chkOffset, SIGNAL(toggled(bool)), ui->offsetAspectButton, SLOT(setVisible(bool)));

    connect(ui->chkStyle, SIGNAL(toggled(bool)), ui->lblMainStyle, SLOT(setVisible(bool)));
    connect(ui->chkStyle, SIGNAL(toggled(bool)), ui->selectMainStyle, SLOT(setVisible(bool)));
    connect(ui->chkStyle, SIGNAL(toggled(bool)), ui->colorMain, SLOT(setVisible(bool)));
    connect(ui->chkStyle, SIGNAL(toggled(bool)), ui->lblSubdivisionStyle, SLOT(setVisible(bool)));
    connect(ui->chkStyle, SIGNAL(toggled(bool)), ui->selectSubdivisionStyle, SLOT(setVisible(bool)));
    connect(ui->chkStyle, SIGNAL(toggled(bool)), ui->colorSubdivision, SLOT(setVisible(bool)));

    connect(ui->chkShowGrid, SIGNAL(stateChanged(int)), SLOT(slotGuiChanged()));
    connect(ui->chkSnapToGrid, SIGNAL(stateChanged(int)), SLOT(slotGuiChanged()));

    connect(ui->intHSpacing, SIGNAL(valueChanged(int)), SLOT(slotGuiChanged()));
    connect(ui->intVSpacing, SIGNAL(valueChanged(int)), SLOT(slotGuiChanged()));
    connect(ui->spacingAspectButton, SIGNAL(keepAspectRatioChanged(bool)), SLOT(slotGuiChanged()));

    connect(ui->intXOffset, SIGNAL(valueChanged(int)), SLOT(slotGuiChanged()));
    connect(ui->intYOffset, SIGNAL(valueChanged(int)), SLOT(slotGuiChanged()));
    connect(ui->offsetAspectButton, SIGNAL(keepAspectRatioChanged(bool)), SLOT(slotGuiChanged()));

    connect(ui->intSubdivision, SIGNAL(valueChanged(int)), SLOT(slotGuiChanged()));

    connect(ui->selectMainStyle, SIGNAL(currentIndexChanged(int)), SLOT(slotGuiChanged()));
    connect(ui->colorMain, SIGNAL(changed(const QColor&)), SLOT(slotGuiChanged()));
    connect(ui->selectSubdivisionStyle, SIGNAL(currentIndexChanged(int)), SLOT(slotGuiChanged()));
    connect(ui->colorSubdivision, SIGNAL(changed(const QColor&)), SLOT(slotGuiChanged()));

    ui->chkStyle->setChecked(false);
    ui->chkOffset->setChecked(false);

    KisAspectRatioLocker *offsetLocker = new KisAspectRatioLocker(this);
    offsetLocker->connectSpinBoxes(ui->intXOffset, ui->intYOffset, ui->offsetAspectButton);

    KisAspectRatioLocker *spacingLocker = new KisAspectRatioLocker(this);
    spacingLocker->connectSpinBoxes(ui->intHSpacing, ui->intVSpacing, ui->spacingAspectButton);
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

void GridConfigWidget::setGridConfigImpl(const KisGridConfig &value)
{
    m_d->config = value;
    m_d->guiSignalsBlocked = true;

    ui->offsetAspectButton->setKeepAspectRatio(m_d->config.offsetAspectLocked());
    ui->spacingAspectButton->setKeepAspectRatio(m_d->config.spacingAspectLocked());
    ui->chkShowGrid->setChecked(m_d->config.showGrid());
    ui->chkSnapToGrid->setChecked(m_d->config.snapToGrid());
    ui->intHSpacing->setValue(m_d->config.spacing().x());
    ui->intVSpacing->setValue(m_d->config.spacing().y());
    ui->intXOffset->setValue(m_d->config.offset().x());
    ui->intYOffset->setValue(m_d->config.offset().y());
    ui->intSubdivision->setValue(m_d->config.subdivision());

    ui->selectMainStyle->setCurrentIndex(int(m_d->config.lineTypeMain()));
    ui->selectSubdivisionStyle->setCurrentIndex(int(m_d->config.lineTypeSubdivision()));

    ui->colorMain->setColor(m_d->config.colorMain());
    ui->colorSubdivision->setColor(m_d->config.colorSubdivision());

    m_d->guiSignalsBlocked = false;

    emit valueChanged();
}

KisGridConfig GridConfigWidget::gridConfig() const
{
    return m_d->config;
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

    config.setOffsetAspectLocked(ui->offsetAspectButton->keepAspectRatio());
    config.setSpacingAspectLocked(ui->spacingAspectButton->keepAspectRatio());

    config.setLineTypeMain(KisGridConfig::LineTypeInternal(ui->selectMainStyle->currentIndex()));
    config.setLineTypeSubdivision(KisGridConfig::LineTypeInternal(ui->selectSubdivisionStyle->currentIndex()));

    config.setColorMain(ui->colorMain->color());
    config.setColorSubdivision(ui->colorSubdivision->color());

    return config;
}

void GridConfigWidget::slotGuiChanged()
{
    if (m_d->guiSignalsBlocked) return;

    KisGridConfig currentConfig = fetchGuiGridConfig();
    if (currentConfig == m_d->config) return;

    setGridConfigImpl(currentConfig);
}
