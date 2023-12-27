/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorSelectorDock.h"

#include "WGActionManager.h"
#include "WGColorSelectorSettings.h"
#include "WGColorPatches.h"
#include "WGColorPreviewToolTip.h"
#include "WGCommonColorSet.h"
#include "WGConfigSelectorTypes.h"
#include "WGQuickSettingsWidget.h"
#include "WGShadeSelector.h"
#include "KisVisualColorSelector.h"
#include "KisColorSourceToggle.h"

#include <klocalizedstring.h>

#include <kis_icon.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_config_notifier.h>
#include <kis_display_color_converter.h>
#include <kis_signal_compressor.h>
#include <KisUniqueColorSet.h>
#include <KoCanvasResourceProvider.h>

#include <QLabel>
#include <QBoxLayout>
#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>

#include <QDebug>
#include <kis_assert.h>

WGColorSelectorDock::WGColorSelectorDock()
	: QDockWidget()
    , m_displayConfig(new WGSelectorDisplayConfig)
    , m_colorChangeCompressor(new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this))
    , m_actionManager(new WGActionManager(this))
    , m_colorTooltip(new WGColorPreviewToolTip(this))
    , m_colorModelBG(new KisVisualColorModel)
{
    setWindowTitle(i18n("Wide Gamut Color Selector"));

    QWidget *mainWidget = new QWidget();
    m_mainWidgetLayout = new QVBoxLayout(mainWidget);
    m_verticalElementsLayout = new QHBoxLayout();
    m_selectorAreaLayout = new QBoxLayout(QBoxLayout::TopToBottom);

    m_selector = new KisVisualColorSelector(mainWidget);
    m_selector->setMinimumSliderWidth(12);
    connect(m_selector, SIGNAL(sigNewColor(KoColor)), SLOT(slotColorSelected(KoColor)));
    connect(m_selector, SIGNAL(sigInteraction(bool)), SLOT(slotColorInteraction(bool)));
    connect(m_colorChangeCompressor, SIGNAL(timeout()), SLOT(slotSetNewColors()));
    m_colorModelFG = m_selector->selectorModel();

    // Header
    QWidget *headerWidget = new QWidget(mainWidget);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);

    m_toggle = new KisColorSourceToggle(mainWidget);
    connect(m_toggle, SIGNAL(toggled(bool)), SLOT(slotColorSourceToggled(bool)));
    headerLayout->addWidget(m_toggle);
    headerLayout->addStretch();
    headerLayout->setMargin(0);

    m_configButton = new QToolButton(this);
    m_configButton->setIcon(KisIconUtils::loadIcon("view-choose"));
    m_configButton->setAutoRaise(true);
    m_configButton->setPopupMode(QToolButton::InstantPopup);
    headerLayout->addWidget(m_configButton);

    m_mainWidgetLayout->addWidget(headerWidget);
    m_mainWidgetLayout->addLayout(m_verticalElementsLayout);
    m_verticalElementsLayout->addLayout(m_selectorAreaLayout);

    m_selectorAreaLayout->addWidget(m_selector);

    KisVisualColorModelSP model = m_selector->selectorModel();
    m_shadeSelector = new WGShadeSelector(m_displayConfig, model, this);
    m_selectorAreaLayout->addWidget(m_shadeSelector);
    connect(m_shadeSelector, SIGNAL(sigColorInteraction(bool)), SLOT(slotColorInteraction(bool)));

    // eventually it should made be a global history, not specific to any plugin
    m_colorHistory = new KisUniqueColorSet(this);

    m_history = new WGColorPatches(m_displayConfig, m_colorHistory, mainWidget);
    m_history->setPreset(WGColorPatches::History);
    connect(m_history, SIGNAL(sigColorChanged(KoColor)), SLOT(slotColorSelected(KoColor)));
    connect(m_history, SIGNAL(sigColorInteraction(bool)), SLOT(slotColorInteraction(bool)));

    // Common Colors (Colors from Image)
    m_commonColorSet = new WGCommonColorSet(this);

    m_commonColors = new WGColorPatches(m_displayConfig, m_commonColorSet, mainWidget);
    m_commonColors->setPreset(WGColorPatches::CommonColors);
    connect(m_commonColors, SIGNAL(sigColorChanged(KoColor)), SLOT(slotColorSelected(KoColor)));
    connect(m_commonColors, SIGNAL(sigColorInteraction(bool)), SLOT(slotColorInteraction(bool)));

    connect(WGConfig::notifier(), SIGNAL(configChanged()), SLOT(slotConfigurationChanged()));

    setWidget(mainWidget);
    slotConfigurationChanged();
    setEnabled(false);
}

const KisVisualColorModel &WGColorSelectorDock::colorModel() const
{
    // currently always return foreground model;
    // do lazy conversion to current HSX model if required
    if (selectingBackground() && m_colorModelBG->isHSXModel()) {
        m_colorModelFG->setRGBColorModel(m_colorModelBG->colorModel());
    }
    return *(m_colorModelFG);
}

KisDisplayColorConverter *WGColorSelectorDock::displayColorConverter(bool rawPointer) const
{
    KisDisplayColorConverter *converter = nullptr;
    if (m_canvas) {
        converter = m_canvas->displayColorConverter();
    }
    return (rawPointer || converter) ?  converter : KisDisplayColorConverter::dumbConverterInstance();
}

bool WGColorSelectorDock::selectingBackground() const
{
    return m_toggle->isChecked();
}

void WGColorSelectorDock::setChannelValues(const QVector4D &values)
{
    // currently always set foreground color
    if (!m_canvas) {
        return;
    }

    // This could be nicer...if setting active model, this triggers slotColorSelected()
    // and leaves timer running with NOP, otherwise the resource update is detected as
    // external event and updates UI.
    m_colorModelFG->slotSetChannelValues(values);

    m_canvas->resourceManager()->setForegroundColor(m_colorModelFG->currentColor());
    m_pendingFgUpdate = false;
}

void WGColorSelectorDock::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_colorTooltip->hide();
}

void WGColorSelectorDock::setCanvas(KoCanvasBase *canvas)
{
    if (m_canvas.data() == canvas)
    {
        // not sure why setCanvas gets called 3 times for new canvas, just skip
        return;
    }
    if (m_canvas) {
        disconnectFromCanvas();
    }
    m_actionManager->setCanvas(qobject_cast<KisCanvas2*>(canvas), m_canvas);
    m_canvas = qobject_cast<KisCanvas2*>(canvas);
    // TODO: unset display converters if canvas is null
    if (m_canvas) {
        KoColorDisplayRendererInterface *dri = m_canvas->displayColorConverter()->displayRendererInterface();
        KisCanvasResourceProvider *resourceProvider = m_canvas->imageView()->resourceProvider();
        m_selector->setDisplayRenderer(dri);
        m_displayConfig->setDisplayConverter(m_canvas->displayColorConverter());
        m_commonColorSet->setImage(m_canvas->image());
        //m_toggle->setBackgroundColor(dri->toQColor(color));
        connect(dri, SIGNAL(displayConfigurationChanged()), this, SLOT(slotDisplayConfigurationChanged()));
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                this, SLOT(slotCanvasResourceChanged(int,QVariant)));
        connect(resourceProvider, SIGNAL(sigFGColorUsed(KoColor)),
                this, SLOT(slotFGColorUsed(KoColor)), Qt::UniqueConnection);

        // Gamut Mask
        connect(resourceProvider, SIGNAL(sigGamutMaskChanged(KoGamutMaskSP)),
                m_selector, SLOT(slotGamutMaskChanged(KoGamutMaskSP)), Qt::UniqueConnection);

        connect(resourceProvider, SIGNAL(sigGamutMaskPreviewUpdate()),
                m_selector, SLOT(slotGamutMaskPreviewUpdate()), Qt::UniqueConnection);

        connect(resourceProvider, SIGNAL(sigGamutMaskUnset()),
                m_selector, SLOT(slotGamutMaskUnset()), Qt::UniqueConnection);

        connect(resourceProvider, SIGNAL(sigGamutMaskDeactivated()),
                m_selector, SLOT(slotGamutMaskUnset()), Qt::UniqueConnection);

        // quirk note: displayConfigurationChanged signal gets sent *before* the new canvas gets
        // passed, through the display converter of the now inactive canvas! So we need to repeat...
        slotDisplayConfigurationChanged();
    }
    setEnabled(canvas != 0);
}

void WGColorSelectorDock::unsetCanvas()
{
    setEnabled(false);
    m_actionManager->setCanvas(0, 0);
    m_displayConfig->setDisplayConverter(0);
    m_selector->setDisplayRenderer(0);
    m_commonColorSet->setImage(KisImageSP());
    m_canvas = 0;
}

void WGColorSelectorDock::setViewManager(KisViewManager *viewManager)
{
    m_actionManager->registerActions(viewManager);
}

void WGColorSelectorDock::disconnectFromCanvas()
{
    m_canvas->disconnectCanvasObserver(this);
    m_canvas->displayColorConverter()->displayRendererInterface()->disconnect(this);
    m_canvas->imageView()->resourceProvider()->disconnect(m_selector);
    m_canvas = 0;
}

void WGColorSelectorDock::updateLayout()
{
    WGConfig::Accessor cfg;

    bool historyEnabled = cfg.get(WGConfig::colorHistoryEnabled);
    Qt::Orientation historyOrientation = cfg.get(WGConfig::colorHistory.orientation);

    m_verticalElementsLayout->removeWidget(m_history);
    m_verticalElementsLayout->removeWidget(m_commonColors);
    m_mainWidgetLayout->removeWidget(m_history);
    m_mainWidgetLayout->removeWidget(m_commonColors);

    if (historyEnabled) {
        if (historyOrientation == Qt::Vertical) {
            m_verticalElementsLayout->addWidget(m_history);
        }
        else {
            m_mainWidgetLayout->addWidget(m_history);
        }
        m_history->show();
    }
    else {
        m_history->hide();
    }

    bool commonColorsEnabled = cfg.get(WGConfig::commonColorsEnabled);
    Qt::Orientation commonColorsOrientation = cfg.get(WGConfig::commonColors.orientation);

    if (commonColorsEnabled) {
        if (commonColorsOrientation == Qt::Vertical) {
            m_verticalElementsLayout->addWidget(m_commonColors);
        }
        else {
            m_mainWidgetLayout->addWidget(m_commonColors);
        }
        m_commonColors->show();
    }
    else {
        m_commonColors->hide();
    }

}

void WGColorSelectorDock::slotConfigurationChanged()
{
    WGConfig::Accessor cfg;
    m_selector->setRenderMode(cfg.get(WGConfig::selectorRenderMode));
    m_selector->selectorModel()->setRGBColorModel(cfg.get(WGConfig::rgbColorModel));
    KisColorSelectorConfiguration selectorCfg = cfg.colorSelectorConfiguration();
    m_selector->setConfiguration(&selectorCfg);
    m_CSSource = cfg.get(WGConfig::colorSpaceSource);
    if (m_CSSource == FixedColorSpace) {
        m_customCS = cfg.customSelectionColorSpace();
    }
    bool proofColors = cfg.get(WGConfig::proofToPaintingColors);
    m_selector->setProofColors(proofColors);
    m_displayConfig->setPreviewInPaintingCS(proofColors);
    m_shadeSelector->updateSettings();
    m_history->updateSettings();
    m_commonColors->updateSettings();
    m_commonColorSet->setAutoUpdate(cfg.get(WGConfig::commonColorsAutoUpdate));
    // Quick settings menu
    if (cfg.get(WGConfig::quickSettingsEnabled)) {
        if (!m_configButton->menu()) {
            m_configButton->disconnect(this);
            QMenu *configureMenu = new QMenu(this);
            m_quickSettings = new WGQuickSettingsWidget(this, m_selector);
            // prevents hover indicator being stuck on other menu entries
            m_quickSettings->setMouseTracking(true);
            m_quickSettingAction = new QWidgetAction(this);
            m_quickSettingAction->setDefaultWidget(m_quickSettings);

            configureMenu->addAction(m_quickSettingAction);
            QAction *cfgAction = configureMenu->addAction(i18nc("Wide Gamut Color Selector", "Configure..."));
            connect(cfgAction, SIGNAL(triggered(bool)), SLOT(slotOpenSettings()));
            m_configButton->setMenu(configureMenu);
        } else {
            // WORKAROUND: force geometry update by re-adding action, all other attempts failed...
            QMenu *menu = m_configButton->menu();
            menu->removeAction(m_quickSettingAction);
            menu->insertAction(menu->actions().constFirst(), m_quickSettingAction);
        }
        m_quickSettings->loadConfiguration();
    } else {
        if (m_configButton->menu()) {
            m_configButton->menu()->deleteLater();
            m_configButton->setMenu(0);
            delete m_quickSettingAction;
            m_quickSettingAction = 0;
            m_quickSettings = 0;
        }
        connect(m_configButton, SIGNAL(clicked(bool)), SLOT(slotOpenSettings()), Qt::UniqueConnection);
    }

    updateLayout();

    if (m_canvas) {
        slotDisplayConfigurationChanged();
    }
}

void WGColorSelectorDock::slotDisplayConfigurationChanged()
{
    if (!m_canvas) {
        return;
    }

    const KoColorSpace *selectionCS = nullptr;

    // Note: displayConfigurationChanged is also emitted on image color space changes,
    // so there's no need to handle extra signals, although it's a bit convoluted
    if (m_CSSource == ImageColorSpace) {
        selectionCS  = m_canvas->image()->colorSpace();
    } else if (m_CSSource == FixedColorSpace) {
        selectionCS = m_customCS;
    } else {
        selectionCS = m_canvas->displayColorConverter()->nodeColorSpace();
    }

    // TODO: use m_viewManager->canvasResourceProvider()->fgColor();
    KoColor fgColor = m_canvas->resourceManager()->foregroundColor();
    KoColor bgColor = m_canvas->resourceManager()->backgroundColor();

    bool proof = m_displayConfig->previewInPaintingCS();
    m_toggle->setForegroundColor(displayColorConverter()->toQColor(fgColor, proof));
    m_toggle->setBackgroundColor(displayColorConverter()->toQColor(bgColor, proof));
    KisVisualColorModelSP activeModel = m_selector->selectorModel();

    if (selectionCS && selectionCS != activeModel->colorSpace()) {
        activeModel->slotSetColorSpace(selectionCS);
        activeModel->slotSetColor(activeModel == m_colorModelFG ? fgColor : bgColor);
    }
}

void WGColorSelectorDock::slotColorSelected(const KoColor &color)
{
    QColor displayCol = displayColorConverter()->toQColor(color, m_displayConfig->previewInPaintingCS());
    m_colorTooltip->setCurrentColor(displayCol);
    if (selectingBackground()) {
        m_toggle->setBackgroundColor(displayCol);
        m_pendingBgUpdate = true;
        m_colorChangeCompressor->start();
    }
    else {
        m_toggle->setForegroundColor(displayCol);
        m_pendingFgUpdate = true;
        m_colorChangeCompressor->start();
    }
    if (sender() != m_selector) {
        m_selector->slotSetColor(color);
    }
}

void WGColorSelectorDock::slotColorSourceToggled(bool selectingBg)
{
    if (selectingBg) {
        if (m_colorModelFG->isHSXModel()) {
            m_colorModelBG->setRGBColorModel(m_colorModelFG->colorModel());
        }
        m_selector->setSelectorModel(m_colorModelBG);
        m_shadeSelector->setModel(m_colorModelBG);
    }
    else {
        if (m_colorModelBG->isHSXModel()) {
            m_colorModelFG->setRGBColorModel(m_colorModelBG->colorModel());
        }
        m_selector->setSelectorModel(m_colorModelFG);
        m_shadeSelector->setModel(m_colorModelFG);
    }
    // currently, only the color space of the active model is updated, so it may be outdated
    if (m_canvas) {
        slotDisplayConfigurationChanged();
    }
}

void WGColorSelectorDock::slotColorInteraction(bool active)
{
    if (active) {
        QColor baseCol = m_displayConfig->displayConverter()->toQColor(m_selector->getCurrentColor());
        m_colorTooltip->setCurrentColor(baseCol);
        m_colorTooltip->setPreviousColor(baseCol);
        if (sender() == m_shadeSelector) {
            m_colorTooltip->show(m_shadeSelector);
        } else {
            m_colorTooltip->show(this);
        }
    }
}

void WGColorSelectorDock::slotFGColorUsed(const KoColor &color)
{
    QColor lastCol = m_displayConfig->displayConverter()->toQColor(color);
    m_colorTooltip->setLastUsedColor(lastCol);
    m_actionManager->setLastUsedColor(color);
    m_colorHistory->addColor(color);
}

void WGColorSelectorDock::slotSetNewColors()
{
    //KIS_SAFE_ASSERT_RECOVER_RETURN(m_pendingFgUpdate || m_pendingBgUpdate);
    if (m_pendingFgUpdate) {
        m_canvas->resourceManager()->setForegroundColor(m_colorModelFG->currentColor());
        m_pendingFgUpdate = false;
    }
    if (m_pendingBgUpdate) {
        m_canvas->resourceManager()->setBackgroundColor(m_colorModelBG->currentColor());
        m_pendingBgUpdate = false;
    }
}

void WGColorSelectorDock::slotCanvasResourceChanged(int key, const QVariant &value)
{
    switch (key) {
    case KoCanvasResource::ForegroundColor:
        if (!m_pendingFgUpdate) {
            KoColor color = value.value<KoColor>();
            m_toggle->setForegroundColor(displayColorConverter()->toQColor(color));
            m_colorModelFG->slotSetColor(color);
        }
        break;
    case KoCanvasResource::BackgroundColor:
        if (!m_pendingBgUpdate) {
            KoColor color = value.value<KoColor>();
            m_toggle->setBackgroundColor(displayColorConverter()->toQColor(color));
            m_colorModelBG->slotSetColor(color);
        }
    default:
        break;
    }
}

void WGColorSelectorDock::slotOpenSettings()
{
    if (!m_canvas) return;

    WGColorSelectorSettingsDialog settings;
    if (settings.exec() == QDialog::Accepted) {
        //WGConfig::notifier()->notifyConfigChanged();
    }
}

namespace WGConfig {
const NumericSetting<WGColorSelectorDock::ColorSpaceSource> colorSpaceSource
{
    "colorSpaceSource", WGColorSelectorDock::LayerColorSpace,
    WGColorSelectorDock::LayerColorSpace, WGColorSelectorDock::FixedColorSpace, true
};
}
