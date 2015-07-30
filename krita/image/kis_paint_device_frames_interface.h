/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PAINT_DEVICE_FRAMES_INTERFACE_H
#define __KIS_PAINT_DEVICE_FRAMES_INTERFACE_H

#include "kis_types.h"
#include "krita_export.h"

class KisPaintDeviceData;
class KisPaintDeviceWriter;


class KRITAIMAGE_EXPORT KisPaintDeviceFramesInterface
{
public:
    KisPaintDeviceFramesInterface(KisPaintDevice *parentDevice);

    /**
     * Return a list of IDs for the frames contained in this paint device
     * @return list of frame IDs
     */
    QList<int> frames();

    /**
     * Creates a new frame on the device and returns an identifier for it.
     * @return frame id
     */
    int createFrame(bool copy, int copySrc, const QPoint &offset);

    /**
     * Delete the frame with given id
     * @param frame frame ID
     */
    void deleteFrame(int frame);

    /**
     * Copy the given frame into the target device
     * @param frameId ID of the frame to be copied
     * @param targetDevice paint device to copy to
     */
    void fetchFrame(int frameId, KisPaintDeviceSP targetDevice);

    /**
     * @return extent() of \p frameId
     */
    QRect frameBounds(int frameId);

    /**
     * @return offset of a data on \p frameId
     */
    QPoint frameOffset(int frameId) const;

    /**
     * Sets default pixel for \p frameId
     */
    void setFrameDefaultPixel(const quint8 *defPixel, int frameId);

    /**
     * @return default pixel for \p frameId
     */
    const quint8* frameDefaultPixel(int frameId) const;

    /**
     * Write a \p frameId onto \p store
     */
    bool writeFrame(KisPaintDeviceWriter &store, int frameId);

    /**
     * Loads content of a \p frameId from \p stream.
     *
     * NOTE: the frame must be created manually with createFrame()
     *       beforehand!
     */
    bool readFrame(QIODevice *stream, int frameId);


    struct TestingDataObjects {
        typedef KisPaintDeviceData Data;
        typedef QHash<int, Data*> FramesHash;

        Data *m_data;
        Data *m_lodData;
        Data *m_externalFrameData;

        FramesHash m_frames;

        Data *m_currentData;
    };

    TestingDataObjects testingGetDataObjects() const;
    QList<KisPaintDeviceData*> testingGetDataObjectsList() const;

private:
    KisPaintDevice *q;
};

#endif /* __KIS_PAINT_DEVICE_FRAMES_INTERFACE_H */
