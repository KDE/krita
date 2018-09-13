/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_TIFF_CONVERTER_H_
#define _KIS_TIFF_CONVERTER_H_

#include <stdio.h>
#include <tiffio.h>

#include <QVector>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_annotation.h"
#include <KisImageBuilderResult.h>
class KisDocument;

struct KisTIFFOptions {
    quint16 compressionType = 0;
    quint16 predictor = 1;
    bool alpha = true;
    bool flatten = true;
    quint16 jpegQuality = 80;
    quint16 deflateCompress = 6;
    quint16 pixarLogCompress = 6;
    bool saveProfile = true;

    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP cfg);
};

class KisTIFFConverter : public QObject
{
    Q_OBJECT
public:
    KisTIFFConverter(KisDocument *doc);
    ~KisTIFFConverter() override;
public:
    KisImageBuilder_Result buildImage(const QString &filename);
    KisImageBuilder_Result buildFile(const QString &filename, KisImageSP layer, KisTIFFOptions);
    /** Retrieve the constructed image
    */
    KisImageSP image();
public Q_SLOTS:
    virtual void cancel();
private:
    KisImageBuilder_Result decode(const QString &filename);
    KisImageBuilder_Result readTIFFDirectory(TIFF* image);
private:
    KisImageSP m_image;
    KisDocument *m_doc;
    bool m_stop;
};

#endif
