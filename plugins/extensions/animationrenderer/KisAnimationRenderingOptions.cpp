#include "KisAnimationRenderingOptions.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

KisAnimationRenderingOptions::KisAnimationRenderingOptions()
    : videoMimeType("video/mp4"),
      frameMimeType("image/png"),
      basename("frame"),
      directory("")
{

}

inline QString composePath(const QString &pathChunk, const QString &fileNameChunk)
{
    if (QFileInfo(fileNameChunk).isAbsolute()) {
        return fileNameChunk;
    }

    return QFileInfo(QDir(QFileInfo(pathChunk).absolutePath()),
                     fileNameChunk).absoluteFilePath();
}

QString KisAnimationRenderingOptions::resolveAbsoluteVideoFilePath() const
{
    return composePath(lastDocuemntPath, videoFileName);
}

QString KisAnimationRenderingOptions::resolveAbsoluteFramesDirectory() const
{
    if (renderMode() == RENDER_VIDEO_ONLY) {
        return QFileInfo(resolveAbsoluteVideoFilePath()).absolutePath();
    }

    return composePath(lastDocuemntPath, directory);
}

KisAnimationRenderingOptions::RenderMode KisAnimationRenderingOptions::renderMode() const
{
    if (shouldDeleteSequence) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(shouldEncodeVideo);
        return RENDER_VIDEO_ONLY;
    } else if (!shouldEncodeVideo) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!shouldDeleteSequence);
        return RENDER_FRAMES_ONLY;
    } else {
        return RENDER_FRAMES_AND_VIDEO;
    }
}

KisPropertiesConfigurationSP KisAnimationRenderingOptions::toProperties() const
{
    KisPropertiesConfigurationSP config = new KisPropertiesConfiguration();

    config->setProperty("basename", basename);
    config->setProperty("last_document_path", lastDocuemntPath);
    config->setProperty("directory", directory);
    config->setProperty("first_frame", firstFrame);
    config->setProperty("last_frame", lastFrame);
    config->setProperty("sequence_start", sequenceStart);
    config->setProperty("video_mimetype", videoMimeType);
    config->setProperty("frame_mimetype", frameMimeType);

    config->setProperty("encode_video", shouldEncodeVideo);
    config->setProperty("delete_sequence", shouldDeleteSequence);

    config->setProperty("ffmpeg_path", ffmpegPath);
    config->setProperty("framerate", frameRate);
    config->setProperty("height", height);
    config->setProperty("width", width);
    config->setProperty("include_audio", includeAudio);
    config->setProperty("filename", videoFileName);
    config->setProperty("custom_ffmpeg_options", customFFMpegOptions);

    config->setPrefixedProperties("frame_export/", frameExportConfig);

    return config;
}

void KisAnimationRenderingOptions::fromProperties(KisPropertiesConfigurationSP config)
{
    basename = config->getPropertyLazy("basename", basename);
    lastDocuemntPath = config->getPropertyLazy("last_document_path", QString());
    directory = config->getPropertyLazy("directory", directory);
    firstFrame = config->getPropertyLazy("first_frame", 0);
    lastFrame = config->getPropertyLazy("last_frame", 0);
    sequenceStart = config->getPropertyLazy("sequence_start", 0);
    videoMimeType = config->getPropertyLazy("video_mimetype", videoMimeType);
    frameMimeType = config->getPropertyLazy("frame_mimetype", frameMimeType);

    shouldEncodeVideo = config->getPropertyLazy("encode_video", false);
    shouldDeleteSequence = config->getPropertyLazy("delete_sequence", false);

    ffmpegPath = config->getPropertyLazy("ffmpeg_path", QString());
    frameRate = config->getPropertyLazy("framerate", 25);
    height = config->getPropertyLazy("height", 0);
    width = config->getPropertyLazy("width", 0);
    includeAudio = config->getPropertyLazy("include_audio", true);
    videoFileName = config->getPropertyLazy("filename", QString());
    customFFMpegOptions = config->getPropertyLazy("custom_ffmpeg_options", QString());

    frameExportConfig = new KisPropertiesConfiguration();
    frameExportConfig->setPrefixedProperties("frame_export/", frameExportConfig);
}
