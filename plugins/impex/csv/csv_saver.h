/*
 *  Copyright (c) 2016 Laszlo Fazekas <mneko@freemail.hu>
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

#ifndef CSV_SAVER_H_
#define CSV_SAVER_H_

#include <QObject>

#include "kis_types.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_png_converter.h"

/* The KisImageBuilder_Result definitions come from kis_png_converter.h here */

#include "csv_layer_record.h"
class KisDocument;

class CSVSaver : public QObject {

    Q_OBJECT

public:
    CSVSaver(KisDocument* doc, bool batchMode);
    ~CSVSaver() override;

    KisImageBuilder_Result buildAnimation(const QString &filename);
    KisImageSP image();

private:
    KisImageBuilder_Result encode(const QString &filename);
    KisImageBuilder_Result getLayer(CSVLayerRecord* , KisDocument* , KisKeyframeSP, const QString &, int, int);
    void createTempImage(KisDocument* );
    QString convertToBlending(const QString &);

private Q_SLOTS:
    void cancel();

private:
    KisImageSP m_image;
    KisDocument* m_doc;
    bool m_batchMode;
    bool m_stop;
};

#endif
