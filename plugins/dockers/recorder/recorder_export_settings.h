/*
 *  SPDX-FileCopyrightText: 2023 Florian Reiser <reiserfm@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 *
 *  The sourcecode of this file was moved from recorder_export.cpp
 */

#ifndef KRITA_RECORDER_EXPORT_SETTINGS_H
#define KRITA_RECORDER_EXPORT_SETTINGS_H

#include <QString>
#include <QList>
#include <QSize>
#include "recorder_format.h"

#ifdef Q_OS_ANDROID
#include <QVariantMap>
#else
struct RecorderProfile
{
    QString name;
    QString extension;
    QString arguments;
};
#endif

struct RecorderExportSettings {

    // The following settings are stored in the RecorderExportConfiguration
    bool resize = false;
    bool lockRatio = false;
    bool lockFps = false;
    bool resultPreview = true;
    bool extendResult = true;
    int inputFps = 30;
    int fps = 30;
    int firstFrameSec = 2;
    int lastFrameSec = 5;
    QSize size;
#ifdef Q_OS_ANDROID
    QString selectedFormat;
    QVariantMap formatPreferences;
#else
    int profileIndex = 0;
    QString ffmpegPath;
    QString videoDirectory;
    QString h264Encoder;
    QList<RecorderProfile> profiles;
    QList<RecorderProfile> defaultProfiles;
#endif


    // The following are additional settings, which will not be serialized in
    // the RecorderExportConfiguration
    QString inputDirectory;
    RecorderFormat format;
    QSize imageSize;

#ifdef Q_OS_ANDROID
    QStringList inputFilePaths;
#else
    QString videoFileName;
#endif
    QString videoFilePath;
    int framesCount = 0;

    bool realTimeCaptureMode = false;
    bool realTimeCaptureModeWasSet = false;
};


#endif // KRITA_RECORDER_EXPORT_SETTINGS_H
