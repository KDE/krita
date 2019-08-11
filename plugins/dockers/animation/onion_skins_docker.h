/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

class OnionSkinsDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT

public:
    explicit OnionSkinsDocker(QWidget *parent = 0);
    ~OnionSkinsDocker() override;

    QString observerName() override { return "OnionSkinsDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;

private:
    Ui::OnionSkinsDocker *ui;

    KisSignalCompressor m_updatesCompressor;
    KisEqualizerWidget *m_equalizerWidget;
    KisAction *m_toggleOnionSkinsAction;

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
