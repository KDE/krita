/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_CONFIG_H
#define RECORDER_CONFIG_H

#include "recorder_format.h"

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

    RecorderFormat format() const;
    void setFormat(RecorderFormat value);

    // for JPEG (0...100)
    int quality() const;
    void setQuality(int value);

    // for PNG (0...10)
    int compression() const;
    void setCompression(int value);

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
