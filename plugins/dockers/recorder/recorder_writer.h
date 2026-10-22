/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_WRITER_H
#define RECORDER_WRITER_H

#include "kis_types.h"
#include "recorder_format.h"

#include <QMutex>
#include <QObject>
#include <QPointer>
#include <kis_paint_device.h>

struct RecorderExportSettings;
class RecorderConfig;
class KisCanvas2;
class QDir;

struct RecorderWriterSettings
{
    QString outputDirectory;
    RecorderFormat format{RecorderFormat::JPEG};
    int quality;
    int compression;
    int resolution;
    double captureInterval;
    bool recordIsolateLayerMode;
    bool realTimeCaptureMode;
    bool focusedRecord;
};

// This class is used to provide mechanism ro sync the number of available
// and used recording threads between the GUI and the RecorderWriterManager
class ThreadCounter : public QObject
{
    Q_OBJECT
public:
    ThreadCounter() = default;
    ThreadCounter(const ThreadCounter&) = delete;
    ThreadCounter(ThreadCounter&&) = delete;
    ThreadCounter& operator=(const ThreadCounter&) = delete;
    ThreadCounter& operator=(ThreadCounter&&) = delete;

    bool set(int value);
    void setAndNotify(int value);
    unsigned int get() const;

    bool setUsed(int value);
    void setUsedAndNotify(int value);
    void incUsedAndNotify();
    void decUsedAndNotify();
    unsigned int getUsed() const;

Q_SIGNALS:
    void notifyValueChange(bool valueWasIncreased);
    void notifyInUseChange(bool valueWasIncreased);

private:
    bool setUsedImpl(int value);

private:
    unsigned int threads;
    unsigned int inUse;
    QMutex inUseMutex;
};

class RecorderWriter : public QObject
{
    Q_OBJECT
public:
    RecorderWriter(
        unsigned int i,
        QPointer<KisCanvas2> c,
        const RecorderWriterSettings& s,
        const QDir& d);
    ~RecorderWriter();

    RecorderWriter() = delete;
    RecorderWriter(const RecorderWriter&) = delete;
    RecorderWriter(RecorderWriter&&) = delete;
    RecorderWriter& operator=(const RecorderWriter&) = delete;
    RecorderWriter& operator=(RecorderWriter&&) = delete;

    bool isActive();

Q_SIGNALS:
    void capturingDone(int writerId, bool success);

public Q_SLOTS:
    void onCaptureImage(int index, QVector<QRect> regions, KisPaintDeviceSP device);

private:
    bool onCaptureImageImpl(int index, QRect region, KisPaintDevice* device);

private:
    class Private;
    Private *const d;
    unsigned int id;
};

class RecorderWriterManager : public QObject
{
    Q_OBJECT
public:
    explicit RecorderWriterManager(const RecorderExportSettings &es);
    ~RecorderWriterManager();

    RecorderWriterManager() = delete;
    RecorderWriterManager(const RecorderWriterManager&) = delete;
    RecorderWriterManager(RecorderWriterManager&&) = delete;
    RecorderWriterManager& operator=(const RecorderWriterManager&) = delete;
    RecorderWriterManager& operator=(RecorderWriterManager&&) = delete;

    // restarts recorder
    void setCanvas(QPointer<KisCanvas2> canvas);
    // restarts recorder
    void setup(const RecorderWriterSettings &settings);

    void start(bool toggleEnabled, bool restoreState);
    bool stop(bool toggleEnabled, bool saveState);

    void setEnabled(bool enabled);

    ///Generate a tail and save the recording region
    void saveState();
    ///Remove the tail and restore region from a file
    void restoreState();

    // A note about tail frames:
    // They are needed for focused recording. It is possible that the recording is stopped with the last frame being
    // very zoomed in, so if a user tries to export the timelapse in this state, it won't properly dispaly the resulting
    // artwork. One option to fix that is to capture an overview frame whenever a timelapse is epxorted, but the artwork
    // may have changed while recording was off, so we can't do that
    //
    // My solution is a "tail" frame, an overivew captured whenever recording is stopped, and removed if recording is
    // resumed, to avoid adding dead air to timelapses.
    //
    // To avoid big jumps we capture multiple of them with interpolated motion with the last frame, like any other
    // frame.
    //
    // They are technically not needed if the user isn't using focused recording, but they don't hurt to have.

    // Saving the oldRegion varaible, for smooth frame motion, it is very cheap to do as it's literally just 16 bytes,
    // so we're free to do it excessively without worrying. We also use it to keep track of the region with multiple
    // files open, instead of doing that with some global state
    void saveRegion();
    void tryRestoreRegion();

    bool canStartCapture() const;

Q_SIGNALS:
    void started();
    void stopped();
    void frameWriteFailed();
    void lowPerformanceWarning();
    void recorderStopWarning();

private Q_SLOTS:
    void onTimer();
    void onCapturingDone(int workerId, bool success);
    void onImageModified();
    void onToolChanged(const QString &toolId);
    void onToolPrimaryActionActivated(bool activated);

public:
    ThreadCounter recorderThreads{};

private:
    class Private;
    Private *const d;
    const RecorderExportSettings &exporterSettings;

};

#endif // RECORDER_WRITER_H
