/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGPOPUPMANAGER_H
#define WGPOPUPMANAGER_H

#include "WGSelectorWidgetBase.h"

#include <KisVisualColorModel.h>
#include <QObject>

class QAction;
class KisCanvas2;
class KisSignalCompressor;
class KisViewManager;
class KisVisualColorSelector;

class WGColorPreviewToolTip;
class WGColorSelectorDock;
class WGMyPaintShadeSelector;
class WGSelectorPopup;
class WGShadeSelector;
namespace WGConfig {
typedef class WGConfig Accessor;
}

class WGActionManager : public QObject
{
    Q_OBJECT
public:
    explicit WGActionManager(WGColorSelectorDock *parentDock = nullptr);
    ~WGActionManager() override;

    void setCanvas(KisCanvas2* canvas, KisCanvas2* oldCanvas);
    void registerActions(KisViewManager *viewManager);
    void setLastUsedColor(const KoColor &col);
private:
    void updateWidgetSize(QWidget *widget, int size);
    void showPopup(WGSelectorPopup *popup);
    void loadColorSelectorSettings(WGConfig::Accessor &cfg);
    void modifyHSX(int channel, float amount);
private Q_SLOTS:
    void slotConfigChanged();
    void slotSelectorConfigChanged();
    void slotPopupClosed(WGSelectorPopup *popup);
    void slotShowColorSelectorPopup();
    void slotShowShadeSelectorPopup();
    void slotShowMyPaintSelectorPopup();
    void slotShowColorHistoryPopup();
    void slotIncreaseLightness();
    void slotDecreaseLightness();
    void slotIncreaseSaturation();
    void slotDecreaseSaturation();
    void slotShiftHueCW();
    void slotShiftHueCCW();

    void slotChannelValuesChanged();
    void slotColorInteraction(bool active);
    void slotColorPatchInteraction(bool active);
    void slotColorSelected(const KoColor &color);
    void slotUpdateDocker();
Q_SIGNALS:
private:
    WGColorSelectorDock *m_docker {0};
    WGSelectorDisplayConfigSP m_displayConfig;
    WGColorPreviewToolTip *m_colorTooltip;
    KisSignalCompressor *m_colorChangeCompressor;
    WGSelectorPopup *m_currentPopup {0};
    WGSelectorPopup *m_colorSelectorPopup {0};
    WGSelectorPopup *m_shadeSelectorPopup {0};
    WGSelectorPopup *m_myPaintSelectorPopup {0};
    WGSelectorPopup *m_colorHistoryPopup {0};
    KisVisualColorSelector *m_colorSelector {0};
    WGShadeSelector *m_shadeSelector {0};
    WGMyPaintShadeSelector *m_myPaintSelector {0};
    KisVisualColorModelSP m_colorModel;
    KoColor m_lastUsedColor;
    bool m_isSynchronizing {false};
};

#endif // WGPOPUPMANAGER_H
