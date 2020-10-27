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
