/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_export_config.h"

#include <kis_config.h>

#include <QString>
#include <QDir>
#include <QRegularExpression>

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
const QString keyLockFps = "recorder_export/lockfps";
const QString keyProfileIndex = "recorder_export/profileIndex";
const QString keyProfiles = "recorder_export/profiles";
const QString keyEditedProfiles = "recorder_export/editedprofiles";

const QString profilePrefix = "-reinit_filter 0\n-framerate $IN_FPS\n-i \"$INPUT_DIR%07d.$EXT\"\n-framerate $OUT_FPS\n-start_number $FRAMES-1\n-i \"$INPUT_DIR%07d.$EXT\"\n";

const QList<RecorderProfile> defaultProfiles = {
    { "MP4 x264",   "mp4",  profilePrefix % "-filter_complex \"\n"
      /* Filter documentation https://trac.ffmpeg.org/wiki/FilteringGuide
       * Inside the complex filter command each line is a different filter that is applied to an input stream.
       * The input stream to transform is specified at the start of each line in the format '[$STREAM_NAME]' (ex. [p1], [v1], [v2]).
       * Immediately following the input stream name will be the filter to apply (ex: trim, scale, loop)
       * Depending on the filter it may or may have various options set
       * After all options for the filter a name can be specified that will correspond to the output stream of the filter. If no name is specified it will be used as the final result for the output file.
       */
                                            " [1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))'[hold1];\n"
                                            " [hold1]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2[hold2];\n"
                                            " [hold2]setsar=1:1[hold3];\n"
                                            " [hold3]split=3[preview1][transition1][end1];\n"

                                            " [preview1]tpad=stop_mode=clone:stop_duration=$FIRST_FRAME_SEC[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"

                                            " [transition1]tpad=stop_mode=clone:stop_duration=$TRANSITION_LENGTH[transition2];\n"
                                            " [transition2]setpts=PTS-STARTPTS[transition3];\n"
                                            " [transition3]framerate=$OUT_FPS[transition4];\n"

                                            " [0]fps=$OUT_FPS[main1];\n"
                                            " [main1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))':eval=frame[main2];\n"
                                            " [main2]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2:eval=frame[main3];\n"
                                            " [main3]setsar=1:1[main4];\n"
                                            " [transition4][main4]xfade=transition=smoothright:duration=$TRANSITION_LENGTH:offset=0[main5];\n"

                                            " [end1]tpad=stop_mode=clone:stop_duration=$LAST_FRAME_SEC[end2];\n"
                                            " [end2]setpts=PTS-STARTPTS[end3];\n"

                                            " [preview3][main5][end3]concat=n=3:v=1:a=0[final1];\n"
                                            " [final1]setpts=PTS-STARTPTS\n"

                                            "\"\n-stats\n-loglevel error\n-c:v $H264_ENCODER\n-r $OUT_FPS\n-pix_fmt yuv420p" },
    { "GIF",        "gif",  profilePrefix % "-filter_complex \"\n"
                                            " [1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))'[hold1];\n"
                                            " [hold1]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2[hold2];\n"
                                            " [hold2]setsar=1:1[hold3];\n"
                                            " [hold3]split=3[preview1][transition1][end1];\n"

                                            " [preview1]tpad=stop_mode=clone:stop_duration=$FIRST_FRAME_SEC[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"

                                            " [transition1]tpad=stop_mode=clone:stop_duration=$TRANSITION_LENGTH[transition2];\n"
                                            " [transition2]setpts=PTS-STARTPTS[transition3];\n"
                                            " [transition3]framerate=$OUT_FPS[transition4];\n"

                                            " [0]fps=$OUT_FPS[main1];\n"
                                            " [main1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))':eval=frame[main2];\n"
                                            " [main2]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2:eval=frame[main3];\n"
                                            " [main3]setsar=1:1[main4];\n"
                                            " [transition4][main4]xfade=transition=smoothright:duration=$TRANSITION_LENGTH:offset=0[main5];\n"

                                            " [end1]tpad=stop_mode=clone:stop_duration=$LAST_FRAME_SEC[end2];\n"
                                            " [end2]setpts=PTS-STARTPTS[end3];\n"

                                            " [preview3][main5][end3]concat=n=3:v=1:a=0[final1];\n"
                                            " [final1]setpts=PTS-STARTPTS[final2];\n"
                                            " [final2]split[final3][final4];\n"
                                            " [final3]palettegen[palettegen];\n"
                                            " [final4][palettegen]paletteuse\n"
                                            "\"\n-stats\n-loglevel error\n-r $OUT_FPS" },
    { "Matroska",   "mkv",  profilePrefix % "-filter_complex \"\n"
                                            " [1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))'[hold1];\n"
                                            " [hold1]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2[hold2];\n"
                                            " [hold2]setsar=1:1[hold3];\n"
                                            " [hold3]split=3[preview1][transition1][end1];\n"

                                            " [preview1]tpad=stop_mode=clone:stop_duration=$FIRST_FRAME_SEC[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"

                                            " [transition1]tpad=stop_mode=clone:stop_duration=$TRANSITION_LENGTH[transition2];\n"
                                            " [transition2]setpts=PTS-STARTPTS[transition3];\n"
                                            " [transition3]framerate=$OUT_FPS[transition4];\n"

                                            " [0]fps=$OUT_FPS[main1];\n"
                                            " [main1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))':eval=frame[main2];\n"
                                            " [main2]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2:eval=frame[main3];\n"
                                            " [main3]setsar=1:1[main4];\n"
                                            " [transition4][main4]xfade=transition=smoothright:duration=$TRANSITION_LENGTH:offset=0[main5];\n"

                                            " [end1]tpad=stop_mode=clone:stop_duration=$LAST_FRAME_SEC[end2];\n"
                                            " [end2]setpts=PTS-STARTPTS[end3];\n"

                                            " [preview3][main5][end3]concat=n=3:v=1:a=0[final1];\n"
                                            " [final1]setpts=PTS-STARTPTS\n"
                                            "\"\n-stats\n-loglevel error\n-pix_fmt yuv420p\n-r $OUT_FPS" },
    { "WebM",       "webm", profilePrefix % "-filter_complex \"\n"
                                            " [1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))'[hold1];\n"
                                            " [hold1]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2[hold2];\n"
                                            " [hold2]setsar=1:1[hold3];\n"
                                            " [hold3]split=3[preview1][transition1][end1];\n"

                                            " [preview1]tpad=stop_mode=clone:stop_duration=$FIRST_FRAME_SEC[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"

                                            " [transition1]tpad=stop_mode=clone:stop_duration=$TRANSITION_LENGTH[transition2];\n"
                                            " [transition2]setpts=PTS-STARTPTS[transition3];\n"
                                            " [transition3]framerate=$OUT_FPS[transition4];\n"

                                            " [0]fps=$OUT_FPS[main1];\n"
                                            " [main1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))':eval=frame[main2];\n"
                                            " [main2]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2:eval=frame[main3];\n"
                                            " [main3]setsar=1:1[main4];\n"
                                            " [transition4][main4]xfade=transition=smoothright:duration=$TRANSITION_LENGTH:offset=0[main5];\n"

                                            " [end1]tpad=stop_mode=clone:stop_duration=$LAST_FRAME_SEC[end2];\n"
                                            " [end2]setpts=PTS-STARTPTS[end3];\n"

                                            " [preview3][main5][end3]concat=n=3:v=1:a=0[final1];\n"
                                            " [final1]setpts=PTS-STARTPTS\n"
                                            "\"\n-stats\n-loglevel error\n-r $OUT_FPS" },
    { "MP4 x264 (Flash Effect)",  "mp4", profilePrefix % "-filter_complex \"\n"
                                            " [1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))'[hold1];\n"
                                            " [hold1]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2[hold2];\n"
                                            " [hold2]setsar=1:1[hold3];\n"
                                            " [hold3]split=3[preview1][fade1][end1];\n"

                                            " [preview1]tpad=stop_mode=clone:stop_duration=$FIRST_FRAME_SEC[preview2];\n"
                                            " [preview2]setpts=PTS-STARTPTS[preview3];\n"

                                            " [fade1]tpad=stop_mode=clone:stop_duration=$TRANSITION_LENGTH[fade2];\n"
                                            " [fade2]setpts=PTS-STARTPTS[fade3];\n"
                                            " [fade3]fade=out:duration=$TRANSITION_LENGTH:color=white[fade4];\n"

                                            " [0]fps=$OUT_FPS[main1];\n"
                                            " [main1]scale='min($WIDTH, iw*($HEIGHT/ih))':'min($HEIGHT, ih*($WIDTH/iw))':eval=frame[main2];\n"
                                            " [main2]pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2:eval=frame[main3];\n"
                                            " [main3]setsar=1:1[main4];\n"
                                            " [fade4][main4]concat[main5];\n"

                                            " [end1]tpad=stop_mode=clone:stop_duration=$LAST_FRAME_SEC[end2];\n"
                                            " [end2]setpts=PTS-STARTPTS[end3];\n"

                                            " [preview3][main5][end3]concat=n=3:v=1:a=0[final1];\n"
                                            " [final1]setpts=PTS-STARTPTS\n"
                                            "\"\n-stats\n-loglevel error\n-c:v $H264_ENCODER\n-r $OUT_FPS\n-pix_fmt yuv420p" },
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

void RecorderExportConfig::loadConfiguration(RecorderExportSettings *settings, bool loadLockFps) const
{
    settings->inputFps = inputFps();
    settings->fps = fps();
    settings->resultPreview = resultPreview();
    settings->firstFrameSec = firstFrameSec();
    settings->extendResult = extendResult();
    settings->lastFrameSec = lastFrameSec();
    settings->resize = resize();
    settings->size = size();
    settings->lockRatio = lockRatio();
    settings->ffmpegPath = ffmpegPath();
    settings->profiles = profiles();
    settings->defaultProfiles = defaultProfiles();
    settings->profileIndex = profileIndex();
    settings->videoDirectory = videoDirectory();
    if (loadLockFps)
        settings->lockFps = lockFps();
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

bool RecorderExportConfig::lockFps() const
{
    return config->readEntry(keyLockFps, false);
}

void RecorderExportConfig::setLockFps(bool value)
{
    config->writeEntry(keyLockFps, value);
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
    const QRegularExpression cleanUp("[\n|]");
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
