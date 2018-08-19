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

#include <klocalizedstring.h>
#include <KoCanvasResourceManager.h>
#include <KoCanvasBase.h>
#include <KoColor.h>

#include <QWidget>
#include <QMenu>
#include <QButtonGroup>

#include "artisticcolorselector_dock.h"
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>

#include "ui_wdgArtisticColorSelector.h"
#include "ui_wdgColorPreferencesPopup.h"

enum { ACTION_RESET_EVERYTHING, ACTION_RESET_SELECTED_RING, ACTION_RESET_EVERY_RING, ACTION_RESET_LIGHT };

struct ArtisticColorSelectorUI: public QWidget, public Ui_wdgArtisticColorSelector
{
    ArtisticColorSelectorUI() {
        setupUi(this);
    }
};

struct ColorPreferencesPopupUI: public QWidget, public Ui_wdgColorPreferencesPopup
{
    ColorPreferencesPopupUI() {
        setupUi(this);
    }
};

ArtisticColorSelectorDock::ArtisticColorSelectorDock():
    QDockWidget(i18n("Artistic Color Selector")),
    m_resourceProvider(0)
{
    m_hsxButtons    = new QButtonGroup();
    m_resetMenu     = new QMenu();
    m_preferencesUI = new ColorPreferencesPopupUI();
    m_selectorUI    = new ArtisticColorSelectorUI();

    m_resetMenu->addAction(i18n("Reset All Rings"))->setData(ACTION_RESET_EVERY_RING);
    m_resetMenu->addAction(i18n("Reset Selected Ring"))->setData(ACTION_RESET_SELECTED_RING);
    m_resetMenu->addAction(i18n("Reset Light"))->setData(ACTION_RESET_LIGHT);
    m_resetMenu->addAction(i18n("Reset Everything"))->setData(ACTION_RESET_EVERYTHING);

    m_selectorUI->colorSelector->loadSettings();
    m_selectorUI->bnColorPrefs->setPopupWidget(m_preferencesUI);
    m_selectorUI->bnReset->setMenu(m_resetMenu);
    m_selectorUI->bnAbsLight->setChecked(!m_selectorUI->colorSelector->islightRelative());

    m_hsxButtons->addButton(m_preferencesUI->bnHsy, KisColor::HSY);
    m_hsxButtons->addButton(m_preferencesUI->bnHsi, KisColor::HSI);
    m_hsxButtons->addButton(m_preferencesUI->bnHsl, KisColor::HSL);
    m_hsxButtons->addButton(m_preferencesUI->bnHsv, KisColor::HSV);

    m_preferencesUI->numPiecesSlider->setRange(1, 48);
    m_preferencesUI->numRingsSlider->setRange(1, 20);
    m_preferencesUI->numLightPiecesSlider->setRange(1, 30);
    m_preferencesUI->numPiecesSlider->setValue(m_selectorUI->colorSelector->getNumPieces());
    m_preferencesUI->numRingsSlider->setValue(m_selectorUI->colorSelector->getNumRings());
    m_preferencesUI->numLightPiecesSlider->setValue(m_selectorUI->colorSelector->getNumLightPieces());
    m_preferencesUI->bnInverseSat->setChecked(m_selectorUI->colorSelector->isSaturationInverted());
    
    switch(m_selectorUI->colorSelector->getColorSpace())
    {
        case KisColor::HSV: { m_preferencesUI->bnHsv->setChecked(true); } break;
        case KisColor::HSI: { m_preferencesUI->bnHsi->setChecked(true); } break;
        case KisColor::HSL: { m_preferencesUI->bnHsl->setChecked(true); } break;
        case KisColor::HSY: { m_preferencesUI->bnHsy->setChecked(true); } break;
    }
    
    connect(m_preferencesUI->numLightPiecesSlider, SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->numPiecesSlider     , SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->numRingsSlider      , SIGNAL(valueChanged(int))                      , SLOT(slotPreferenceChanged()));
    connect(m_preferencesUI->bnInverseSat        , SIGNAL(clicked(bool))                          , SLOT(slotPreferenceChanged()));
    connect(m_selectorUI->colorSelector          , SIGNAL(sigFgColorChanged(const KisColor&))     , SLOT(slotFgColorChanged(const KisColor&)));
    connect(m_selectorUI->colorSelector          , SIGNAL(sigBgColorChanged(const KisColor&))     , SLOT(slotBgColorChanged(const KisColor&)));
    connect(m_hsxButtons                         , SIGNAL(buttonClicked(int))                     , SLOT(slotColorSpaceSelected(int)));
    connect(m_preferencesUI->bnDefault           , SIGNAL(clicked(bool))                          , SLOT(slotResetDefaultSettings()));
    connect(m_selectorUI->bnAbsLight             , SIGNAL(toggled(bool))                          , SLOT(slotLightModeChanged(bool)));
    connect(m_resetMenu                          , SIGNAL(triggered(QAction*))                    , SLOT(slotMenuActionTriggered(QAction*)));
    
    setWidget(m_selectorUI);
}

ArtisticColorSelectorDock::~ArtisticColorSelectorDock()
{
    m_selectorUI->colorSelector->saveSettings();
    delete m_hsxButtons;
    delete m_resetMenu;
}

void ArtisticColorSelectorDock::setViewManager(KisViewManager* kisview)
{
    m_resourceProvider = kisview->resourceProvider();
    m_selectorUI->colorSelector->setFgColor(m_resourceProvider->resourceManager()->foregroundColor().toQColor());
    m_selectorUI->colorSelector->setBgColor(m_resourceProvider->resourceManager()->backgroundColor().toQColor());
    connect(m_resourceProvider->resourceManager(), SIGNAL(canvasResourceChanged(int, const QVariant&)),
            SLOT(slotCanvasResourceChanged(int, const QVariant&)));
}

void ArtisticColorSelectorDock::slotCanvasResourceChanged(int key, const QVariant& value)
{
    if(key == KoCanvasResourceManager::ForegroundColor)
        m_selectorUI->colorSelector->setFgColor(value.value<KoColor>().toQColor());

    if(key == KoCanvasResourceManager::BackgroundColor)
        m_selectorUI->colorSelector->setBgColor(value.value<KoColor>().toQColor());
}

void ArtisticColorSelectorDock::slotFgColorChanged(const KisColor& color)
{
    m_resourceProvider->resourceManager()->setForegroundColor(
        KoColor(color.getQColor(), m_resourceProvider->resourceManager()->foregroundColor().colorSpace())
    );
}

void ArtisticColorSelectorDock::slotBgColorChanged(const KisColor& color)
{
    m_resourceProvider->resourceManager()->setBackgroundColor(
        KoColor(color.getQColor(), m_resourceProvider->resourceManager()->backgroundColor().colorSpace())
    );
}

void ArtisticColorSelectorDock::slotColorSpaceSelected(int type)
{
    m_selectorUI->colorSelector->setColorSpace(static_cast<KisColor::Type>(type));
}

void ArtisticColorSelectorDock::slotPreferenceChanged()
{
    m_selectorUI->colorSelector->setNumPieces(m_preferencesUI->numPiecesSlider->value());
    m_selectorUI->colorSelector->setNumRings(m_preferencesUI->numRingsSlider->value());
    m_selectorUI->colorSelector->setNumLightPieces(m_preferencesUI->numLightPiecesSlider->value());
    m_selectorUI->colorSelector->setInverseSaturation(m_preferencesUI->bnInverseSat->isChecked());
}

void ArtisticColorSelectorDock::slotMenuActionTriggered(QAction* action)
{
    switch(action->data().toInt())
    {
    case ACTION_RESET_SELECTED_RING:
        m_selectorUI->colorSelector->resetSelectedRing();
        break;

    case ACTION_RESET_EVERY_RING:
        m_selectorUI->colorSelector->resetRings();
        break;

    case ACTION_RESET_LIGHT:
        m_selectorUI->colorSelector->resetLight();
        break;

    case ACTION_RESET_EVERYTHING:
        m_selectorUI->colorSelector->resetLight();
        m_selectorUI->colorSelector->resetRings();
        break;
    }
}

void ArtisticColorSelectorDock::slotResetDefaultSettings()
{
    m_selectorUI->colorSelector->setNumRings(7);
    m_preferencesUI->numRingsSlider->blockSignals(true);
    m_preferencesUI->numRingsSlider->setValue(7);
    m_preferencesUI->numRingsSlider->blockSignals(false);

    m_selectorUI->colorSelector->setNumPieces(12);
    m_preferencesUI->numPiecesSlider->blockSignals(true);
    m_preferencesUI->numPiecesSlider->setValue(12);
    m_preferencesUI->numPiecesSlider->blockSignals(false);

    m_selectorUI->colorSelector->setNumLightPieces(9);
    m_preferencesUI->numLightPiecesSlider->blockSignals(true);
    m_preferencesUI->numLightPiecesSlider->setValue(9);
    m_preferencesUI->numLightPiecesSlider->blockSignals(false);
}

void ArtisticColorSelectorDock::slotLightModeChanged(bool setToAbsolute)
{
    m_selectorUI->colorSelector->setLight(m_selectorUI->colorSelector->getLight(), !setToAbsolute);
}



void ArtisticColorSelectorDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
}

void ArtisticColorSelectorDock::unsetCanvas()
{
    setEnabled(false);
}
