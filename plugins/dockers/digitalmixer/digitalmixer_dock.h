/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _DIGITALMIXER_DOCK_H_
#define _DIGITALMIXER_DOCK_H_

#include <QPointer>
#include <kddockwidgets/DockWidget.h>

#include <KoColor.h>
#include <KoCanvasObserverBase.h>

#include <KoCanvasBase.h>

class KoColorPopupAction;
class KoColorSlider;
class KoColorPatch;
class KisColorButton;

class DigitalMixerDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    DigitalMixerDock( );
    QString observerName() override { return "DigitalMixerDock"; }
    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override { m_canvas = 0; setEnabled(false);}
public Q_SLOTS:
    void setCurrentColor(const KoColor& );
    void canvasResourceChanged(int, const QVariant&);
private Q_SLOTS:
    void popupColorChanged(int i);
    void colorSliderChanged(int i);
    void targetColorChanged(int);

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
};


#endif
