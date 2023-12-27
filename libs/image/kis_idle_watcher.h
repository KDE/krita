/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IDLE_WATCHER_H
#define __KIS_IDLE_WATCHER_H

#include "kritaimage_export.h"

#include <QScopedPointer>
#include <QObject>
#include <QString>

#include "kis_types.h"


class KRITAIMAGE_EXPORT KisIdleWatcher : public QObject
{
    Q_OBJECT
public:
    KisIdleWatcher(int delay = 200, QObject* parent = 0);
    ~KisIdleWatcher() override;

    bool isIdle() const;
    bool isCounting() const;

    void setTrackedImages(const QVector<KisImageSP> &images);
    void setTrackedImage(KisImageSP image);

    //Force to image modified state and start countdown to event
    void forceImageModified() { slotImageModified(); }
    void restartCountdown();
    void triggerCountdownNoDelay();

Q_SIGNALS:
    void startedIdleMode();
    void imageModified();

private Q_SLOTS:
    void slotImageModified();
    void slotIdleCheckTick();

    void startIdleCheck();
    void stopIdleCheck();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_IDLE_WATCHER_H */
