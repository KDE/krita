/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_WRITER_H
#define RECORDER_WRITER_H

#include <QThread>
#include <QPointer>

class RecorderConfig;
class KisCanvas2;

struct RecorderWriterSettings
{
    QString outputDirectory;
    int quality;
    int resolution;
    int captureInterval;
    bool recordIsolateLayerMode;
};

class RecorderWriter: public QThread
{
    Q_OBJECT
public:
    RecorderWriter();
    ~RecorderWriter();

    void setCanvas(QPointer<KisCanvas2> canvas);
    void setup(const RecorderWriterSettings &settings);

    bool stop();

    void setEnabled(bool enabled);

Q_SIGNALS:
    void pausedChanged(bool paused);
    void prefixChanged(QString prefix);

protected:
    void run() override;
    void timerEvent(QTimerEvent *event) override;

private Q_SLOTS:
    void onImageModified();
    void onToolChanged(const QString &toolId);

private:
    Q_DISABLE_COPY(RecorderWriter)
    class Private;
    Private *const d;
};

#endif // RECORDER_WRITER_H
