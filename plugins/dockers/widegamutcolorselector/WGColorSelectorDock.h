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
class KisSignalCompressor;
class KisVisualColorSelector;
class QVariant;

class WGColorSelectorDock : public QDockWidget, public KoCanvasObserverBase // public KisMainwindowObserver ?
{
    Q_OBJECT
public:
    WGColorSelectorDock();
protected:
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    void disconnectFromCanvas();

private Q_SLOTS:
    void slotDisplayConfigurationChanged();
    void slotColorSelected(const KoColor &color);
    void slotSetNewColors();
    void slotCanvasResourceChanged(int key, const QVariant &value);
private:
    QPointer<KisCanvas2> m_canvas;
    KisVisualColorSelector *m_selector {0};
    KisSignalCompressor *m_colorChangeCompressor;
    bool m_pendingFgUpdate {false};
    bool m_pendingBgUpdate {false};
    KoColor m_fgColor;
    KoColor m_bgColor;
};

#endif // WG_COLOR_SELECTOR_DOCKER_H
