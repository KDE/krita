/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef H_ARTISTIC_COLOR_SELECTOR_DOCK_H
#define H_ARTISTIC_COLOR_SELECTOR_DOCK_H

#include <QDockWidget>
#include <kis_mainwindow_observer.h>

class KisCanvasResourceProvider;
class KisColor;
class QButtonGroup;
class QMenu;
struct ArtisticColorSelectorUI;
struct ColorPreferencesPopupUI;

class ArtisticColorSelectorDock: public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
    
public:
    ArtisticColorSelectorDock();
    ~ArtisticColorSelectorDock() override;
    QString observerName() override { return "ArtisticColorSelectorDock"; }
    void setViewManager(KisViewManager* kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    
private Q_SLOTS:
    void slotCanvasResourceChanged(int key, const QVariant& value);
    void slotFgColorChanged(const KisColor& color);
    void slotBgColorChanged(const KisColor& color);
    void slotColorSpaceSelected(int type);
    void slotPreferenceChanged();
    void slotMenuActionTriggered(QAction* action);
    void slotResetDefaultSettings();
    void slotLightModeChanged(bool setToAbsolute);
    
private:
    KisCanvasResourceProvider* m_resourceProvider;
    QButtonGroup*            m_hsxButtons;
    QMenu*                   m_resetMenu;
    ArtisticColorSelectorUI* m_selectorUI;
    ColorPreferencesPopupUI* m_preferencesUI;
};


#endif // H_ARTISTIC_COLOR_SELECTOR_DOCK_H
