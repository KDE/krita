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
const QString keyQuality = "recorder/quality";
const QString keyResolution = "recorder/resolution";
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
    return config->readEntry(keySnapshotDirectory, defaultSnapshotDirectory);
}

void RecorderConfig::setSnapshotDirectory(const QString &value)
{
    config->writeEntry(keySnapshotDirectory, value);
}


int RecorderConfig::captureInterval() const
{
    return config->readEntry(keyCaptureInterval, 1);
}

void RecorderConfig::setCaptureInterval(int value)
{
    config->writeEntry(keyCaptureInterval, value);
}


int RecorderConfig::quality() const
{
    return config->readEntry(keyQuality, 80);
}

void RecorderConfig::setQuality(int value)
{
    config->writeEntry(keyQuality, value);
}


int RecorderConfig::resolution() const
{
    return config->readEntry(keyResolution, 0);
}

void RecorderConfig::setResolution(int value)
{
    config->writeEntry(keyResolution, value);
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
