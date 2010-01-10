/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef VIDEODATA_H
#define VIDEODATA_H

#include <QUrl>

#include <KoShapeUserData.h>

class QIODevice;
class VideoCollection;
class VideoDataPrivate;
class KoStore;
class KTemporaryFile;

/**
 * This class is meant to represent the video data so it can be shared between video shapes.
 * This class inherits from KoShapeUserData which means you can set it on any KoShape using
 * KoShape::setUserData() and get it using KoShape::userData().  The videoshape plugin
 * uses this class to show its video data.
 */
class VideoData : public KoShapeUserData
{
    Q_OBJECT
public:
    /// Various error codes representing what has gone wrong
    enum ErrorCode {
        Success,
        OpenFailed,
        StorageFailed, ///< This is set if the video data has to be stored on disk in a temporary file, but we failed to do so
        LoadFailed
    };

    /// default constructor, creates an invalid imageData object
    VideoData();

    /**
     * copy constructor
     * @param videoData the other one.
     */
    VideoData(const VideoData &videoData);
    
    /// destructor
    virtual ~VideoData();

    void setExternalVideo(const QUrl &location, VideoCollection *collection = 0);
    void setVideo(const QString &location, KoStore *store, VideoCollection *collection = 0);

    /**
     * Save the video data to the param device.
     * The full file is saved.
     * @param device the device that is used to get the data from.
     * @return returns true if load was successful.
     */
    bool saveData(QIODevice &device);

    QString tagForSaving(int &counter);
    
    VideoData &operator=(const VideoData &other);

    inline bool operator!=(const VideoData &other) const { return !operator==(other); }
    bool operator==(const VideoData &other) const;

    /**
     * a unique key of the video data
     */
    qint64 key;

    QString suffix; // the suffix of the picture e.g. png  TODO use a QByteArray ?

    /// returns if this is a valid imageData with actual video data present on it.
    bool isValid() const;

    ErrorCode errorCode;
    
    QString saveName;
    QUrl videoLocation;

protected:
    friend class VideoCollection;

    /// store the suffix based on the full filename.
    void setSuffix(const QString &fileName);

    /// take the data from \a device and store it in the temporaryFile
    void copyToTemporary(QIODevice &device);

    void clear();

    static qint64 generateKey(const QByteArray &bytes);

    enum DataStoreState {
        StateEmpty,     ///< No video data, possible an external video
        StateSpooled, ///< Video data is spooled
    };

    VideoCollection *collection;

    // video data store.
    DataStoreState dataStoreState;

    KTemporaryFile *temporaryFile;
};

#endif
