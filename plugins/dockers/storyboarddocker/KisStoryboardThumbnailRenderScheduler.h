/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCSTORYBOARDTHUMBNAILRENDERSCHEDULER_H
#define KISASYNCSTORYBOARDTHUMBNAILRENDERSCHEDULER_H

#include <QObject>
#include <QVector>

#include <kis_image.h>

class KisPaintDevice;
class KisAsyncStoryboardThumbnailRenderer;

/**
 * @class KisStoryboardThumbnailRenderScheduler
 * @brief This class maintains queues of dirty frames sorted in the order of proximity
 * to the last changed frame. It regenerates the frames emits the paintdevice for each
 * of the frames. The m_changedFramesQueue list is given preference.
 */
class KisStoryboardThumbnailRenderScheduler : public QObject
{
    Q_OBJECT
public:
    KisStoryboardThumbnailRenderScheduler(QObject *parent);
    ~KisStoryboardThumbnailRenderScheduler();

    /**
     * @brief Sets an image, the class takes an image, clones it and calls frame
     * regeneration on the clone so do not pass cloned image explicitly. The image
     * should be set only_once and not every time before scheduling a frame for
     * regeneration. Setting an image cancels rendering of all previously scheduled frames.
     * @param image Image whose frames are to be rendered.
    */
    void setImage(KisImageSP image);
    /**
     * @brief Adds the frame to the list of "to be regenerated" frames.
     * @param frame To be regenerated frame
     * @param affected Denotes whether this frame was directly changed or affected
     * by changes made to other keyframes.
     */
    void scheduleFrameForRegeneration(int frame, bool affected);
    /**
     * @brief Cancels all frame rendering. Empties all queues and cancels the current rendering, if any.
     */
    void cancelAllFrameRendering();
    /**
     * @brief Cancel rendering of a single frame.
     * @param frame The frame whose rendering is to be cancelled.
     */
    void cancelFrameRendering(int frame);

public Q_SLOTS:
    void slotStartFrameRendering();

private Q_SLOTS:
    /**
     * @brief Emits @c sigFrameCompleted(int,KisPaintDeviceSP) if the regeneration was complete
     * and calls regeneration of the next frame in queue.
     */
    void slotFrameRegenerationCompleted(int frame, KisPaintDeviceSP contents);

    /**
     * @brief Emits @c sigFrameCancelled(int) and schedules the next frame for regeneration.
     */
    void slotFrameRegenerationCancelled(int frame);

private:
    /**
     * @brief Sorts the @c m_affectedFramesQueue based on proximity to the last changed frame
     */
    void sortAffectedFrameQueue();

    /**
     * @brief Renders the next frame, either from affected or changed queue. Changed
     * queue is given preference. It removes the frame from the queue right after
     * calling @c startFrameRegeneration()
     */
    void renderNextFrame();

Q_SIGNALS:
    void sigFrameCompleted(int frame, KisPaintDeviceSP dev);
    void sigFrameCancelled(int frame);

private:
    QVector<int> m_changedFramesQueue;
    QVector<int> m_affectedFramesQueue;
    KisAsyncStoryboardThumbnailRenderer *m_renderer;
    KisImageSP m_image;
    int m_currentFrame;
};

#endif
