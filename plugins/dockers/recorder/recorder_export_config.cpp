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
const QString keyResultPreview="recorder_export/resultpreview";
const QString keyFirstFrameSec = "recorder_export/firstframesec";
const QString keyExtendResult = "recorder_export/extendresult";
const QString keyLastFrameSec = "recorder_export/lastframesec";
const QString keyResize = "recorder_export/resize";
const QString keySize = "recorder_export/size";
const QString keyLockRatio = "recorder_export/lockratio";
const QString keyProfileIndex = "recorder_export/profileIndex";
const QString keyProfiles = "recorder_export/profiles";
const QString keyEditedProfiles = "recorder_export/editedprofiles";

const QString profilePrefix = "-framerate $IN_FPS\n-i \"$INPUT_DIR%07d.$EXT\"\n-framerate $IN_FPS\n-start_number $FRAMES-1\n-i \"$INPUT_DIR%07d.$EXT\"\n";

const QList<RecorderProfile> defaultProfiles = {
    { "MP4 x264",   "mp4",  profilePrefix % "-filter_complex \"\n"
      /* Filter documentation https://trac.ffmpeg.org/wiki/FilteringGuide
       * Inside the complex filter command each line is a different filter that is applied to an input stream.
       * The input stream to transform is specified at the start of each line in the format '[$STREAM_NAME]' (ex. [p1], [v1], [v2]).
       * Immediatly following the input stream name will be the filter to apply (ex: trim, scale, loop)
       * Depending on the filter it may or may have various options set
       * After all options for the filter a name can be specified that will correspond to the output stream of the filter. If no name is specified it will be used as the final result for the output file.
       */
                                            " [0]loop=$LAST_FRAME_SEC*$OUT_FPS:size=1:start=$FRAMES[main1];\n"
                                            " [main1]scale=$WIDTH:$HEIGHT[main2];\n"
                                            " [main2]loop=1:size=1:start=0[main3];\n"
                                            " [main3]setpts=PTS-STARTPTS[main4];\n"

                                            " [1]split [first1][transition1];\n"
                                            " [transition1]scale=$WIDTH:$HEIGHT [transition2];\n"
                                            " [transition2]loop='if(gte($FIRST_FRAME_SEC, 1), 1*$OUT_FPS, 0)':size=1:start=1[transition3];\n"
                                            " [transition3]setpts=PTS-STARTPTS[transition4];\n"

                                            " [transition4][main4]xfade=transition=smoothright:duration=0.5:offset=0[v1];\n"
                                            " [v1]setpts=PTS-STARTPTS[v2];\n"
                                            " [v2]trim=start_frame=1[v3];\n"

                                            " [first1]loop='if(gte($FIRST_FRAME_SEC, 1), ($FIRST_FRAME_SEC*$OUT_FPS) - 0.5, $FIRST_FRAME_SEC*$OUT_FPS)':size=1:start=1[preview1];\n"
                                            " [preview1]scale=$WIDTH:$HEIGHT[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"
                                            " [preview3][v3] concat [final1];\n"
                                            " [final1] setpts=PTS-STARTPTS[final2];\n"
                                            " [final2] trim=start_frame=1\n"

                                            "\"\n-c:v libx264\n-r $OUT_FPS\n-pix_fmt yuv420p" },
    { "GIF",        "gif",  profilePrefix % "-filter_complex \"\n"
                                            " [0]loop=$LAST_FRAME_SEC*$OUT_FPS:size=1:start=$FRAMES[main1];\n"
                                            " [main1]scale=$WIDTH:$HEIGHT[main2];\n"
                                            " [main2]loop=1:size=1:start=0[main3];\n"
                                            " [main3]setpts=PTS-STARTPTS[main4];\n"
                                            " [1]split [first1][transition1];\n"
                                            " [transition1]scale=$WIDTH:$HEIGHT [transition2];\n"
                                            " [transition2]loop='if(gte($FIRST_FRAME_SEC, 1), 1*$OUT_FPS, 0)':size=1:start=1[transition3];\n"
                                            " [transition3]setpts=PTS-STARTPTS[transition4];\n"
                                            " [transition4][main4]xfade=transition=smoothright:duration=0.5:offset=0[v1];\n"
                                            " [v1]setpts=PTS-STARTPTS[v2];\n"
                                            " [v2]trim=start_frame=1[v3];\n"
                                            " [first1]loop='if(gte($FIRST_FRAME_SEC, 1), ($FIRST_FRAME_SEC*$OUT_FPS) - 0.5, $FIRST_FRAME_SEC*$OUT_FPS)':size=1:start=1[preview1];\n"
                                            " [preview1]scale=$WIDTH:$HEIGHT[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"
                                            " [preview3][v3] concat [final1];\n"
                                            " [final1] setpts=PTS-STARTPTS[final2];\n"
                                            " [final2] trim=start_frame=1[final3];\n"
                                            " [final3]split[final4][final5];\n"
                                            " [final4]palettegen[palettegen];\n"
                                            " [final5][palettegen]paletteuse\"\n-loop -1" },
    { "Matroska",   "mkv",  profilePrefix % "-filter_complex \"\n"
                                            " [0]loop=$LAST_FRAME_SEC*$OUT_FPS:size=1:start=$FRAMES[main1];\n"
                                            " [main1]scale=$WIDTH:$HEIGHT[main2];\n"
                                            " [main2]loop=1:size=1:start=0[main3];\n"
                                            " [main3]setpts=PTS-STARTPTS[main4];\n"

                                            " [1]split [first1][transition1];\n"
                                            " [transition1]scale=$WIDTH:$HEIGHT [transition2];\n"
                                            " [transition2]loop='if(gte($FIRST_FRAME_SEC, 1), 1*$OUT_FPS, 0)':size=1:start=1[transition3];\n"
                                            " [transition3]setpts=PTS-STARTPTS[transition4];\n"

                                            " [transition4][main4]xfade=transition=smoothright:duration=0.5:offset=0[v1];\n"
                                            " [v1]setpts=PTS-STARTPTS[v2];\n"
                                            " [v2]trim=start_frame=1[v3];\n"

                                            " [first1]loop='if(gte($FIRST_FRAME_SEC, 1), ($FIRST_FRAME_SEC*$OUT_FPS) - 0.5, $FIRST_FRAME_SEC*$OUT_FPS)':size=1:start=1[preview1];\n"
                                            " [preview1]scale=$WIDTH:$HEIGHT[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"
                                            " [preview3][v3] concat [final1];\n"
                                            " [final1] setpts=PTS-STARTPTS[final2];\n"
                                            " [final2] trim=start_frame=1\n"
                                            "\"\n-r $OUT_FPS" },
    { "WebM",       "webm", profilePrefix % "-filter_complex \"\n"
                                            " [0]loop=$LAST_FRAME_SEC*$OUT_FPS:size=1:start=$FRAMES[main1];\n"
                                            " [main1]scale=$WIDTH:$HEIGHT[main2];\n"
                                            " [main2]loop=1:size=1:start=0[main3];\n"
                                            " [main3]setpts=PTS-STARTPTS[main4];\n"

                                            " [1]split [first1][transition1];\n"
                                            " [transition1]scale=$WIDTH:$HEIGHT [transition2];\n"
                                            " [transition2]loop='if(gte($FIRST_FRAME_SEC, 1), 1*$OUT_FPS, 0)':size=1:start=1[transition3];\n"
                                            " [transition3]setpts=PTS-STARTPTS[transition4];\n"

                                            " [transition4][main4]xfade=transition=smoothright:duration=0.5:offset=0[v1];\n"
                                            " [v1]setpts=PTS-STARTPTS[v2];\n"
                                            " [v2]trim=start_frame=1[v3];\n"

                                            " [first1]loop='if(gte($FIRST_FRAME_SEC, 1), ($FIRST_FRAME_SEC*$OUT_FPS) - 0.5, $FIRST_FRAME_SEC*$OUT_FPS)':size=1:start=1[preview1];\n"
                                            " [preview1]scale=$WIDTH:$HEIGHT[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"
                                            " [preview3][v3] concat [final1];\n"
                                            " [final1] setpts=PTS-STARTPTS[final2];\n"
                                            " [final2] trim=start_frame=1\n"
                                            "\"\n-r $OUT_FPS" },
    { "MP4 x264 (Flash Effect)",  "mp4", profilePrefix % "-filter_complex \"\n"
                                            " [1]loop=$LAST_FRAME_SEC*$OUT_FPS:size=1:start=0[fade1];\n"
                                            " [fade1]fps=$OUT_FPS[fade2];\n"
                                            " [fade2]fade=type=in:color=white:start_time=0.7:duration=0.7[fade3];\n"
                                            " [fade3]setsar=1[fade4];\n"
                                            " [0]setsar=1[main0];\n"
                                            " [main0][fade4] concat=n=2:v=1[main1];\n"
                                            " [main1]scale=$WIDTH:$HEIGHT[main2];\n"
                                            " [main2]loop=1:size=1:start=0[main3];\n"
                                            " [main3]setpts=PTS-STARTPTS[main4];\n"
                                            " [main4]fps=fps=$OUT_FPS[main5];\n"
                                            " [1]split [first1][transition1];\n"
                                            " [transition1]scale=$WIDTH:$HEIGHT [transition2];\n"
                                            " [transition2]loop='if(gte($FIRST_FRAME_SEC, 1), 1*$OUT_FPS, 0)':size=1:start=1[transition3];\n"
                                            " [transition3]setpts=PTS-STARTPTS[transition4];\n"
                                            " [transition4][main5]xfade=transition=smoothright:duration=0.5:offset=0[v1];\n"
                                            " [v1]setpts=PTS-STARTPTS[v2];\n"
                                            " [v2]trim=start_frame=1[v3];\n"
                                            " [first1]loop='if(gte($FIRST_FRAME_SEC, 1), ($FIRST_FRAME_SEC*$OUT_FPS) - 0.5, $FIRST_FRAME_SEC*$OUT_FPS)':size=1:start=1[preview1];\n"
                                            " [preview1]scale=$WIDTH:$HEIGHT[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"
                                            " [preview3][v3] concat [final1];\n"
                                            " [final1] setpts=PTS-STARTPTS[final2];\n"
                                            " [final2] trim=start_frame=1\n"
                                            "\"\n-c:v libx264\n-r $OUT_FPS\n-pix_fmt yuv420p" },
    { "Custom1",  "editme", profilePrefix % "-filter_complex \"loop=$LAST_FRAME_SEC:size=1:start=$FRAMES,scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "Custom2",  "editme", profilePrefix % "-filter_complex \"loop=$LAST_FRAME_SEC:size=1:start=$FRAMES,scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "Custom3",  "editme", profilePrefix % "-filter_complex \"loop=$LAST_FRAME_SEC:size=1:start=$FRAMES,scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" },
    { "Custom4",  "editme", profilePrefix % "-filter_complex \"loop=$LAST_FRAME_SEC:size=1:start=$FRAMES,scale=$WIDTH:$HEIGHT\"\n-r $OUT_FPS" }
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

bool RecorderExportConfig::resultPreview() const
{
    return config->readEntry(keyResultPreview, true);
}

void RecorderExportConfig::setResultPreview(bool value)
{
    config->writeEntry(keyResultPreview, value);
}

int RecorderExportConfig::firstFrameSec() const
{
    return config->readEntry(keyFirstFrameSec, 2);
}

void RecorderExportConfig::setFirstFrameSec(int value)
{
    config->writeEntry(keyFirstFrameSec, value);
}


bool RecorderExportConfig::extendResult() const
{
    return config->readEntry(keyExtendResult, true);
}

void RecorderExportConfig::setExtendResult(bool value)
{
    config->writeEntry(keyExtendResult, value);
}

int RecorderExportConfig::lastFrameSec() const
{
    return config->readEntry(keyLastFrameSec, 5);
}

void RecorderExportConfig::setLastFrameSec(int value)
{
    config->writeEntry(keyLastFrameSec, value);
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
