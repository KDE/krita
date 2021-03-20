/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_CONFIG_H
#define RECORDER_CONFIG_H

#include <QtGlobal>

class KisConfig;

class RecorderConfig
{
public:
    RecorderConfig(bool readOnly);
    ~RecorderConfig();

    QString snapshotDirectory() const;
    void setSnapshotDirectory(const QString &value);

    int captureInterval() const;
    void setCaptureInterval(int value);

    int quality() const;
    void setQuality(int value);

    int resolution() const;
    void setResolution(int value);

    bool recordIsolateLayerMode() const;
    void setRecordIsolateLayerMode(bool value);

    bool recordAutomatically() const;
    void setRecordAutomatically(bool value);

private:
    Q_DISABLE_COPY(RecorderConfig)
    mutable KisConfig *config;
};

#endif // RECORDER_CONFIG_H
