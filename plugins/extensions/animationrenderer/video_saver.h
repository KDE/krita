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

#ifndef VIDEO_SAVER_H_
#define VIDEO_SAVER_H_

#include <QObject>

#include "kis_types.h"

#include "KisImageBuilderResult.h"
#include <KisImportExportFilter.h>

class KisFFMpegRunner;

/* The KisImageBuilder_Result definitions come from kis_png_converter.h here */

class KisDocument;
class KisAnimationRenderingOptions;

class VideoSaver : public QObject {
    Q_OBJECT
public:
    /**
     * @brief VideoSaver
     * This is the object that takes an animation document and config and tells ffmpeg
     * to render it. Log files are generated here too.
     * @param doc the document to use for rendering.
     * @param ffmpegPath the path to the ffmpeg executable.
     * @param batchMode whether Krita is in batchmde and we can thus not show gui widgets.
     */
    VideoSaver(KisDocument* doc, bool batchMode);
    ~VideoSaver() override;

    /**
     * @brief image
     * @return get the image used by this exporter.
     */
    KisImageSP image();
    /**
     * @brief encode the main encoding function.
     * This in turn calls runFFMpeg, which is a private function inside this class.
     * @param filename the filename to which to render the animation.
     * @param configuration the configuration
     * @return whether it is successful or had another failure.
     */
    KisImageBuilder_Result encode(const QString &savedFilesMask, const KisAnimationRenderingOptions &options);

    static KisImportExportFilter::ConversionStatus convert(KisDocument *document, const QString &savedFilesMask, const KisAnimationRenderingOptions &options, bool batchMode);

private:
    KisImageSP m_image;
    KisDocument* m_doc;
    bool m_batchMode;
};

#endif
