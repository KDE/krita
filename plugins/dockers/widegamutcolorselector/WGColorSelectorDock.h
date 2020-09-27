/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WG_COLOR_SELECTOR_DOCKER_H
#define WG_COLOR_SELECTOR_DOCKER_H

#include <QPointer>
#include <QDockWidget>
#include <KoCanvasObserverBase.h>

#include <KoColor.h>

class KisCanvas2;
class KisColorSourceToggle;
class KisSignalCompressor;
class KisVisualColorSelector;
class WGColorPatches;
class WGColorPreviewPopup;
class WGQuickSettingsWidget;
class WGShadeSelector;
class QToolButton;
class QVariant;
class QWidgetAction;

class WGColorSelectorDock : public QDockWidget, public KoCanvasObserverBase // public KisMainwindowObserver ?
{
    Q_OBJECT
public:
    WGColorSelectorDock();
protected:
    void leaveEvent(QEvent *event) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    void disconnectFromCanvas();

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
    KisVisualColorSelector *m_selector {0};
    KisColorSourceToggle *m_toggle {0};
    KisSignalCompressor *m_colorChangeCompressor;
    WGColorPreviewPopup *m_previewPopup {0};
    WGShadeSelector *m_shadeSelector {0};
    WGColorPatches *m_history {0};
    QWidgetAction *m_quickSettingAction {0};
    WGQuickSettingsWidget *m_quickSettings {0};
    QToolButton *m_configButton {0};
    bool m_pendingFgUpdate {false};
    bool m_pendingBgUpdate {false};
    KoColor m_fgColor;
    KoColor m_bgColor;
};

#endif // WG_COLOR_SELECTOR_DOCKER_H
