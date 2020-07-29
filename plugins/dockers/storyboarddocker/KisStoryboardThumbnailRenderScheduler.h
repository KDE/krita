/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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

#ifndef KISASYNCSTORYBOARDTHUMBNAILRENDERSCHEDULER_H
#define KISASYNCSTORYBOARDTHUMBNAILRENDERSCHEDULER_H

#include <QObject>
#include <QVector>

#include <kis_image.h>

class KisPaintDevice;
class KisAsyncStoryboardThumbnailRenderer;

/*
This class gets the modified frames and maintains queues of dirty frames sorted
in the order of proximity to the last changed frame. It regenerates the frames
one by one and emits the paintdevice for each of the frames. It
schedules the regeneration of frames based on the list of dirty frames maintained.
The m_changedFramesQueue list is given preference when regenerating frames.
*/
class KisStoryboardThumbnailRenderScheduler : public QObject
{
    Q_OBJECT
public:
    KisStoryboardThumbnailRenderScheduler(QObject *parent);
    ~KisStoryboardThumbnailRenderScheduler();

    /*
    The class takes an image, clones it and performs all the regenrations
    on the clone so no need to pass cloned image explicitly. Also the image
    should be set only once and not every time before scheduling a frame
    for regenration.
    */
    void setImage(KisImageSP image);
    /*
    adds the frame to the list of "to be rendered" frames. the affected
    argument denotes whether this frame was directly changed or affected
    by changes made to other keyframes.
    */
    void scheduleFrameForRegeneration(int frame, bool affected);

private Q_SLOTS:
    /*
    emits the paintDevice for the frame if the regeneration was complete.
    Also calls regenration of the next frame in queue.
    */
    void slotFrameRegenerationCompleted(int frame);

    /*
    schedules the next frame for regenration.
    */
    void slotFrameRegenerationCancelled(int frame);

private:
    /*
    sorts the m_affectedFramesQueue based on proximity to the last changed frame
    */
    void sortAffectedFrameQueue();

    /*
    renders the next frame, either from affected or changed queue. Changed
    queue is given preference. It removes the the frame from the queue right after
    calling the startFrameRegeneration().
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
};

#endif
