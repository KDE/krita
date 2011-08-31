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

#ifndef H_ARTISTIC_COLOR_SELECTOR_DOCK_H
#define H_ARTISTIC_COLOR_SELECTOR_DOCK_H

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

class KisColor;
class QButtonGroup;
class QMenu;
struct ArtisticColorSelectorUI;
struct ColorPreferencesPopupUI;

class ArtisticColorSelectorDock: public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
    
public:
     ArtisticColorSelectorDock();
    ~ArtisticColorSelectorDock();
    
    virtual void setCanvas(KoCanvasBase* canvas);
    virtual void unsetCanvas() { m_canvas = 0; }
    
private slots:
    void slotResourceChanged(int key, const QVariant& value);
    void slotFgColorChanged(const KisColor& color);
    void slotBgColorChanged(const KisColor& color);
    void slotColorSpaceSelected(int type);
    void slotPreferenceChanged();
    void slotMenuActionTriggered(QAction* action);
    void slotResetDefaultSettings();
    void slotLightModeChanged(bool setToAbsolute);
    void slotDockLocationChanged(Qt::DockWidgetArea area);
    void slotTopLevelChanged(bool topLevel);
    
private:
    KoCanvasBase*            m_canvas;
    QButtonGroup*            m_hsxButtons;
    QMenu*                   m_resetMenu;
    ArtisticColorSelectorUI* m_selectorUI;
    ColorPreferencesPopupUI* m_preferencesUI;
};


#endif // H_ARTISTIC_COLOR_SELECTOR_DOCK_H
