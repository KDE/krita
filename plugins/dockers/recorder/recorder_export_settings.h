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

struct RecorderProfile
{
    QString name;
    QString extension;
    QString arguments;
};

struct RecorderExportSettings {

    // The following settings are stored in the RecorderExportConfiguration
    bool resize = false;
    bool lockRatio = false;
    bool lockFps = false;
    bool resultPreview = true;
    bool extendResult = true;
    int inputFps = 30;
    int fps = 30;
    int profileIndex = 0;
    int firstFrameSec = 2;
    int lastFrameSec = 5;
    QSize size;
    QString ffmpegPath;
    QString videoDirectory;
    QList<RecorderProfile> profiles;
    QList<RecorderProfile> defaultProfiles;


    // The following are additional settings, which will not be serialized in
    // the RecorderExportConfiguration
    QString inputDirectory;
    RecorderFormat format;
    QSize imageSize;

    QString videoFileName;
    QString videoFilePath;
    int framesCount = 0;

    bool realTimeCaptureMode = false;
    bool realTimeCaptureModeWasSet = false;
};


#endif // KRITA_RECORDER_EXPORT_SETTINGS_H
