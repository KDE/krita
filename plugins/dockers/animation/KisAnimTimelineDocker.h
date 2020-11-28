/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class KisAnimTimelineDocker : public KisUtilityTitleBar
{
    Q_OBJECT

public:
    KisAnimTimelineDocker(QWidget *parent = nullptr);

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
    void setAutoKey(bool value);

    void handleClipRangeChange();
    void handleFrameRateChange();
    void handlePlaybackSpeedChange(double normalizedPlaybackSpeed);

    void updateFrameCache();
    void updateFrameRegister();
    void updatePlaybackStatistics();

    void handleThemeChange();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
