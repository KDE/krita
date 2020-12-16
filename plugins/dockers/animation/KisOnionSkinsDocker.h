/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ONION_SKINS_DOCKER_H
#define ONION_SKINS_DOCKER_H

#include <QDockWidget>

#include <kis_mainwindow_observer.h>
#include "kis_signal_compressor.h"

class KisAction;

namespace Ui {
class OnionSkinsDocker;
}

class KisEqualizerWidget;

class KisOnionSkinsDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT

public:
    explicit KisOnionSkinsDocker(QWidget *parent = 0);
    ~KisOnionSkinsDocker() override;

    QString observerName() override { return "OnionSkinsDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;

private:
    Ui::OnionSkinsDocker *ui;

    KisSignalCompressor m_updatesCompressor;
    KisEqualizerWidget *m_equalizerWidget;
    KisAction *m_toggleOnionSkinsAction;

    class KisColorLabelFilterGroup *m_filterButtonGroup;
    class KisColorLabelMouseDragFilter *m_dragFilter;

private:
    void loadSettings();

private Q_SLOTS:
    void changed();
    void slotShowAdditionalSettings(bool value);
    void slotUpdateIcons();
    void slotToggleOnionSkins();
    void slotFilteredColorsChanged();
};

#endif // ONION_SKINS_DOCKER_H
