/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "onion_skins_docker.h"
#include "ui_onion_skins_docker.h"

#include <QSlider>
#include <QFrame>
#include <QGridLayout>

#include "kis_icon_utils.h"
#include "kis_image_config.h"
#include "kis_onion_skin_compositor.h"
#include "kis_signals_blocker.h"

#include "kis_equalizer_widget.h"


static const int MAX_SKIN_COUNT = 10;

OnionSkinsDocker::OnionSkinsDocker(QWidget *parent) :
    QDockWidget(i18n("Onion Skins"), parent),
    ui(new Ui::OnionSkinsDocker),
    m_updatesCompressor(300, KisSignalCompressor::FIRST_ACTIVE)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    KisImageConfig config;
    ui->setupUi(mainWidget);

    ui->doubleTintFactor->setMinimum(0);
    ui->doubleTintFactor->setMaximum(100);
    ui->doubleTintFactor->setPrefix(i18n("Tint: "));
    ui->doubleTintFactor->setSuffix("%");

    ui->btnBackwardColor->setToolTip(i18n("Tint color for past frames"));
    ui->btnForwardColor->setToolTip(i18n("Tint color for future frames"));


    QVBoxLayout *layout = ui->slidersLayout;

    m_equalizerWidget = new KisEqualizerWidget(10, this);
    connect(m_equalizerWidget, SIGNAL(sigConfigChanged()), &m_updatesCompressor, SLOT(start()));
    layout->addWidget(m_equalizerWidget, 1);

    connect(ui->btnBackwardColor, SIGNAL(changed(QColor)), &m_updatesCompressor, SLOT(start()));
    connect(ui->btnForwardColor, SIGNAL(changed(QColor)), &m_updatesCompressor, SLOT(start()));
    connect(ui->doubleTintFactor, SIGNAL(valueChanged(qreal)), &m_updatesCompressor, SLOT(start()));

    connect(&m_updatesCompressor, SIGNAL(timeout()),
            SLOT(changed()));


    {
        const bool isShown = config.showAdditionalOnionSkinsSettings();
        ui->btnShowHide->setChecked(isShown);
        connect(ui->btnShowHide, SIGNAL(toggled(bool)), SLOT(slotShowAdditionalSettings(bool)));
        slotShowAdditionalSettings(isShown);
    }

    loadSettings();
    KisOnionSkinCompositor::instance()->configChanged();

    resize(sizeHint());
}

OnionSkinsDocker::~OnionSkinsDocker()
{
    delete ui;
}

void OnionSkinsDocker::setCanvas(KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);
}

void OnionSkinsDocker::unsetCanvas()
{
}

void OnionSkinsDocker::setMainWindow(KisViewManager *kisview)
{
    Q_UNUSED(kisview);
}

void OnionSkinsDocker::slotShowAdditionalSettings(bool value)
{
    ui->lblPrevColor->setVisible(value);
    ui->lblNextColor->setVisible(value);

    ui->btnBackwardColor->setVisible(value);
    ui->btnForwardColor->setVisible(value);

    ui->doubleTintFactor->setVisible(value);


    QIcon icon = KisIconUtils::loadIcon(value ? "arrow-down" : "arrow-up");
    ui->btnShowHide->setIcon(icon);

    KisImageConfig config;
    config.setShowAdditionalOnionSkinsSettings(value);
}

void OnionSkinsDocker::changed()
{
    KisImageConfig config;

    KisEqualizerWidget::EqualizerValues v = m_equalizerWidget->getValues();
    config.setNumberOfOnionSkins(v.maxDistance);

    for (int i = -v.maxDistance; i <= v.maxDistance; i++) {
        config.setOnionSkinOpacity(i, v.value[i] * 255.0 / 100.0);
        config.setOnionSkinState(i, v.state[i]);
    }

    config.setOnionSkinTintFactor(ui->doubleTintFactor->value() * 255.0 / 100.0);
    config.setOnionSkinTintColorBackward(ui->btnBackwardColor->color());
    config.setOnionSkinTintColorForward(ui->btnForwardColor->color());

    KisOnionSkinCompositor::instance()->configChanged();
}

void OnionSkinsDocker::loadSettings()
{
    KisImageConfig config;

    KisSignalsBlocker b(ui->doubleTintFactor,
                        ui->btnBackwardColor,
                        ui->btnForwardColor,
                        m_equalizerWidget);

    ui->doubleTintFactor->setValue(config.onionSkinTintFactor() * 100.0 / 255);
    ui->btnBackwardColor->setColor(config.onionSkinTintColorBackward());
    ui->btnForwardColor->setColor(config.onionSkinTintColorForward());

    KisEqualizerWidget::EqualizerValues v;
    v.maxDistance = 10;

    for (int i = -v.maxDistance; i <= v.maxDistance; i++) {
        v.value.insert(i, config.onionSkinOpacity(i) * 100.0 / 255.0);
        v.state.insert(i, config.onionSkinState(i));
    }

    m_equalizerWidget->setValues(v);
}
