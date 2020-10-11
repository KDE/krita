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

#ifndef RECORDER_EXPORT_CONFIG_H
#define RECORDER_EXPORT_CONFIG_H

#include <QString>
#include <QList>

class KisConfig;
class QSize;

struct RecorderProfile
{
    QString name;
    QString extension;
    QString arguments;
};

class RecorderExportConfig
{
public:
    RecorderExportConfig(bool readOnly);
    ~RecorderExportConfig();

    int fps() const;
    void setFps(int value);

    bool resize() const;
    void setResize(bool value);

    QSize size() const;
    void setSize(const QSize &value);

    bool lockRatio() const;
    void setLockRatio(bool value);

    int profileIndex() const;
    void setProfileIndex(int value);

    QList<RecorderProfile> profiles() const;
    void setProfiles(const QList<RecorderProfile> &value);

    QList<RecorderProfile> defaultProfiles() const;

    QString ffmpegPath() const;
    void setFfmpegPath(const QString &value);

    QString videoDirectory() const;
    void setVideoDirectory(const QString &value);

private:
    Q_DISABLE_COPY(RecorderExportConfig)
    mutable KisConfig *config;
};

#endif // RECORDER_EXPORT_CONFIG_H
