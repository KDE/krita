/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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


class KisCanvas2;
class KisAction;

class KisAnimationCurveDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    KisAnimationCurveDocker();
    ~KisAnimationCurveDocker() override;

    QString observerName() override { return "AnimationCurveDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;

private:
    void addKeyframe(const QString &channel);
    void removeKeyframe(const QString &channel);

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state);
    void slotNodeActivated(KisNodeSP node);

private Q_SLOTS:
    void slotUpdateIcons();

    void slotAddAllEnabledKeys();
    void slotAddOpacityKey();
    void slotRemoveOpacityKey();

    void slotListRowsInserted(const QModelIndex &parentIndex, int first, int last);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
