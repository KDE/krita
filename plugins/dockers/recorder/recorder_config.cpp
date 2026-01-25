/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_config.h"

#include <kis_config.h>

#include <QString>
#include <QDir>

namespace
{
const QString keySnapshotDirectory = "recorder/snapshotdirectory";
const QString keyCaptureInterval = "recorder/captureinterval";
const QString keyFormat = "recorder/format";
const QString keyQuality = "recorder/quality";
const QString keyCompression = "recorder/compression";
const QString keyResolution = "recorder/resolution";
const QString keyThreads = "recorder/threads";
const QString keyRealTimeCaptureMode = "recorder/realtimecapturemode";
const QString keyRecordIsolateLayerMode = "recorder/recordisolatelayermode";
const QString keyRecordAutomatically = "recorder/recordautomatically";
const QString defaultSnapshotDirectory = QDir::homePath() % QDir::separator() % "KritaRecorder";
}

RecorderConfig::RecorderConfig(bool readOnly)
    : config(new KisConfig(readOnly))
{
}

RecorderConfig::~RecorderConfig()
{
    delete config;
}


QString RecorderConfig::snapshotDirectory() const
{
    // On Android, there's no user-visible directory we can write to, so we
    // default to nothing here.
#ifdef Q_OS_ANDROID
    const QString defaultValue;
#else
    const QString &defaultValue = defaultSnapshotDirectory;
#endif
    return config->readEntry(keySnapshotDirectory, defaultValue);
}

void RecorderConfig::setSnapshotDirectory(const QString &value)
{
    config->writeEntry(keySnapshotDirectory, value);
}


double RecorderConfig::captureInterval() const
{
    return config->readEntry(keyCaptureInterval, 1.);
}

void RecorderConfig::setCaptureInterval(double value)
{
    config->writeEntry(keyCaptureInterval, value);
}


RecorderFormat RecorderConfig::format() const
{
    return static_cast<RecorderFormat>(config->readEntry(keyFormat, static_cast<int>(RecorderFormat::JPEG)));
}

void RecorderConfig::setFormat(RecorderFormat value)
{
    return config->writeEntry(keyFormat, static_cast<int>(value));
}


int RecorderConfig::quality() const
{
    return config->readEntry(keyQuality, 80);
}

void RecorderConfig::setQuality(int value)
{
    config->writeEntry(keyQuality, value);
}


int RecorderConfig::compression() const
{
    return config->readEntry(keyCompression, 1);
}

void RecorderConfig::setCompression(int value)
{
    config->writeEntry(keyCompression, value);
}


int RecorderConfig::resolution() const
{
    return config->readEntry(keyResolution, 0);
}

void RecorderConfig::setResolution(int value)
{
    config->writeEntry(keyResolution, value);
}

int RecorderConfig::threads() const
{
    return config->readEntry(keyThreads, 1);
}

void RecorderConfig::setThreads(int value)
{
    config->writeEntry(keyThreads, value);
}

bool RecorderConfig::realTimeCaptureMode() const
{
    return config->readEntry(keyRealTimeCaptureMode, false);
}
void RecorderConfig::setRealTimeCaptureMode(bool value)
{
    config->writeEntry(keyRealTimeCaptureMode, value);
}

bool RecorderConfig::recordIsolateLayerMode() const
{
    return config->readEntry(keyRecordIsolateLayerMode, false);
}

void RecorderConfig::setRecordIsolateLayerMode(bool value)
{
    config->writeEntry(keyRecordIsolateLayerMode, value);
}

bool RecorderConfig::recordAutomatically() const
{
    return config->readEntry(keyRecordAutomatically, false);
}

void RecorderConfig::setRecordAutomatically(bool value)
{
    config->writeEntry(keyRecordAutomatically, value);
}

#ifdef Q_OS_ANDROID
const QString &RecorderConfig::defaultInternalSnapshotDirectory()
{
    return defaultSnapshotDirectory;
}
#endif
