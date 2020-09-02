/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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

#ifndef _TIMELINE_DOCKER_H_
#define _TIMELINE_DOCKER_H_

#include "kritaimage_export.h"

#include <QScopedPointer>
#include <QDockWidget>

#include <kis_mainwindow_observer.h>
#include <kis_utility_title_bar.h>

#ifdef Q_OS_MACOS
#include <sys/types.h>
#endif

class QPushButton;
class QToolButton;
class KisTransportControls;
class KisIntParseSpinBox;
class KisSliderSpinBox;

class KisCanvas2;
class KisAction;


/** @brief A customized titlebar for the Animation Timeline Docker that's
 * packed with useful widgets and menus.
 *
 * To avoid cluttering the UI, elements that are important to the
 * animator's workflow should be available at a glace, while
 * set-and-forget types of things should be hidden inside of menus.
 */
class TimelineDockerTitleBar : public KisUtilityTitleBar
{
    Q_OBJECT

public:
    TimelineDockerTitleBar(QWidget *parent = nullptr);

    KisTransportControls* transport;

    KisIntParseSpinBox *frameRegister;

    QToolButton *btnAddKeyframe;
    QToolButton *btnDuplicateKeyframe;
    QToolButton *btnRemoveKeyframe;

    QPushButton *btnOnionSkinsMenu;
    QPushButton *btnAudioMenu;
    QToolButton *btnSettingsMenu;

    KisIntParseSpinBox *sbStartFrame;
    KisIntParseSpinBox *sbEndFrame;
    KisIntParseSpinBox *sbFrameRate;
    KisSliderSpinBox *sbSpeed;

    QToolButton *btnDropFrames;

    QToolButton *btnAutoKey;
    QAction *autoKeyBlank;
    QAction *autoKeyDuplicate;

private:
    const int MAX_FRAMES = 9999;
};


/** @brief Krita's Animation Timeline Docker.
 * This is the GUI heart of Krita's traditional animation workflow,
 * and is where artists can configure, edit, scrub and play their animation.
 *
 * Currently interacts with the TimelinFramesView/Model as well as
 * the KisImageAnimationInterface. (TODO: Consider refactoring to
 * streamline this interaction towards Docker -> AnimationPlayer -> ImageAnimInterface)
 */
class TimelineDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    TimelineDocker();
    ~TimelineDocker() override;

    QString observerName() override { return "TimelineDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;

public Q_SLOTS:
    void playPause();
    void stop();
    void previousFrame();
    void nextFrame();
    void previousKeyframe();
    void nextKeyframe();
    void previousMatchingKeyframe();
    void nextMatchingKeyframe();

    void goToFrame(int frameIndex);
    void setStartFrame(int frame);
    void setEndFrame(int frame);
    void setFrameRate(int frmaerate);
    void setPlaybackSpeed(int playbackSpeed);
    void setDropFrames(bool dropFrames);
    void setAutoKey(bool value);

    void handleClipRangeChange();
    void handleFrameRateChange();

    void updateFrameCache();
    void updateFrameRegister();
    void updatePlaybackStatistics();

    void handleThemeChange();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
