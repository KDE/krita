/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _DIGITALMIXER_DOCK_H_
#define _DIGITALMIXER_DOCK_H_

#include <QDockWidget>
#include <KoColor.h>
#include <KoCanvasObserver.h>

class KoColorPopupAction;
class KoColorSlider;
class KoColorPatch;
class KisView2;

class DigitalMixerDock : public QDockWidget, public KoCanvasObserver {
    Q_OBJECT
public:
    DigitalMixerDock( KisView2 *view );
    /// reimplemented from KoCanvasObserver
    virtual void setCanvas(KoCanvasBase *canvas);
public slots:
    void setCurrentColor(const KoColor& );
    void resourceChanged(int, const QVariant&);
private slots:
    void popupColorChanged(int i);
    void colorSliderChanged(int i);
    void targetColorChanged(int);
private:
    KoCanvasBase* m_canvas;
    KisView2* m_view;
    KoColor m_currentColor;
    KoColorPatch* m_currentColorPatch;
    struct Mixer {
      KoColorPatch* targetColor;
      KoColorSlider* targetSlider;
      KoColorPopupAction* actionColor;
    };
    QList<Mixer> m_mixers;
    bool m_tellCanvas;
};


#endif
