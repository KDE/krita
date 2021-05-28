/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_export_config.h"

#include <kis_config.h>

#include <QString>
#include <QDir>

namespace
{
const QString keyAnimationExport = "ANIMATION_EXPORT";
const QString keyFfmpegPath = "ffmpeg_path";
const QString keyVideoDirectory = "recorder_export/videodirectory";
const QString keyInputFps = "recorder_export/inputfps";
const QString keyFps = "recorder_export/fps";
const QString keyResize = "recorder_export/resize";
const QString keySize = "recorder_export/size";
const QString keyLockRatio = "recorder_export/lockratio";
const QString keyProfileIndex = "recorder_export/profileIndex";
const QString keyProfiles = "recorder_export/profiles";
const QString keyEditedProfiles = "recorder_export/editedprofiles";

const QString profilePrefix = "-framerate $IN_FPS\n-i \"$INPUT_DIR%07d.$EXT\"\n";

const QList<RecorderProfile> defaultProfiles = {
    { "MP4 x264",   "mp4",  profilePrefix % "-vf \"scale=$WIDTH:$HEIGHT\"\n-c:v libx264\n-r $OUT_FPS\n-pix_fmt yuv420p" },
    { "GIF",        "gif",  profilePrefix % "-vf \"fps=$OUT_FPS,scale=$WIDTH:$HEIGHT:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse\"\n-loop -1" },
    { "Matroska",   "mkv",  profilePrefix % "-vf \"scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "WebM",       "webm", profilePrefix % "-vf \"scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "MP4 x264 (+5s Flash Effect)",  "mp4", profilePrefix % "-filter_complex \"\n"
                                            " [0]scale=$WIDTH:$HEIGHT[q1];\n"
                                            " [q1]tpad=stop_mode=clone:stop_duration=1[v1];\n"
                                            " [0]trim=start_frame=$FRAMES-1[p1];\n"
                                            " [p1]setpts=PTS-STARTPTS[p2];\n"
                                            " [p2]fps=$OUT_FPS[p3];\n"
                                            " [p3]trim=start_frame=0:end_frame=1[p4];\n"
                                            " [p4]scale=$WIDTH:$HEIGHT[p5];\n"
                                            " [p5]tpad=stop_mode=clone:stop_duration=4[p6];\n"
                                            " [p6]fade=type=in:color=white:start_time=0.7:duration=0.7[v2];\n"
                                            " [v1][v2]concat=n=2:v=1\n"
                                            "\"\n-c:v libx264\n-r $OUT_FPS\n-pix_fmt yuv420p" },
    { "Custom1",  "editme", profilePrefix % "-vf \"scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "Custom2",  "editme", profilePrefix % "-vf \"scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "Custom3",  "editme", profilePrefix % "-vf \"scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "Custom4",  "editme", profilePrefix % "-vf \"scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" }
};
}

RecorderExportConfig::RecorderExportConfig(bool readOnly)
    : config(new KisConfig(readOnly))
{
}

RecorderExportConfig::~RecorderExportConfig()
{
    delete config;
}


int RecorderExportConfig::inputFps() const
{
    return config->readEntry(keyInputFps, 30);
}

void RecorderExportConfig::setInputFps(int value)
{
    config->writeEntry(keyInputFps, value);
}


int RecorderExportConfig::fps() const
{
    return config->readEntry(keyFps, 30);
}

void RecorderExportConfig::setFps(int value)
{
    config->writeEntry(keyFps, value);
}


bool RecorderExportConfig::resize() const
{
    return config->readEntry(keyResize, false);
}

void RecorderExportConfig::setResize(bool value)
{
    config->writeEntry(keyResize, value);
}


QSize RecorderExportConfig::size() const
{
    return config->readEntry(keySize, QSize(1024, 1024));
}

void RecorderExportConfig::setSize(const QSize &value)
{
    config->writeEntry(keySize, value);
}


bool RecorderExportConfig::lockRatio() const
{
    return config->readEntry(keyLockRatio, false);
}

void RecorderExportConfig::setLockRatio(bool value)
{
    config->writeEntry(keyLockRatio, value);
}


int RecorderExportConfig::profileIndex() const
{
    return config->readEntry(keyProfileIndex, 0);
}

void RecorderExportConfig::setProfileIndex(int value)
{
    config->writeEntry(keyProfileIndex, value);
}

QList<RecorderProfile> RecorderExportConfig::profiles() const
{
    const QString &profilesStr = config->readEntry(keyProfiles, QString());
    if (profilesStr.isEmpty())
        return ::defaultProfiles;

    const QSet<int> &editedIndexes = editedProfilesIndexes();
    QList<RecorderProfile> profiles;
    const QStringList &profilesList = profilesStr.split("\n");
    int index = 0;
    for (const QString &profileStr : profilesList) {
        // take saved profile in case it's edited.. or something bad happen
        if (editedIndexes.contains(index) || index >= ::defaultProfiles.size()) {
            const QStringList& profileList = profileStr.split("|");
            if (profileList.size() == 3) {
                profiles.append({profileList[0], profileList[1], QString(profileList[2]).replace("\\n", "\n")});
            }
        } else {
            // use default profile to make it possible to upgrade
            profiles.append(::defaultProfiles[index]);
        }

        ++index;
    }
    return profiles;
}

void RecorderExportConfig::setProfiles(const QList<RecorderProfile> &value)
{
    Q_ASSERT(value.size() == ::defaultProfiles.size());

    const QSet<int> &savedEditedProfilesIndexes = editedProfilesIndexes();
    QSet<int> editedProfilesIndexes = savedEditedProfilesIndexes;
    QString outValue;
    const QRegExp cleanUp("[\n|]");
    int index = 0;
    for (const RecorderProfile &profile : value) {
        const RecorderProfile &defaultProfile = ::defaultProfiles[index];
        if (profile != defaultProfile) {
            editedProfilesIndexes.insert(index);
        } else {
            editedProfilesIndexes.remove(index);
        }

        outValue += QString(profile.name).replace(cleanUp, " ") % "|"
                % QString(profile.extension).replace(cleanUp, " ") % "|"
                % QString(profile.arguments).replace("\n", "\\n").replace("|", " ") % "\n";

        ++index;
    }

    config->writeEntry(keyProfiles, outValue);
    if (savedEditedProfilesIndexes != editedProfilesIndexes) {
        setEditedProfilesIndexes(editedProfilesIndexes);
    }
}


QList<RecorderProfile> RecorderExportConfig::defaultProfiles() const
{
    return ::defaultProfiles;
}

QSet<int> RecorderExportConfig::editedProfilesIndexes() const
{
    const QVariantList &readValue = config->readEntry(keyEditedProfiles, QVariantList());
    QSet<int> result;
    for (const QVariant &item : readValue) {
        result.insert(item.toInt());
    }
    return result;
}

void RecorderExportConfig::setEditedProfilesIndexes(const QSet<int> &value)
{
    QVariantList writeValue;
    for (int index : value) {
        writeValue.append(index);
    }
    config->writeEntry(keyEditedProfiles, writeValue);
}


QString RecorderExportConfig::ffmpegPath() const
{
    return KisConfig(true).ffmpegLocation();
}

void RecorderExportConfig::setFfmpegPath(const QString &value)
{
    KisConfig cfg(false);
    cfg.setFFMpegLocation(value);
}


QString RecorderExportConfig::videoDirectory() const
{
    return config->readEntry(keyVideoDirectory, QDir::homePath());
}

void RecorderExportConfig::setVideoDirectory(const QString &value)
{
    config->writeEntry(keyVideoDirectory, value);
}

bool operator==(const RecorderProfile &left, const RecorderProfile &right)
{
    return left.arguments == right.arguments
        && left.name == right.name
        && left.extension == right.extension;
}

bool operator!=(const RecorderProfile &left, const RecorderProfile &right)
{
    return !(left == right);
}
