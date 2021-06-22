/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVE_DOCKER_H_
#define _KIS_ANIMATION_CURVE_DOCKER_H_

#include <QDockWidget>
#include <kis_mainwindow_observer.h>
#include <QScopedPointer>
#include <kis_types.h>
#include <KisKineticScroller.h>
#include <kis_utility_title_bar.h>

class QPushButton;
class QToolButton;
class KisCanvas2;
class KisAction;
class KisTransportControls;
class KisIntParseSpinBox;
class KisDoubleParseSpinBox;
class KisSliderSpinBox;
class KisZoomButton;

/** @brief A customized titlebar for the Animation Curves Docker that's
 * packed with useful widgets and menus.
 *
 * To avoid cluttering the UI, elements that are important to the
 * animator's workflow should be available at a glace, while
 * set-and-forget types of things should be hidden inside of menus.
 */
class KisAnimCurvesDockerTitlebar : public KisUtilityTitleBar
{
    Q_OBJECT

public:
    KisAnimCurvesDockerTitlebar(QWidget *parent = nullptr);

    KisTransportControls* transport;

    KisIntParseSpinBox *sbFrameRegister;

    QToolButton *btnAddKey;
    QToolButton *btnRemoveKey;

    QToolButton *btnInterpConstant;
    QToolButton *btnInterpLinear;
    QToolButton *btnInterpBezier;

    QToolButton *btnTangentSharp;
    QToolButton *btnTangentSmooth;

    KisDoubleParseSpinBox *sbValueRegister;

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

    QToolButton *btnZoomFitRange;
    QToolButton *btnZoomFitCurve;
    KisZoomButton *btnZoomHori;
    KisZoomButton *btnZoomVert;


private:
    const int MAX_FRAMES = 9999;
};

/** @brief Krita's Animation Curves Docker.
 * This is the GUI heart of Krita's scalar animation workflow.
 */
class KisAnimCurvesDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    KisAnimCurvesDocker();
    ~KisAnimCurvesDocker() override;

    QString observerName() override { return "AnimationCurveDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state);
    void slotNodeActivated(KisNodeSP node);
    void updateFrameRegister();

    void handleFrameRateChange();
    void handleClipRangeChange();
    void handlePlaybackSpeedChange(double normalizedSpeed);

private Q_SLOTS:
    void slotUpdateIcons();

    void slotAddAllEnabledKeys();
    void slotAddOpacityKey();
    void slotRemoveSelectedKeys();
    void slotRemoveOpacityKey();

    void slotListRowsInserted(const QModelIndex &parentIndex, int first, int last);

    void slotValueRegisterChanged(double value);

    void slotActiveNodeUpdate(const QModelIndex index);

    void requestChannelMenuAt(const QPoint& point);
    void resetChannelTreeSelection();
private:
    // Used for adding multiple keyframes as a batch under one undo command.
    void addKeyframeCommandToParent(const QString &channelIdentity, KUndo2Command* parentCMD);

    // Used to quickly add one type of specific key automatically, e.g. Opacity.
    void addKeyframeQuick(const QString &channelIdentity);

    void removeKeyframe(const QString &channel);

    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
