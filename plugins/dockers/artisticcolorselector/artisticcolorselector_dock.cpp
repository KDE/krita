/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include <kis_debug.h>

#include <klocalizedstring.h>
#include <KoCanvasResourceProvider.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerObserver.h>
#include <KoResourceServerAdapter.h>
#include <KoCanvasBase.h>
#include <kis_canvas2.h>
#include <KoColor.h>
#include <resources/KoGamutMask.h>
#include <kis_icon_utils.h>
#include <KisPart.h>
#include <kis_shape_layer.h>
#include <kis_types.h>
#include <KisDocument.h>
#include <kis_node_selection_adapter.h>
#include <kis_group_layer.h>
#include <KisView.h>
#include <KoResourceItemChooser.h>
#include <kis_display_color_converter.h>

#include <QWidget>
#include <QMenu>
#include <QButtonGroup>
#include <QRegExpValidator>
#include <QRegExp>
#include <QFileInfo>

#include "artisticcolorselector_dock.h"
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_arcs_constants.h>
#include <KisGamutMaskToolbar.h>

#include "ui_wdgArtisticColorSelector.h"
#include "ui_wdgARCSSettings.h"
#include "ui_wdgWheelPreferencesPopup.h"

class KisMainWindow;

struct ArtisticColorSelectorUI: public QWidget, public Ui_wdgArtisticColorSelector
{
    ArtisticColorSelectorUI() {
        setupUi(this);
    }
};

struct ARCSSettingsUI: public QWidget, public Ui_wdgARCSSettings
{
    ARCSSettingsUI() {
        setupUi(this);
    }
};

struct WheelPreferencesPopupUI: public QWidget, public Ui_wdgWheelPreferencesPopup
{
    WheelPreferencesPopupUI() {
        setupUi(this);
    }
};


ArtisticColorSelectorDock::ArtisticColorSelectorDock()
    : QDockWidget(i18n("Artistic Color Selector"))
    , m_canvas(nullptr)
    , m_resourceProvider(0)
    , m_selectedMask(nullptr)
{
    setEnabled(false);

    m_hsxButtons    = new QButtonGroup();
    m_preferencesUI = new ARCSSettingsUI();
    m_wheelPrefsUI  = new WheelPreferencesPopupUI();
    m_selectorUI    = new ArtisticColorSelectorUI();

    QPixmap hueStepsPixmap = KisIconUtils::loadIcon("wheel-sectors").pixmap(16,16);
    QPixmap saturationStepsPixmap = KisIconUtils::loadIcon("wheel-rings").pixmap(16,16);
    QPixmap valueScaleStepsPixmap = KisIconUtils::loadIcon("wheel-light").pixmap(16,16);
    QIcon infinityIcon = KisIconUtils::loadIcon("infinity");
    m_infinityPixmap = infinityIcon.pixmap(16,16);

    m_selectorUI->colorSelector->loadSettings();

    m_selectorUI->bnWheelPrefs->setIcon(KisIconUtils::loadIcon("wheel-sectors"));
    m_selectorUI->bnWheelPrefs->setPopupWidget(m_wheelPrefsUI);

    m_selectorUI->bnDockerPrefs->setPopupWidget(m_preferencesUI);
    m_selectorUI->bnDockerPrefs->setIcon(KisIconUtils::loadIcon("configure"));

    //preferences
    m_hsxButtons->addButton(m_preferencesUI->bnHsy, KisColor::HSY);
    m_hsxButtons->addButton(m_preferencesUI->bnHsi, KisColor::HSI);
    m_hsxButtons->addButton(m_preferencesUI->bnHsl, KisColor::HSL);
    m_hsxButtons->addButton(m_preferencesUI->bnHsv, KisColor::HSV);

    m_wheelPrefsUI->bnInverseSat->setChecked(m_selectorUI->colorSelector->isSaturationInverted());

    m_wheelPrefsUI->labelHueSteps->setPixmap(hueStepsPixmap);
    m_wheelPrefsUI->labelSaturationSteps->setPixmap(saturationStepsPixmap);
    m_wheelPrefsUI->labelValueScaleSteps->setPixmap(valueScaleStepsPixmap);

    m_wheelPrefsUI->numHueSteps->setRange(MIN_NUM_UI_HUE_PIECES, MAX_NUM_HUE_PIECES);
    m_wheelPrefsUI->numSaturationSteps->setRange(MIN_NUM_SATURATION_RINGS, MAX_NUM_SATURATION_RINGS);
    m_wheelPrefsUI->numValueScaleSteps->setRange(MIN_NUM_UI_LIGHT_PIECES, MAX_NUM_LIGHT_PIECES);

    m_wheelPrefsUI->bnInfHueSteps->setIcon(infinityIcon);
    m_wheelPrefsUI->bnInfValueScaleSteps->setIcon(infinityIcon);

    m_wheelPrefsUI->bnInfHueSteps->setToolTip(i18n("Continuous Mode"));
    m_wheelPrefsUI->bnInfValueScaleSteps->setToolTip(i18n("Continuous Mode"));

    int selectorHueSteps = m_selectorUI->colorSelector->getNumPieces();
    if (selectorHueSteps == 1) {
        m_wheelPrefsUI->bnInfHueSteps->setChecked(true);
    } else {
        m_wheelPrefsUI->bnInfHueSteps->setChecked(false);
    }
    m_wheelPrefsUI->numHueSteps->setValue(selectorHueSteps);

    m_wheelPrefsUI->numSaturationSteps->setValue(m_selectorUI->colorSelector->getNumRings());

    int selectorValueScaleSteps = m_selectorUI->colorSelector->getNumLightPieces();
    if (selectorValueScaleSteps == 1) {
        m_wheelPrefsUI->bnInfValueScaleSteps->setChecked(true);
    } else {
        m_wheelPrefsUI->bnInfValueScaleSteps->setChecked(false);
    }
    m_wheelPrefsUI->numValueScaleSteps->setValue(m_selectorUI->colorSelector->getNumLightPieces());

    m_preferencesUI->bnDefInfHueSteps->setIcon(infinityIcon);
    m_preferencesUI->bnDefInfValueScaleSteps->setIcon(infinityIcon);

    m_preferencesUI->labelDefHueSteps->setPixmap(hueStepsPixmap);
    m_preferencesUI->labelDefSaturationSteps->setPixmap(saturationStepsPixmap);
    m_preferencesUI->labelDefValueScaleSteps->setPixmap(valueScaleStepsPixmap);

    m_preferencesUI->defaultHueSteps->setRange(MIN_NUM_HUE_PIECES, MAX_NUM_HUE_PIECES);
    m_preferencesUI->defaultSaturationSteps->setRange(MIN_NUM_SATURATION_RINGS, MAX_NUM_SATURATION_RINGS);
    m_preferencesUI->defaultValueScaleSteps->setRange(MIN_NUM_LIGHT_PIECES, MAX_NUM_LIGHT_PIECES);

    m_preferencesUI->defaultHueSteps->setValue(m_selectorUI->colorSelector->getDefaultHueSteps());
    m_preferencesUI->defaultSaturationSteps->setValue(m_selectorUI->colorSelector->getDefaultSaturationSteps());
    m_preferencesUI->defaultValueScaleSteps->setValue(m_selectorUI->colorSelector->getDefaultValueScaleSteps());

    m_preferencesUI->showBgColor->setChecked(m_selectorUI->colorSelector->getShowBgColor());
    m_preferencesUI->showValueScaleNumbers->setChecked(m_selectorUI->colorSelector->getShowValueScaleNumbers());

    m_preferencesUI->enforceGamutMask->setChecked(m_selectorUI->colorSelector->enforceGamutMask());
    m_preferencesUI->permissiveGamutMask->setChecked(!m_selectorUI->colorSelector->enforceGamutMask());

    m_preferencesUI->spLumaR->setValue(m_selectorUI->colorSelector->lumaR());
    m_preferencesUI->spLumaG->setValue(m_selectorUI->colorSelector->lumaG());
    m_preferencesUI->spLumaB->setValue(m_selectorUI->colorSelector->lumaB());
    m_preferencesUI->spLumaGamma->setValue(m_selectorUI->colorSelector->lumaGamma());

    switch(m_selectorUI->colorSelector->getColorSpace())
    {
        case KisColor::HSV: { m_preferencesUI->bnHsv->setChecked(true); } break;
        case KisColor::HSI: { m_preferencesUI->bnHsi->setChecked(true); } break;
        case KisColor::HSL: { m_preferencesUI->bnHsl->setChecked(true); } break;
        case KisColor::HSY: { m_preferencesUI->bnHsy->setChecked(true); } break;
    }

    if (m_selectorUI->colorSelector->getColorSpace() == KisColor::HSY) {
        m_preferencesUI->lumaCoefficientBox->show();
    } else {
        m_preferencesUI->lumaCoefficientBox->hide();
    }

    connect(m_wheelPrefsUI->numValueScaleSteps  , SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_wheelPrefsUI->numHueSteps         , SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_wheelPrefsUI->numSaturationSteps  , SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_wheelPrefsUI->bnInverseSat        , SIGNAL(clicked(bool))                          , SLOT(slotPreferenceChanged()));
    connect(m_wheelPrefsUI->bnInfHueSteps       , SIGNAL(clicked(bool))                           , SLOT(slotPreferenceChanged()));
    connect(m_wheelPrefsUI->bnInfValueScaleSteps, SIGNAL(clicked(bool))                           , SLOT(slotPreferenceChanged()));
    connect(m_wheelPrefsUI->bnDefault           , SIGNAL(clicked(bool))                          , SLOT(slotResetDefaultSettings()));

    connect(m_preferencesUI->defaultHueSteps    , SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->defaultSaturationSteps, SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->defaultValueScaleSteps, SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->bnDefInfHueSteps       , SIGNAL(clicked(bool))                           , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->bnDefInfValueScaleSteps, SIGNAL(clicked(bool))                           , SLOT(slotPreferenceChanged()));

    connect(m_preferencesUI->showBgColor        , SIGNAL(toggled(bool))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->showValueScaleNumbers, SIGNAL(toggled(bool))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->enforceGamutMask   , SIGNAL(toggled(bool))                      , SLOT(slotPreferenceChanged()));

    connect(m_preferencesUI->spLumaR   , SIGNAL(valueChanged(qreal)), SLOT(slotColorSpaceSelected()));
    connect(m_preferencesUI->spLumaG   , SIGNAL(valueChanged(qreal)), SLOT(slotColorSpaceSelected()));
    connect(m_preferencesUI->spLumaB   , SIGNAL(valueChanged(qreal)), SLOT(slotColorSpaceSelected()));
    connect(m_preferencesUI->spLumaGamma   , SIGNAL(valueChanged(qreal)), SLOT(slotColorSpaceSelected()));

    connect(m_selectorUI->colorSelector         , SIGNAL(sigFgColorChanged(KisColor))     , SLOT(slotFgColorChanged(KisColor)));
    connect(m_selectorUI->colorSelector         , SIGNAL(sigBgColorChanged(KisColor))     , SLOT(slotBgColorChanged(KisColor)));

    // gamut mask connections
    connect(m_selectorUI->gamutMaskToolbar, SIGNAL(sigGamutMaskToggle(bool)), SLOT(slotGamutMaskToggle(bool)));

    connect(m_hsxButtons                        , SIGNAL(buttonClicked(int))                     , SLOT(slotColorSpaceSelected()));

    setWidget(m_selectorUI);
}

ArtisticColorSelectorDock::~ArtisticColorSelectorDock()
{
    m_selectorUI->colorSelector->saveSettings();
    delete m_hsxButtons;
}

void ArtisticColorSelectorDock::setViewManager(KisViewManager* kisview)
{
    m_resourceProvider = kisview->canvasResourceProvider();
    m_selectorUI->colorSelector->setFgColor(m_resourceProvider->resourceManager()->foregroundColor());
    m_selectorUI->colorSelector->setBgColor(m_resourceProvider->resourceManager()->backgroundColor());

    connect(m_resourceProvider, SIGNAL(sigGamutMaskChanged(KoGamutMask*)),
            this, SLOT(slotGamutMaskSet(KoGamutMask*)), Qt::UniqueConnection);

    connect(m_resourceProvider, SIGNAL(sigGamutMaskUnset()),
            this, SLOT(slotGamutMaskUnset()), Qt::UniqueConnection);

    connect(m_resourceProvider, SIGNAL(sigGamutMaskPreviewUpdate()),
            this, SLOT(slotGamutMaskPreviewUpdate()), Qt::UniqueConnection);

    m_selectorUI->gamutMaskToolbar->connectMaskSignals(m_resourceProvider);
}

void ArtisticColorSelectorDock::slotCanvasResourceChanged(int key, const QVariant& value)
{
    if(key == KoCanvasResourceProvider::ForegroundColor)
        m_selectorUI->colorSelector->setFgColor(value.value<KoColor>());

    if(key == KoCanvasResourceProvider::BackgroundColor)
        m_selectorUI->colorSelector->setBgColor(value.value<KoColor>());
}

void ArtisticColorSelectorDock::slotFgColorChanged(const KisColor& color)
{
    m_resourceProvider->resourceManager()->setForegroundColor(
        KoColor(color.toKoColor(), m_resourceProvider->resourceManager()->foregroundColor().colorSpace())
    );
}

void ArtisticColorSelectorDock::slotBgColorChanged(const KisColor& color)
{
    m_resourceProvider->resourceManager()->setBackgroundColor(
        KoColor(color.toKoColor(), m_resourceProvider->resourceManager()->backgroundColor().colorSpace())
    );
}

void ArtisticColorSelectorDock::slotColorSpaceSelected()
{
    KisColor::Type type = static_cast<KisColor::Type>(
                m_hsxButtons->id(m_hsxButtons->checkedButton()));

    m_selectorUI->colorSelector->setColorSpace(type);

    if (type == KisColor::HSY) {
        m_preferencesUI->lumaCoefficientBox->show();
    } else {
        m_preferencesUI->lumaCoefficientBox->hide();
    }

    m_selectorUI->colorSelector->setLumaCoefficients(
                m_preferencesUI->spLumaR->value(),
                m_preferencesUI->spLumaG->value(),
                m_preferencesUI->spLumaB->value(),
                m_preferencesUI->spLumaGamma->value()
                );
}

void ArtisticColorSelectorDock::slotPreferenceChanged()
{
    int hueSteps = DEFAULT_HUE_STEPS;
    if (m_wheelPrefsUI->bnInfHueSteps->isChecked()) {
        m_wheelPrefsUI->numHueSteps->setEnabled(false);
        hueSteps = 1;
    } else {
        m_wheelPrefsUI->numHueSteps->setEnabled(true);
        hueSteps = m_wheelPrefsUI->numHueSteps->value();
    }
    m_selectorUI->colorSelector->setNumPieces(hueSteps);

    m_selectorUI->colorSelector->setNumRings(m_wheelPrefsUI->numSaturationSteps->value());

    int valueScaleSteps;
    if (m_wheelPrefsUI->bnInfValueScaleSteps->isChecked()) {
        m_wheelPrefsUI->numValueScaleSteps->setEnabled(false);
        valueScaleSteps = 1;
    } else {
        valueScaleSteps = m_wheelPrefsUI->numValueScaleSteps->value();
        m_wheelPrefsUI->numValueScaleSteps->setEnabled(true);
    }
    m_selectorUI->colorSelector->setNumLightPieces(valueScaleSteps);

    int defHueSteps;
    if (m_preferencesUI->bnDefInfHueSteps->isChecked()) {
        m_preferencesUI->defaultHueSteps->setEnabled(false);
        defHueSteps = 1;
    } else {
        m_preferencesUI->defaultHueSteps->setEnabled(true);
        defHueSteps = m_preferencesUI->defaultHueSteps->value();
    }
    m_selectorUI->colorSelector->setDefaultHueSteps(defHueSteps);

    m_selectorUI->colorSelector->setDefaultSaturationSteps(m_preferencesUI->defaultSaturationSteps->value());

    int defValueScaleSteps;
    if (m_preferencesUI->bnDefInfValueScaleSteps->isChecked()) {
        m_preferencesUI->defaultValueScaleSteps->setEnabled(false);
        defValueScaleSteps = 1;
    } else {
        m_preferencesUI->defaultValueScaleSteps->setEnabled(true);
        defValueScaleSteps = m_preferencesUI->defaultValueScaleSteps->value();
    }
    m_selectorUI->colorSelector->setDefaultValueScaleSteps(defValueScaleSteps);

    m_selectorUI->colorSelector->setShowBgColor(m_preferencesUI->showBgColor->isChecked());
    m_selectorUI->colorSelector->setShowValueScaleNumbers(m_preferencesUI->showValueScaleNumbers->isChecked());
    m_selectorUI->colorSelector->setEnforceGamutMask(m_preferencesUI->enforceGamutMask->isChecked());

    m_selectorUI->colorSelector->setInverseSaturation(m_wheelPrefsUI->bnInverseSat->isChecked());
}

void ArtisticColorSelectorDock::slotResetDefaultSettings()
{
    quint32 hueSteps = m_selectorUI->colorSelector->getDefaultHueSteps();
    quint32 saturationSteps = m_selectorUI->colorSelector->getDefaultSaturationSteps();
    quint32 valueScaleSteps = m_selectorUI->colorSelector->getDefaultValueScaleSteps();

    m_selectorUI->colorSelector->setNumRings(saturationSteps);
    m_wheelPrefsUI->numSaturationSteps->blockSignals(true);
    m_wheelPrefsUI->numSaturationSteps->setValue(saturationSteps);
    m_wheelPrefsUI->numSaturationSteps->blockSignals(false);

    m_selectorUI->colorSelector->setNumPieces(hueSteps);
    m_wheelPrefsUI->numHueSteps->blockSignals(true);
    m_wheelPrefsUI->numHueSteps->setValue(hueSteps);
    m_wheelPrefsUI->numHueSteps->blockSignals(false);

    if (hueSteps == 1) {
        m_wheelPrefsUI->numHueSteps->setEnabled(false);
        m_wheelPrefsUI->bnInfHueSteps->setChecked(true);
    } else {
        m_wheelPrefsUI->numHueSteps->setEnabled(true);
        m_wheelPrefsUI->bnInfHueSteps->setChecked(false);
    }

    m_selectorUI->colorSelector->setNumLightPieces(valueScaleSteps);
    m_wheelPrefsUI->numValueScaleSteps->blockSignals(true);
    m_wheelPrefsUI->numValueScaleSteps->setValue(valueScaleSteps);
    m_wheelPrefsUI->numValueScaleSteps->blockSignals(false);

    if (valueScaleSteps == 1) {
        m_wheelPrefsUI->numValueScaleSteps->setEnabled(false);
        m_wheelPrefsUI->bnInfValueScaleSteps->setChecked(true);
    } else {
        m_wheelPrefsUI->numValueScaleSteps->setEnabled(true);
        m_wheelPrefsUI->bnInfValueScaleSteps->setChecked(false);
    }
}

void ArtisticColorSelectorDock::slotGamutMaskToggle(bool checked)
{
    bool b = (!m_selectedMask) ? false : checked;

    if (b == true) {
        m_selectorUI->colorSelector->setGamutMask(m_selectedMask);
    }

    m_selectorUI->colorSelector->setGamutMaskOn(b);
}

void ArtisticColorSelectorDock::setCanvas(KoCanvasBase *canvas)
{
    if (!canvas) {
        return;
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    if (m_canvas) {
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                SLOT(slotCanvasResourceChanged(int,QVariant)), Qt::UniqueConnection);

        connect(m_canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()),
                SLOT(slotSelectorSettingsChanged()), Qt::UniqueConnection);

        m_selectorUI->colorSelector->setColorConverter(m_canvas->displayColorConverter());
        setEnabled(true);
    }
}

void ArtisticColorSelectorDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = nullptr;
    m_selectorUI->colorSelector->setColorConverter(KisDisplayColorConverter::dumbConverterInstance());
}

void ArtisticColorSelectorDock::slotGamutMaskSet(KoGamutMask *mask)
{
    if (!mask) {
        return;
    }

    m_selectedMask = mask;

    if (m_selectedMask) {
        m_selectorUI->colorSelector->setGamutMask(m_selectedMask);
        slotGamutMaskToggle(true);
    } else {
        slotGamutMaskToggle(false);
    }
}

void ArtisticColorSelectorDock::slotGamutMaskUnset()
{
    if (!m_selectedMask) {
        return;
    }

    m_selectedMask = nullptr;

    slotGamutMaskToggle(false);
    m_selectorUI->colorSelector->setGamutMask(m_selectedMask);
}

void ArtisticColorSelectorDock::slotGamutMaskPreviewUpdate()
{
    m_selectorUI->colorSelector->setDirty();
}

void ArtisticColorSelectorDock::slotSelectorSettingsChanged()
{
    m_selectorUI->colorSelector->setDirty();
}
