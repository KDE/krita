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

    double captureInterval() const;
    void setCaptureInterval(double value);

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

    int threads() const;
    void setThreads(int value);

    bool realTimeCaptureMode() const;
    void setRealTimeCaptureMode(bool value);

    bool recordIsolateLayerMode() const;
    void setRecordIsolateLayerMode(bool value);

    bool recordAutomatically() const;
    void setRecordAutomatically(bool value);

#ifdef Q_OS_ANDROID
    // Returns the internal storage directory that older versions of Krita
    // defaulted to. Using this directory to store recordings is bogus, since
    // it's effectively inacessible to the user. This path is used to rectify
    // the situation and move stuff out of this directory when the user has
    // given permission to a "real" one.
    static const QString &defaultInternalSnapshotDirectory();
#endif

private:
    Q_DISABLE_COPY(RecorderConfig)
    mutable KisConfig *config;
};

#endif // RECORDER_CONFIG_H
