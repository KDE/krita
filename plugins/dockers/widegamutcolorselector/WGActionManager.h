/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGPOPUPMANAGER_H
#define WGPOPUPMANAGER_H

#include <KisVisualColorModel.h>
#include <QObject>

class QAction;
class KisCanvas2;
class KisSignalCompressor;
class KisViewManager;
class KisVisualColorSelector;

class WGColorSelectorDock;
class WGSelectorPopup;
class WGShadeSelector;

class WGActionManager : public QObject
{
    Q_OBJECT
public:
    explicit WGActionManager(WGColorSelectorDock *parentDock = nullptr);

    //void setCanvas(KisCanvas2* canvas, KisCanvas2* oldCanvas);
    void registerActions(KisViewManager *viewManager);
private:
    void preparePopup();
    void modifyHSX(int channel, float amount);
private Q_SLOTS:
    void slotShowColorSelectorPopup();
    void slotShowShadeSelectorPopup();
    void slotIncreaseLightness();
    void slotDecreaseLightness();
    void slotIncreaseSaturation();
    void slotDecreaseSaturation();
    void slotShiftHueCW();
    void slotShiftHueCCW();

    void slotChannelValuesChanged();
    void slotUpdateDocker();
Q_SIGNALS:
private:
    WGColorSelectorDock *m_docker {0};
    KisSignalCompressor *m_colorChangeCompressor;
    WGSelectorPopup *m_colorSelectorPopup {0};
    WGSelectorPopup *m_shadeSelectorPopup {0};
    KisVisualColorSelector *m_colorSelector {0};
    WGShadeSelector *m_shadeSelector {0};
    KisVisualColorModelSP m_colorModel;
    bool m_isSynchronizing {false};
};

#endif // WGPOPUPMANAGER_H
