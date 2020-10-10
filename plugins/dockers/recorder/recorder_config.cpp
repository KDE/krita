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

#include "recorder_config.h"

#include <kis_config.h>

#include <QString>
#include <QDir>

namespace
{
constexpr const char *keySnapshotDirectory = "recorder/snapshotdirectory";
constexpr const char *keyCaptureInterval = "recorder/captureinterval";
constexpr const char *keyQuality = "recorder/quality";
constexpr const char *keyResolution = "recorder/resolution";
constexpr const char *keyRecordAutomatically = "recorder/recordautomatically";
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
    return config->readEntry(keySnapshotDirectory, QDir::homePath());
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


bool RecorderConfig::recordAutomatically() const
{
    return config->readEntry(keyRecordAutomatically, false);
}

void RecorderConfig::setRecordAutomatically(bool value)
{
    config->writeEntry(keyRecordAutomatically, value);
}
