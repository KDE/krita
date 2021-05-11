/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef H_ARTISTIC_COLOR_SELECTOR_DOCK_H
#define H_ARTISTIC_COLOR_SELECTOR_DOCK_H

#include <QDockWidget>
#include <QPointer>
#include <QRegExpValidator>

#include <KoCanvasObserverBase.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
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
    void slotGamutMaskDeactivate();
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
