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
