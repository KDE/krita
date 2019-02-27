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
#include <QPointer>
#include <QRegExpValidator>

#include <KoCanvasObserverBase.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerObserver.h>
#include <resources/KoGamutMask.h>
#include <KisDocument.h>
#include <kis_types.h>
#include <KisResourceItemChooser.h>

#include <kis_mainwindow_observer.h>

class KisCanvasResourceProvider;
class KisColor;
class QButtonGroup;
class QMenu;

struct ArtisticColorSelectorUI;
struct ARCSSettingsUI;
struct WheelPreferencesPopupUI;

class ArtisticColorSelectorDock: public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
    
public:
    ArtisticColorSelectorDock();
    ~ArtisticColorSelectorDock() override;
    QString observerName() override { return "ArtisticColorSelectorDock"; }
    void setViewManager(KisViewManager* kisview) override;
    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override;

private Q_SLOTS:
    void slotCanvasResourceChanged(int key, const QVariant& value);
    void slotFgColorChanged(const KisColor& color);
    void slotBgColorChanged(const KisColor& color);
    void slotColorSpaceSelected();
    void slotPreferenceChanged();
    void slotResetDefaultSettings();
    void slotGamutMaskToggle(bool value);
    void slotGamutMaskSet(KoGamutMaskSP mask);
    void slotGamutMaskUnset();
    void slotGamutMaskPreviewUpdate();
    void slotSelectorSettingsChanged();

private:
    KisCanvas2* m_canvas;
    KisCanvasResourceProvider* m_resourceProvider;
    QButtonGroup*            m_hsxButtons;
    ArtisticColorSelectorUI* m_selectorUI;
    ARCSSettingsUI* m_preferencesUI;
    WheelPreferencesPopupUI* m_wheelPrefsUI;
    KoGamutMaskSP m_selectedMask;

    QIcon m_iconMaskOff;
    QIcon m_iconMaskOn;

    QPixmap m_infinityPixmap;
};


#endif // H_ARTISTIC_COLOR_SELECTOR_DOCK_H
