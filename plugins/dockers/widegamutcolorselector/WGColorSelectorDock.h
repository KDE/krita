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

class KisCanvas2;
class KisVisualColorSelector;
class KoColor;
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
    void slotCanvasResourceChanged(int key, const QVariant &value);
private:
    QPointer<KisCanvas2> m_canvas;
    KisVisualColorSelector *m_selector {0};
};

#endif // WG_COLOR_SELECTOR_DOCKER_H
