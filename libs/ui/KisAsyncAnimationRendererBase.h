/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCANIMATIONRENDERERBASE_H
#define KISASYNCANIMATIONRENDERERBASE_H

#include <QObject>
#include "kis_types.h"

#include "kritaui_export.h"

class KisRegion;

/**
 * KisAsyncAnimationRendererBase is a special class representing a
 * single worker thread inside KisAsyncAnimationRenderDialogBase. It connects
 * the specified image using correct Qt::DirectConnection connections and
 * reacts on them. On sigFrameReady() signal it calls frameCompletedCallback(),
 * so the derived class can fetch a frame from the image and process it. On
 * sigFrameCancelled() it calls frameCancelledCallback(). The derived class
 * should override these two methods to do the actual work.
 */

class KRITAUI_EXPORT KisAsyncAnimationRendererBase : public QObject
{
    Q_OBJECT

public:
    enum Flag
    {
        None = 0x0,
        Cancellable = 0x1
    };
    Q_DECLARE_FLAGS(Flags, Flag)

public:
    explicit KisAsyncAnimationRendererBase(QObject *parent = 0);
    virtual ~KisAsyncAnimationRendererBase();

    /**
     * Initiates the rendering of the frame \p frame on an image \p image.
     * Only \p regionOfInterest is regenerated. If \p regionOfInterest is
     * empty, then entire bounds of the image is regenerated.
     */
    void startFrameRegeneration(KisImageSP image, int frame, const KisRegion &regionOfInterest, Flags flags = None);

    /**
     * Convenience overload that regenerates the full image
     */
    void startFrameRegeneration(KisImageSP image, int frame, Flags flags = None);

    /**
     * @return true if the regeneration process is in progress
     */
    bool isActive() const;

public Q_SLOTS:
    /**
     * @brief cancels current rendering operation
     *
     * After calling this slot requestedImage() becomes invalid.
     * @see requestedImage()
     */
    void cancelCurrentFrameRendering();

Q_SIGNALS:
    void sigFrameCompleted(int frame);
    void sigFrameCancelled(int frame);

private Q_SLOTS:
    void slotFrameRegenerationCancelled();
    void slotFrameRegenerationFinished(int frame);

protected Q_SLOTS:
    /**
     * Called by a derived class to continue processing of the frames
     */
    void notifyFrameCompleted(int frame);

    /**
     * Called by a derived class to cancel processing of the frames. After calling
     * this method, the dialog will stop processing the frames and close.
     */
    void notifyFrameCancelled(int frame);

protected:
    /**
     * @brief frameCompletedCallback is called by the renderer when
     *        a new frame becomes ready
     *
     * NOTE1: the callback is called from the context of a image
     *        worker thread! So it is asynchronous from the GUI thread.
     * NOTE2: in case of successful processing of the frame, the callback
     *        must issue some signal, connected to notifyFrameCompleted()
     *        via auto connection, to continue processing. Please do not
     *        call the method directly, because notifyFame*() slots should
     *        be called from the context of the GUI thread.
     * NOTE3: In case of failure, notifyFrameCancelled(). The same threading
     *        rules apply.
     */
    virtual void frameCompletedCallback(int frame, const KisRegion &requestedRegion) = 0;

    /**
     * @brief frameCancelledCallback is called when the rendering of
     *        the frame was cancelled.
     *
     * The rendering of the frame can be either cancelled by the image itself or
     * by receiving a timeout signal (10 seconds).
     *
     * NOTE: the slot is called in the GUI thread. Don't forget to call
     *       notifyFrameCancelled() in he end of your call.
     */
    virtual void frameCancelledCallback(int frame) = 0;


    /**
     * Called by KisAsyncAnimationRendererBase when the processing has been completed
     * and the internal state of the populator should be cleared
     *
     * @param isCancelled tells if frame regeneration has failed to be regenerated
     */
    virtual void clearFrameRegenerationState(bool isCancelled);

protected:
    /**
     * @return the image that for which the rendering was requested using
     * startFrameRegeneration(). Should be used by the derived classes only.
     *
     * Please note that requestedImage() will become null as soon as the user
     * cancels the processing. That happens in the GUI thread so
     * frameCompletedCallback() should be extremely careful when requesting the
     * value (check the shared pointer after fetching).
     */
    KisImageSP requestedImage() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisAsyncAnimationRendererBase::Flags)

#endif // KISASYNCANIMATIONRENDERERBASE_H
