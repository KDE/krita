/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_WRITER_H
#define RECORDER_WRITER_H

#include "recorder_format.h"

#include <QThread>
#include <QPointer>
#include <QMutex>

struct RecorderExportSettings;
class RecorderConfig;
class KisCanvas2;

// This class is used to provide a thread safe mechanism for syncing the
// number of used recording threads between the GUI and the
// RecorderWriterManager
class ThreadCounter : public QObject
{
    Q_OBJECT
public:
    ThreadCounter() = default;
    ThreadCounter(const ThreadCounter&) = delete;
    ThreadCounter(ThreadCounter&&) = delete;
    ThreadCounter& operator=(const ThreadCounter&) = delete;
    ThreadCounter& operator=(ThreadCounter&&) = delete;

    //static ThreadCounter& instance();

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


struct RecorderWriterSettings
{
    QString outputDirectory;
    RecorderFormat format;
    int quality;
    int compression;
    int resolution;
    double captureInterval;
    bool recordIsolateLayerMode;
    bool realTimeCaptureMode;
};

class RecorderWriter: public QThread
{
    Q_OBJECT
public:
    RecorderWriter(const RecorderExportSettings &es);
    ~RecorderWriter();

    void setCanvas(QPointer<KisCanvas2> canvas);
    void setup(const RecorderWriterSettings &settings);

    bool stop();

    void setEnabled(bool enabled);

Q_SIGNALS:
    void pausedChanged(bool paused);
    void prefixChanged(QString prefix);
    void frameWriteFailed();
    void lowPerformanceWarning();

protected:
    void run() override;
    void timerEvent(QTimerEvent *event) override;

private Q_SLOTS:
    void onImageModified();
    void onToolChanged(const QString &toolId);

public:
    ThreadCounter recorderThreads{};

private:
    Q_DISABLE_COPY(RecorderWriter)
    class Private;
    Private *const d;
    const RecorderExportSettings &exporterSettings;
};

#endif // RECORDER_WRITER_H
