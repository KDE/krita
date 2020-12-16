/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISBUSYWAITBROKER_H
#define KISBUSYWAITBROKER_H

#include <kis_types.h>
#include "kritaimage_export.h"

/**
 * @brief a simple singleton class for tracking busy-waits on the image and
 * breaking deadlock ties when needed.
 *
 * When KisImage::barrierLock()/waitForDone() goes into sleep, waiting for the
 * worker threads to finish, it notifies KisBusyWaitBroker about that. Therefore
 * worker threads know that GUI thread is inactive now and can resolve the ties.
 */
class KRITAIMAGE_EXPORT KisBusyWaitBroker
{
public:
    KisBusyWaitBroker();
    ~KisBusyWaitBroker();

    static KisBusyWaitBroker* instance();

    void notifyWaitOnImageStarted(KisImage *image);
    void notifyWaitOnImageEnded(KisImage *image);

    void notifyGeneralWaitStarted();
    void notifyGeneralWaitEnded();

    /**
     * Set a callback that is called before image goes into sleep. This callback
     * may show the user some feedback window with a progress bar.
     *
     * This callback is expected to be initialized dorung the construction
     * of KisPart.
     */
    void setFeedbackCallback(std::function<void(KisImageSP)> callback);

    /**
     * @return true if GUI thread is currently sleeping on an image lock. When a worker
     * thread is sure that GUI thread is sleeping, it has a bit more freedom in manipulating
     * the image (and especially vector shapes)
     */
    bool guiThreadIsWaitingForBetterWeather() const;

private:

private:
    Q_DISABLE_COPY(KisBusyWaitBroker);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISBUSYWAITBROKER_H
