/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_WRITER_H
#define RECORDER_WRITER_H

#include "recorder_format.h"

#include <QObject>
#include <QPointer>
#include <QMutex>

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

class RecorderWriterManager;

class RecorderWriter : public QObject
{
    Q_OBJECT
public:
    static constexpr int STATUS_OK = 0;
    static constexpr int STATUS_ERROR = 1;
    static constexpr int STATUS_BLOCKED = 2;

    RecorderWriter(
        unsigned int i,
        QPointer<KisCanvas2> c,
        const RecorderWriterSettings& s,
        const QDir& d,
        RecorderWriterManager *m);
    ~RecorderWriter();

    RecorderWriter() = delete;
    RecorderWriter(const RecorderWriter&) = delete;
    RecorderWriter(RecorderWriter&&) = delete;
    RecorderWriter& operator=(const RecorderWriter&) = delete;
    RecorderWriter& operator=(RecorderWriter&&) = delete;

Q_SIGNALS:
    void capturingDone(int writerId, int status);

public Q_SLOTS:
    void onCaptureImage(int writerId);

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

    void start(bool toggleEnabled = true);
    bool stop(bool toggleEnabled = true);

    void setEnabled(bool enabled);

    // This does not do its own synchronization! At the time of writing, the
    // behavior is:
    // * Only the main thread writes to this. It takes the capture mutex.
    // * Reading from the main thread does not take locks.
    // * Reading from writer threads takes the capture mutex.
    bool canStartCapture() const;

    // This does not do its own synchronization, make sure access here is
    // serialized! At the time of writing, this is accomplished by only calling
    // this while the capture mutex is held.
    int incrementAndGetIndex();

    QMutex *captureMutex();

Q_SIGNALS:
    void started();
    void stopped();
    void frameWriteFailed();
    void lowPerformanceWarning();
    void recorderStopWarning();

    void startCapturing(int writerId);

private Q_SLOTS:
    void onTimer();
    void onCapturingDone(int workerId, int status);
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
