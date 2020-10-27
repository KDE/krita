/*
 *  Copyright (c) 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
