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

// #include <QFileInfo>
// #include <QFile>
// #include <QDir>
// #include <QVector>
// #include <QIODevice>
// #include <QRect>
// #include <KisMimeDatabase.h>

// #include <KisPart.h>
#include <KisDocument.h>
// #include <KoColorSpace.h>
// #include <KoColorSpaceRegistry.h>
// #include <KoColorModelStandardIds.h>

// #include <kis_annotation.h>
// #include <kis_types.h>

// #include <kis_debug.h>
#include <kis_image.h>
// #include <kis_group_layer.h>
// #include <kis_paint_layer.h>
// #include <kis_paint_device.h>
// #include <kis_raster_keyframe_channel.h>
// #include <kis_image_animation_interface.h>
// #include <kis_time_range.h>
// #include <kis_iterator_ng.h>

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

KisImageBuilder_Result VideoSaver::encode(const QString &filename1)
{
    KisImageBuilder_Result retval= KisImageBuilder_RESULT_OK;

    try {
        avtranscoder::preloadCodecsAndFormats();
        avtranscoder::Logger::setLogLevel(AV_LOG_DEBUG);

        avtranscoder::OutputFile outputFile(filename1.toStdString());

        using namespace avtranscoder::constants;
        avtranscoder::ProfileLoader::Profile profile;
        profile[avProfileIdentificator] = "mymkv";
        profile[avProfileIdentificatorHuman] = "mymkv";
        profile[avProfileType] = avProfileTypeFormat;
        profile[avProfileFormat] = "mkv";
        outputFile.setupWrapping(profile);

        avtranscoder::VideoCodec inputCodec(avtranscoder::eCodecTypeDecoder, AV_CODEC_ID_RAWVIDEO);
        avtranscoder::VideoFrameDesc imageDesc(1920, 1080, AV_PIX_FMT_RGBA);
        imageDesc._fps = 25.0;
        inputCodec.setImageParameters(imageDesc);

        avtranscoder::ProfileLoader loader(true);
        avtranscoder::ProfileLoader::Profile videoProfile = loader.getProfile("h264-lq");
        videoProfile["profile"] = "main";

        avtranscoder::StreamTranscoder transcoder(inputCodec, outputFile, videoProfile);


        avtranscoder::VideoGenerator *generator =
            dynamic_cast<avtranscoder::VideoGenerator*>(transcoder.getCurrentDecoder());
        KIS_ASSERT_RECOVER_NOOP(generator);

        QImage canvas(imageDesc._width, imageDesc._height, QImage::Format_RGBA8888);
        QPainter gc(&canvas);


        avtranscoder::VideoFrame intermediateBuffer(imageDesc);

        {
            outputFile.beginWrap();
            transcoder.preProcessCodecLatency();

            for (int i = 0; i < 50; i++) {


                gc.fillRect(i * 10, i * 10, 50, 50, Qt::red);
                intermediateBuffer.assign(canvas.constBits());
                generator->setNextFrame(intermediateBuffer);

                transcoder.processFrame();
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
