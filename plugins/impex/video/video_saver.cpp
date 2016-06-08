/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "video_saver.h"

#include <QDebug>
// #include <QApplication>

#include <QFileInfo>
// #include <QFile>
// #include <QDir>
// #include <QVector>
// #include <QIODevice>
// #include <QRect>
// #include <KisMimeDatabase.h>

// #include <KisPart.h>
#include <KisDocument.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_range.h>

#include "kis_animation_exporter.h"

#include <AvTranscoder/transcoder/Transcoder.hpp>
#include <AvTranscoder/file/OutputFile.hpp>
#include <AvTranscoder/progress/ConsoleProgress.hpp>
#include <AvTranscoder/decoder/VideoGenerator.hpp>
#include <AvTranscoder/data/decoded/VideoFrame.hpp>


VideoSaver::VideoSaver(KisDocument *doc, bool batchMode)
    : m_image(doc->image())
    , m_doc(doc)
    , m_batchMode(batchMode)
    , m_stop(false)
{
}

VideoSaver::~VideoSaver()
{
}

KisImageWSP VideoSaver::image()
{
    return m_image;
}

struct FrameUploader {
    FrameUploader(avtranscoder::StreamTranscoder &_transcoder,
                const avtranscoder::VideoFrameDesc& frameDesc,
                const QRect &_bounds,
                const KoColorSpace *_dstCS)
        : transcoder(_transcoder),
          frameData(frameDesc),
          dstCS(_dstCS),
          bounds(_bounds)
    {
        generator =
            dynamic_cast<avtranscoder::VideoGenerator*>(transcoder.getCurrentDecoder());
        KIS_ASSERT_RECOVER_NOOP(generator);

        buffer.resize(dstCS->pixelSize() * frameDesc._width * frameDesc._height);
    }

    KisImportExportFilter::ConversionStatus
    operator() (int time, KisPaintDeviceSP dev) {
        Q_UNUSED(time);

        dev->convertTo(dstCS);
        dev->readBytes(buffer.data(), bounds);
        frameData.assign(buffer.constData());
        generator->setNextFrame(frameData);
        transcoder.processFrame();

        return KisImportExportFilter::OK;
    }

    avtranscoder::StreamTranscoder &transcoder;
    avtranscoder::VideoGenerator *generator;
    avtranscoder::VideoFrame frameData;
    const KoColorSpace *dstCS;
    QRect bounds;
    QVector<quint8> buffer;
};

KisImageBuilder_Result VideoSaver::encode(const QString &filename, const QMap<QString, QString> &additionalOptions)
{
    const QFileInfo fileInfo(filename);
    const QString suffix = fileInfo.suffix().toLower();

    using namespace avtranscoder::constants;
    avtranscoder::ProfileLoader::Profile videoProfile;

    if (suffix == "gif") {
        videoProfile[avProfileIdentificator] = "gif";
        videoProfile[avProfileIdentificatorHuman] = "GIF format codec";
        videoProfile[avProfileType] = avProfileTypeVideo;
        videoProfile[avProfilePixelFormat] = "bgr8";
        videoProfile[avProfileCodec] = "gif";
        videoProfile["gifflags"] = "-transdiff";
    } else if (suffix == "mkv" || suffix == "mp4") {
        avtranscoder::ProfileLoader loader(true);
        videoProfile = loader.getProfile("h264-hq");
    } else if (suffix == "ogv") {
        videoProfile[avProfileIdentificator] = "theora";
        videoProfile[avProfileIdentificatorHuman] = "Xiph.Org Theora";
        videoProfile[avProfileType] = avProfileTypeVideo;
        videoProfile[avProfilePixelFormat] = "yuv422p";
        videoProfile[avProfileCodec] = "theora";
        //videoProfile[avProfileBitRate] = "140000";
    }

    for (auto it = additionalOptions.constBegin();
         it != additionalOptions.constEnd(); ++it) {

        videoProfile[it.key().toStdString()] = it.value().toStdString();
    }


    KisImageBuilder_Result retval= KisImageBuilder_RESULT_OK;

    KisImageAnimationInterface *animation = m_image->animationInterface();
    const KoColorSpace *dstCS = KoColorSpaceRegistry::instance()->rgb8();
    const QRect saveRect = m_image->bounds();

    AVPixelFormat inputPixelFormat = AV_PIX_FMT_BGRA;

    try {
        avtranscoder::preloadCodecsAndFormats();
        avtranscoder::Logger::setLogLevel(AV_LOG_DEBUG);

        avtranscoder::OutputFile outputFile(filename.toStdString());

        avtranscoder::VideoCodec inputCodec(avtranscoder::eCodecTypeDecoder, AV_CODEC_ID_RAWVIDEO);
        avtranscoder::VideoFrameDesc imageDesc(saveRect.width(), saveRect.height(), inputPixelFormat);
        imageDesc._fps = animation->framerate();
        inputCodec.setImageParameters(imageDesc);

        avtranscoder::StreamTranscoder transcoder(inputCodec, outputFile, videoProfile);


        avtranscoder::VideoGenerator *generator =
            dynamic_cast<avtranscoder::VideoGenerator*>(transcoder.getCurrentDecoder());
        KIS_ASSERT_RECOVER_NOOP(generator);

        {
            outputFile.beginWrap();
            transcoder.preProcessCodecLatency();

            KisTimeRange clipRange = animation->fullClipRange();
            KisAnimationExporter exporter(m_doc, clipRange.start(), clipRange.end());

            FrameUploader uploader(transcoder, imageDesc, saveRect, dstCS);
            exporter.setSaveFrameCallback(uploader);

            KisImportExportFilter::ConversionStatus status =
                exporter.exportAnimation();

            if (status == KisImportExportFilter::UserCancelled) {
                retval =  KisImageBuilder_RESULT_CANCEL;
            } else if (status != KisImportExportFilter::OK) {
                retval =  KisImageBuilder_RESULT_FAILURE;
            }

            outputFile.endWrap();
        }

    } catch(std::exception& e) {
        std::cerr << "ERROR: during process, an error occured: " << e.what() << std::endl;
    }
    catch(...) {
        std::cerr << "ERROR: during process, an unknown error occured" << std::endl;
    }

    return retval;
}

void VideoSaver::cancel()
{
    m_stop = true;
}
