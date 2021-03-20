/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
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

    int inputFps() const;
    void setInputFps(int value);

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

    QSet<int> editedProfilesIndexes() const;
    void setEditedProfilesIndexes(const QSet<int> &value);

    QString ffmpegPath() const;
    void setFfmpegPath(const QString &value);

    QString videoDirectory() const;
    void setVideoDirectory(const QString &value);

private:
    Q_DISABLE_COPY(RecorderExportConfig)
    mutable KisConfig *config;
};

bool operator==(const RecorderProfile &left, const RecorderProfile &right);
bool operator!=(const RecorderProfile &left, const RecorderProfile &right);

#endif // RECORDER_EXPORT_CONFIG_H
