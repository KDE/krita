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
#include "kritavideoexport_export.h"

class KisFFMpegRunner;

/* The KisImageBuilder_Result definitions come from kis_png_converter.h here */

class KisDocument;

class KRITAVIDEOEXPORT_EXPORT VideoSaver : public QObject {

    Q_OBJECT
public:
    VideoSaver(KisDocument* doc, bool batchMode);
    virtual ~VideoSaver();

    KisImageSP image();
    KisImageBuilder_Result encode(const QString &filename, const QStringList &additionalOptionsList = QStringList());

    bool hasFFMpeg() const;

private Q_SLOTS:
    void cancel();

private:
    static QString findFFMpeg();

private:
    KisImageSP m_image;
    KisDocument* m_doc;
    bool m_batchMode;
    QString m_ffmpegPath;
    QScopedPointer<KisFFMpegRunner> m_runner;
};

#endif
