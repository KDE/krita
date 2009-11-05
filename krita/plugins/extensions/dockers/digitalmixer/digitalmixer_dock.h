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

class KoColorPopupAction;
class KoColorSlider;
class KoColorPatch;
class KisView2;

class DigitalMixerDock : public QDockWidget {
    Q_OBJECT
public:
    DigitalMixerDock( KisView2 *view );
private slots:
    void popupColorChanged(int i);
private:
    KisView2* m_view;
    KoColor m_currentColor;
    struct Mixer {
      KoColorPatch* targetColor;
      KoColorSlider* targetSlider;
      KoColorPopupAction* actionColor;
    };
    QList<Mixer> m_mixers;
};


#endif
