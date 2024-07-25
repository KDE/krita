/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WG_COLOR_SELECTOR_DOCKER_H
#define WG_COLOR_SELECTOR_DOCKER_H

#include "WGConfig.h"
#include "WGSelectorWidgetBase.h"

#include <QPointer>
#include <QDockWidget>
#include <kis_mainwindow_observer.h>
#include <KisVisualColorModel.h>

#include <KoColor.h>

class KisCanvas2;
class KisUniqueColorSet;
class KisColorSourceToggle;
class KisDisplayColorConverter;
class KisSignalCompressor;
class KisVisualColorSelector;
class WGActionManager;
class WGColorPatches;
class WGColorPreviewToolTip;
class WGCommonColorSet;
class WGQuickSettingsWidget;
class WGShadeSelector;
class QBoxLayout;
class QToolButton;
class QVariant;
class QWidgetAction;

class WGColorSelectorDock : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    enum ColorSpaceSource {
        LayerColorSpace,
        ImageColorSpace,
        FixedColorSpace
    };

    WGColorSelectorDock();
    const KisVisualColorModel& colorModel() const;
    KisDisplayColorConverter* displayColorConverter(bool rawPointer = false) const;
    KisUniqueColorSet* colorHistory() const { return m_colorHistory; }

    bool selectingBackground() const;
    /**
     * @brief Set new channel values; takes effect immediately!
     *
     * This is intended for synchronization when color gets
     * adjusted outside of the docker, like in WGActionManager
     * @param values
     */
    void setChannelValues(const QVector4D &values);
protected:
    void leaveEvent(QEvent *event) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager* viewManager) override;

    void disconnectFromCanvas();
    void updateLayout();

private Q_SLOTS:
    void slotConfigurationChanged();
    void slotDisplayConfigurationChanged();
    void slotColorSelected(const KoColor &color);
    void slotColorSourceToggled(bool selectingBg);
    void slotColorInteraction(bool active);
    void slotFGColorUsed(const KoColor&color);
    void slotSetNewColors();
    void slotCanvasResourceChanged(int key, const QVariant &value);
    void slotOpenSettings();
private:
    QPointer<KisCanvas2> m_canvas;
    WGSelectorDisplayConfigSP m_displayConfig;
    // Layouts
    QBoxLayout *m_mainWidgetLayout {0};
    QBoxLayout *m_selectorAreaLayout {0};
    QBoxLayout *m_verticalElementsLayout {0};
    // Selector Elements
    KisVisualColorSelector *m_selector {0};
    KisColorSourceToggle *m_toggle {0};
    KisSignalCompressor *m_colorChangeCompressor;
    KisUniqueColorSet *m_colorHistory;
    WGCommonColorSet *m_commonColorSet {0};
    WGActionManager *m_actionManager {0};
    WGColorPreviewToolTip *m_colorTooltip {0};
    WGShadeSelector *m_shadeSelector {0};
    WGColorPatches *m_history {0};
    WGColorPatches *m_commonColors {0};
    QWidgetAction *m_quickSettingAction {0};
    WGQuickSettingsWidget *m_quickSettings {0};
    QToolButton *m_configButton {0};
    KisVisualColorModelSP m_colorModelFG;
    KisVisualColorModelSP m_colorModelBG;
    const KoColorSpace *m_customCS {0};
    ColorSpaceSource m_CSSource {LayerColorSpace};
    bool m_pendingFgUpdate {false};
    bool m_pendingBgUpdate {false};
};

namespace WGConfig {
    extern const NumericSetting<WGColorSelectorDock::ColorSpaceSource> colorSpaceSource;
}

#endif // WG_COLOR_SELECTOR_DOCKER_H
