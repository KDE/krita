/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef VIDEO_SAVER_H_
#define VIDEO_SAVER_H_

#include <QObject>

#include "kis_types.h"

#include <KisImportExportFilter.h>


class KisFFMpegRunner;

class KisDocument;
class KisAnimationRenderingOptions;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisVideoSaver : public QObject {
    Q_OBJECT
public:
    /**
     * @brief KisVideoSaver
     * This is the object that takes an animation document and config and tells ffmpeg
     * to render it. Log files are generated here too.
     * @param doc the document to use for rendering.
     * @param ffmpegPath the path to the ffmpeg executable.
     * @param batchMode whether Krita is in batchmde and we can thus not show gui widgets.
     */
    KisVideoSaver(KisDocument* doc, bool batchMode);
    ~KisVideoSaver() override;

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
    KisImportExportErrorCode encode(const QString &savedFilesMask, const KisAnimationRenderingOptions &options);

    static KisImportExportErrorCode convert(KisDocument *document, const QString &savedFilesMask, const KisAnimationRenderingOptions &options, bool batchMode);

private:
    KisImageSP m_image;
    KisDocument* m_doc;
    bool m_batchMode;
};

#endif
