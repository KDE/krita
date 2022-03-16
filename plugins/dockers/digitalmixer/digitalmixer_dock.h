/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 Bourumir Wyngs <bourumir.wyngs@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _DIGITALMIXER_DOCK_H_
#define _DIGITALMIXER_DOCK_H_

#include <QPointer>
#include <QDockWidget>
#include <QPushButton>

#include <KoColor.h>
#include <KoCanvasBase.h>

#include <kis_workspace_resource.h>
#include <kis_mainwindow_observer.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>

class KoColorPopupAction;
class KoColorSlider;
class KoColorPatch;
class KisColorButton;

class DigitalMixerDock : public QDockWidget, public KisMainwindowObserver {
    Q_OBJECT
public:
    DigitalMixerDock( );
    QString observerName() override { return "DigitalMixerDock"; }
    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override { m_canvas = 0; setEnabled(false);}

public: // KisMainWindowObserver
  void setViewManager(KisViewManager* kisview) override;

public Q_SLOTS:
    void setCurrentColor(const KoColor& );
    void canvasResourceChanged(int, const QVariant&);

private Q_SLOTS:
    void popupColorChanged(int i);
    void colorSliderChanged(int i);
    void targetColorChanged(int);

    void resetMixer();
    void saveToWorkspace(KisWorkspaceResourceSP workspace);
    void loadFromWorkspace(KisWorkspaceResourceSP workspace);

    void gradientStartColorChanged(int);
    void gradientColorSliderChanged(int);
    void gradientEndColorChanged(int);
    void gradientTargetColorChanged(int);

private:
    QPointer<KoCanvasBase> m_canvas;
    KoColor m_currentColor;
    KoColorPatch* m_currentColorPatch;

    struct Mixer {
      KoColorPatch* targetColor;
      KoColorSlider* targetSlider;
      KisColorButton* actionColor;
    };

    struct GradientMixer {
        KoColorPatch* targetColor;
        KisColorButton* startColor;
        KoColorSlider* targetSlider;
        KisColorButton* endColor;
    };

    QList<Mixer> m_mixers;
    GradientMixer m_gradientMixer;
    bool m_tellCanvas;

    QPushButton *m_reset_button;
};


#endif
