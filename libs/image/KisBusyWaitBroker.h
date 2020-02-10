/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
