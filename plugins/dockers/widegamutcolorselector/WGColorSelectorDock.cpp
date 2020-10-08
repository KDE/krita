/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorPreviewPopup.h"
#include "WGColorSelectorDock.h"
#include "WGColorSelectorSettings.h"
#include "WGColorPatches.h"
#include "WGConfig.h"
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
    , m_colorChangeCompressor(new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this))
    , m_previewPopup(new WGColorPreviewPopup(this))
{
    setWindowTitle(i18n("Wide Gamut Color Selector"));

    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(new QVBoxLayout());

    m_selector = new KisVisualColorSelector(mainWidget);
    connect(m_selector, SIGNAL(sigNewColor(KoColor)), SLOT(slotColorSelected(KoColor)));
    connect(m_selector, SIGNAL(sigInteraction(bool)), SLOT(slotColorInteraction(bool)));
    connect(m_colorChangeCompressor, SIGNAL(timeout()), SLOT(slotSetNewColors()));

    // Header
    QWidget *headerWidget = new QWidget(mainWidget);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);

    m_toggle = new KisColorSourceToggle(mainWidget);
    connect(m_toggle, SIGNAL(toggled(bool)), SLOT(slotColorSourceToggled(bool)));
    headerLayout->addWidget(m_toggle);
    headerLayout->addStretch();
    headerLayout->setMargin(0);

    m_configButton = new QToolButton(this);
    m_configButton->setIcon(KisIconUtils::loadIcon("configure"));
    m_configButton->setPopupMode(QToolButton::InstantPopup);
    headerLayout->addWidget(m_configButton);

    mainWidget->layout()->addWidget(headerWidget);
    mainWidget->layout()->addWidget(m_selector);


    KisVisualColorModel *model = m_selector->selectorModel();
    m_shadeSelector = new WGShadeSelector(model, this);
    mainWidget->layout()->addWidget(m_shadeSelector);
    connect(model, SIGNAL(sigChannelValuesChanged(QVector4D)),
            m_shadeSelector, SLOT(slotChannelValuesChanged(QVector4D)));
    connect(m_shadeSelector, SIGNAL(sigChannelValuesChanged(QVector4D)),
            model, SLOT(slotSetChannelValues(QVector4D)));
    connect(m_shadeSelector, SIGNAL(sigColorInteraction(bool)), SLOT(slotColorInteraction(bool)));

    m_history = new WGColorPatches(mainWidget);
    mainWidget->layout()->addWidget(m_history);
    connect(m_history, SIGNAL(sigColorChanged(KoColor)), SLOT(slotColorSelected(KoColor)));
    connect(m_history, SIGNAL(sigInteraction(bool)), SLOT(slotColorInteraction(bool)));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigurationChanged()));

    setWidget(mainWidget);
    slotConfigurationChanged();
    setEnabled(false);
}

void WGColorSelectorDock::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_previewPopup->hide();
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
    m_canvas = qobject_cast<KisCanvas2*>(canvas);
    if (m_canvas) {
        KoColorDisplayRendererInterface *dri = m_canvas->displayColorConverter()->displayRendererInterface();
        m_selector->setDisplayRenderer(dri);
        m_history->setDisplayRenderer(dri);
        //m_toggle->setBackgroundColor(dri->toQColor(color));
        connect(dri, SIGNAL(displayConfigurationChanged()), this, SLOT(slotDisplayConfigurationChanged()));
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                this, SLOT(slotCanvasResourceChanged(int,QVariant)));
        connect(m_canvas->imageView()->resourceProvider(), SIGNAL(sigFGColorUsed(KoColor)),
                this, SLOT(slotFGColorUsed(KoColor)), Qt::UniqueConnection);
    }
    setEnabled(canvas != 0);
}

void WGColorSelectorDock::unsetCanvas()
{
    setEnabled(false);
    m_selector->setDisplayRenderer(0);
    m_history->setDisplayRenderer(0);
    m_canvas = 0;
}

void WGColorSelectorDock::disconnectFromCanvas()
{
    m_canvas->disconnectCanvasObserver(this);
    m_canvas->displayColorConverter()->displayRendererInterface()->disconnect(this);
    m_canvas = 0;
}

void WGColorSelectorDock::slotConfigurationChanged()
{
    WGConfig cfg;
    int renderMode = qBound(int(KisVisualColorSelector::StaticBackground), cfg.readEntry("renderMode", 1),
                            int(KisVisualColorSelector::CompositeBackground));
    m_selector->setRenderMode(static_cast<KisVisualColorSelector::RenderMode>(renderMode));
    m_selector->selectorModel()->setRGBColorModel(static_cast<KisVisualColorModel::ColorModel>(cfg.readEntry("rgbColorModel", 0)));
    KisColorSelectorConfiguration selectorCfg(cfg.readEntry<QString>("colorSelectorConfiguration", "3|0|6|0")); // triangle selector
    m_selector->setConfiguration(&selectorCfg);
    m_shadeSelector->updateSettings();
    // Quick settings menu
    if (cfg.quickSettingsEnabled()) {
        if (!m_configButton->menu()) {
            m_configButton->disconnect(this);
            QMenu *configureMenu = new QMenu(this);
            m_quickSettings = new WGQuickSettingsWidget(this, m_selector);
            // prevents hover indicator being stuck on other menu entries
            m_quickSettings->setMouseTracking(true);
            m_quickSettingAction = new QWidgetAction(this);
            m_quickSettingAction->setDefaultWidget(m_quickSettings);

            configureMenu->addAction(m_quickSettingAction);
            QAction *cfgAction = configureMenu->addAction("Configure...");
            connect(cfgAction, SIGNAL(triggered(bool)), SLOT(slotOpenSettings()));
            m_configButton->setMenu(configureMenu);
        } else {
            // WORKAROUND: force geometry update by re-adding action, all other attempts failed...
            QMenu *menu = m_configButton->menu();
            menu->removeAction(m_quickSettingAction);
            menu->insertAction(menu->actions().first(), m_quickSettingAction);
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
}

void WGColorSelectorDock::slotDisplayConfigurationChanged()
{
    bool selectingBg = m_toggle->isChecked();
    m_selector->slotSetColorSpace(m_canvas->displayColorConverter()->paintingColorSpace());
    // TODO: use m_viewManager->canvasResourceProvider()->fgColor();
    KoColor fgColor = m_canvas->resourceManager()->foregroundColor();
    KoColor bgColor = m_canvas->resourceManager()->backgroundColor();
    // TODO: use painting color space?
    m_toggle->setForegroundColor(m_canvas->displayColorConverter()->toQColor(fgColor));
    m_toggle->setBackgroundColor(m_canvas->displayColorConverter()->toQColor(bgColor));
    m_selector->slotSetColor(selectingBg ? bgColor : fgColor);
}

void WGColorSelectorDock::slotColorSelected(const KoColor &color)
{
    bool selectingBg = m_toggle->isChecked();
    QColor displayCol = m_canvas->displayColorConverter()->toQColor(color);
    m_previewPopup->setCurrentColor(displayCol);
    if (selectingBg) {
        m_toggle->setBackgroundColor(displayCol);
        m_pendingBgUpdate = true;
        m_bgColor = color;
        m_colorChangeCompressor->start();
    }
    else {
        m_toggle->setForegroundColor(displayCol);
        m_pendingFgUpdate = true;
        m_fgColor = color;
        m_colorChangeCompressor->start();
    }
    if (sender() != m_selector) {
        m_selector->slotSetColor(color);
    }
}

void WGColorSelectorDock::slotColorSourceToggled(bool selectingBg)
{
    if (selectingBg) {
        m_selector->slotSetColor(m_canvas->resourceManager()->backgroundColor());
    }
    else {
        m_selector->slotSetColor(m_canvas->resourceManager()->foregroundColor());
    }
}

void WGColorSelectorDock::slotColorInteraction(bool active)
{
    if (active) {
        QColor baseCol = m_selector->selectorModel()->displayRenderer()->toQColor(m_selector->getCurrentColor());
        m_previewPopup->setCurrentColor(baseCol);
        m_previewPopup->setPreviousColor(baseCol);
        if (sender() == m_shadeSelector) {
            m_previewPopup->show(m_shadeSelector);
        } else {
            m_previewPopup->show(this);
        }
    }
}

void WGColorSelectorDock::slotFGColorUsed(const KoColor &color)
{
    QColor lastCol = m_selector->selectorModel()->displayRenderer()->toQColor(color);
    m_previewPopup->setLastUsedColor(lastCol);
    m_history->addUniqueColor(color);
}

void WGColorSelectorDock::slotSetNewColors()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_pendingFgUpdate || m_pendingBgUpdate);
    if (m_pendingFgUpdate) {
        m_canvas->resourceManager()->setForegroundColor(m_fgColor);
        m_pendingFgUpdate = false;
    }
    if (m_pendingBgUpdate) {
        m_canvas->resourceManager()->setBackgroundColor(m_bgColor);
        m_pendingBgUpdate = false;
    }
}

void WGColorSelectorDock::slotCanvasResourceChanged(int key, const QVariant &value)
{
    bool selectingBg = m_toggle->isChecked();
    switch (key) {
    case KoCanvasResource::ForegroundColor:
        if (!m_pendingFgUpdate) {
            KoColor color = value.value<KoColor>();
            m_toggle->setForegroundColor(m_canvas->displayColorConverter()->toQColor(color));
            if (!selectingBg) {
                m_selector->slotSetColor(color);
            }
        }
        break;
    case KoCanvasResource::BackgroundColor:
        if (!m_pendingBgUpdate) {
            KoColor color = value.value<KoColor>();
            m_toggle->setBackgroundColor(m_canvas->displayColorConverter()->toQColor(color));
            if (selectingBg) {
                m_selector->slotSetColor(color);
            }
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
        KisConfigNotifier::instance()->notifyConfigChanged();
    }
}
