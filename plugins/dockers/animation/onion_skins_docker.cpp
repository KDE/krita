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
#include "kis_node_view_color_scheme.h"
#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include <KoColorSpaceRegistry.h>

#include "kis_equalizer_widget.h"

OnionSkinsDocker::OnionSkinsDocker(QWidget *parent) :
    QDockWidget(i18n("Onion Skins"), parent),
    ui(new Ui::OnionSkinsDocker),
    m_updatesCompressor(300, KisSignalCompressor::FIRST_ACTIVE),
    m_toggleOnionSkinsAction(0)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    KisImageConfig config(true);
    ui->setupUi(mainWidget);

    mainWidget->setContentsMargins(10, 10, 10, 10);


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

    connect(ui->btnBackwardColor, SIGNAL(changed(KoColor)), &m_updatesCompressor, SLOT(start()));
    connect(ui->btnForwardColor, SIGNAL(changed(KoColor)), &m_updatesCompressor, SLOT(start()));
    connect(ui->doubleTintFactor, SIGNAL(valueChanged(qreal)), &m_updatesCompressor, SLOT(start()));

    connect(&m_updatesCompressor, SIGNAL(timeout()),
            SLOT(changed()));


    {
        const bool isShown = config.showAdditionalOnionSkinsSettings();
        ui->btnShowHide->setChecked(isShown);
        connect(ui->btnShowHide, SIGNAL(toggled(bool)), SLOT(slotShowAdditionalSettings(bool)));
        slotShowAdditionalSettings(isShown);
    }

    // create colored checkboxes for onion skin filtering
    KisNodeViewColorScheme scm;
    QPalette filterColorPalette;
    QPixmap iconPixmap(10, 10);

    //iconPixmap.fill(scm.colorLabel(0));
    //ui->colorFilter0_checkbox->setIcon(iconPixmap); // default(no) color

    iconPixmap.fill(scm.colorLabel(1));
    ui->colorFilter1_checkbox->setIcon(QIcon(iconPixmap));

    iconPixmap.fill(scm.colorLabel(2));
    ui->colorFilter2_checkbox->setIcon(QIcon(iconPixmap));

    iconPixmap.fill(scm.colorLabel(3));
    ui->colorFilter3_checkbox->setIcon(QIcon(iconPixmap));

    iconPixmap.fill(scm.colorLabel(4));
    ui->colorFilter4_checkbox->setIcon(QIcon(iconPixmap));

    iconPixmap.fill(scm.colorLabel(5));
    ui->colorFilter5_checkbox->setIcon(QIcon(iconPixmap));

    iconPixmap.fill(scm.colorLabel(6));
    ui->colorFilter6_checkbox->setIcon(QIcon(iconPixmap));

    iconPixmap.fill(scm.colorLabel(7));
    ui->colorFilter7_checkbox->setIcon(QIcon(iconPixmap));

    iconPixmap.fill(scm.colorLabel(8));
    ui->colorFilter8_checkbox->setIcon(QIcon(iconPixmap));


    // assign click events to color filters and group checkbox
    connect(ui->colorFilter0_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter1_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter2_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter3_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter4_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter5_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter6_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter7_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilter8_checkbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
    connect(ui->colorFilterGroupbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));

    loadSettings();
    KisOnionSkinCompositor::instance()->configChanged();

    // this mostly hides the checkboxes since no filtering is done by default
    slotFilteredColorsChanged();

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

void OnionSkinsDocker::setViewManager(KisViewManager *view)
{
    KisActionManager *actionManager = view->actionManager();

    m_toggleOnionSkinsAction = actionManager->createAction("toggle_onion_skin");
    connect(m_toggleOnionSkinsAction, SIGNAL(triggered()), SLOT(slotToggleOnionSkins()));

    slotUpdateIcons();
    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));
}

void OnionSkinsDocker::slotToggleOnionSkins()
{
    m_equalizerWidget->toggleMasterSwitch();
}

void OnionSkinsDocker::slotFilteredColorsChanged()
{
    // what colors are selected to filter??
    QList<int> selectedFilterColors;

    if (ui->colorFilter0_checkbox->isChecked()) selectedFilterColors << 0;
    if (ui->colorFilter1_checkbox->isChecked()) selectedFilterColors << 1;
    if (ui->colorFilter2_checkbox->isChecked()) selectedFilterColors << 2;
    if (ui->colorFilter3_checkbox->isChecked()) selectedFilterColors << 3;
    if (ui->colorFilter4_checkbox->isChecked()) selectedFilterColors << 4;
    if (ui->colorFilter5_checkbox->isChecked()) selectedFilterColors << 5;
    if (ui->colorFilter6_checkbox->isChecked()) selectedFilterColors << 6;
    if (ui->colorFilter7_checkbox->isChecked()) selectedFilterColors << 7;
    if (ui->colorFilter8_checkbox->isChecked()) selectedFilterColors << 8;

    // show all colors if the filter is off and ignore the checkboxes
    if(ui->colorFilterGroupbox->isChecked() == false) {
        selectedFilterColors.clear();
        selectedFilterColors << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8; // show everything
    }

    ui->colorFilter0_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter1_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter2_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter3_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter4_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter5_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter6_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter7_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());
    ui->colorFilter8_checkbox->setVisible(ui->colorFilterGroupbox->isChecked());

    // existing code
    KisOnionSkinCompositor::instance()->setColorLabelFilter(selectedFilterColors);
    KisOnionSkinCompositor::instance()->configChanged();
}

void OnionSkinsDocker::slotUpdateIcons()
{
    if (m_toggleOnionSkinsAction) {
        m_toggleOnionSkinsAction->setIcon(KisIconUtils::loadIcon("onion_skin_options"));
    }
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

    KisImageConfig(false).setShowAdditionalOnionSkinsSettings(value);
}

void OnionSkinsDocker::changed()
{
    KisImageConfig config(false);

    KisEqualizerWidget::EqualizerValues v = m_equalizerWidget->getValues();
    config.setNumberOfOnionSkins(v.maxDistance);

    for (int i = -v.maxDistance; i <= v.maxDistance; i++) {
        config.setOnionSkinOpacity(i, v.value[i] * 255.0 / 100.0);
        config.setOnionSkinState(i, v.state[i]);
    }

    config.setOnionSkinTintFactor(ui->doubleTintFactor->value() * 255.0 / 100.0);
    config.setOnionSkinTintColorBackward(ui->btnBackwardColor->color().toQColor());
    config.setOnionSkinTintColorForward(ui->btnForwardColor->color().toQColor());

    KisOnionSkinCompositor::instance()->configChanged();
}

void OnionSkinsDocker::loadSettings()
{
    KisImageConfig config(true);

    KisSignalsBlocker b(ui->doubleTintFactor,
                        ui->btnBackwardColor,
                        ui->btnForwardColor,
                        m_equalizerWidget);

    ui->doubleTintFactor->setValue(qRound(config.onionSkinTintFactor() * 100.0 / 255));
    KoColor bcol(KoColorSpaceRegistry::instance()->rgb8());
    bcol.fromQColor(config.onionSkinTintColorBackward());
    ui->btnBackwardColor->setColor(bcol);
    bcol.fromQColor(config.onionSkinTintColorForward());
    ui->btnForwardColor->setColor(bcol);

    KisEqualizerWidget::EqualizerValues v;
    v.maxDistance = 10;

    for (int i = -v.maxDistance; i <= v.maxDistance; i++) {
        v.value.insert(i, qRound(config.onionSkinOpacity(i) * 100.0 / 255.0));
        v.state.insert(i, config.onionSkinState(i));
    }

    m_equalizerWidget->setValues(v);
}
