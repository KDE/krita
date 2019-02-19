/*
 *  Copyright (c) 2005, 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PNG_CONVERTER_H_
#define _KIS_PNG_CONVERTER_H_

#include <png.h>

#include <QColor>
#include <QVector>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_annotation.h"
#include <kritaui_export.h>
#include <KisImageBuilderResult.h>

class KoStore;
class KisDocument;
class KoColorSpace;


namespace KisMetaData
{
class Filter;
class Store;
}

struct KisPNGOptions {
    KisPNGOptions()
        : compression(0)
        , interlace(false)
        , alpha(true)
        , exif(true)
        , iptc(true)
        , xmp(true)
        , tryToSaveAsIndexed(true)
        , saveSRGBProfile(false)
        , forceSRGB(false)
        , storeMetaData(false)
        , storeAuthor(false)
        , saveAsHDR(false)
        , transparencyFillColor(Qt::white)
    {}

    int compression;
    bool interlace;
    bool alpha;
    bool exif;
    bool iptc;
    bool xmp;
    bool tryToSaveAsIndexed;
    bool saveSRGBProfile;
    bool forceSRGB;
    bool storeMetaData;
    bool storeAuthor;
    bool saveAsHDR;
    QList<const KisMetaData::Filter*> filters;
    QColor transparencyFillColor;

};

/**
 * This class allows to import/export a PNG from either a file or a QIODevice.
 */
// XXX_PROGRESS (pass KoUpdater to the png converter)
class KRITAUI_EXPORT KisPNGConverter : public QObject
{
    Q_OBJECT
public:
    /**
     * Initialize the converter.
     * @param doc the KisDocument related to the image, can be null if you don't have a KisDocument
     * @param batchMode whether to use the batch mode
     */
    KisPNGConverter(KisDocument *doc, bool batchMode = false);
    ~KisPNGConverter() override;
public:
    /**
     * Load an image from an URL. If the image is not on a local drive, the image is first downloaded to a
     * temporary location.
     * @param filename the file name of the image
     */
    KisImageBuilder_Result buildImage(const QString &filename);
    /**
     * Load an image from a QIODevice.
     * @param iod device to access the data
     */
    KisImageBuilder_Result buildImage(QIODevice* iod);
    /**
     * Save a layer to a PNG
     * @param filename the name of the destination file
     * @param imageRect the image rectangle to save
     * @param xRes resolution along x axis
     * @param yRes resolution along y axis
     * @param device the paint device to save
     * @param annotationsStart an iterator on the first annotation
     * @param annotationsEnd an iterator on the last annotation
     * @param options PNG formatting options
     * @param metaData image metadata
     */
    KisImageBuilder_Result buildFile(const QString &filename, const QRect &imageRect, const qreal xRes, const qreal yRes, KisPaintDeviceSP device, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd, KisPNGOptions options, KisMetaData::Store* metaData);
    KisImageBuilder_Result buildFile(QIODevice*, const QRect &imageRect, const qreal xRes, const qreal yRes, KisPaintDeviceSP device, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd, KisPNGOptions options, KisMetaData::Store* metaData);
    /**
     * Retrieve the constructed image
     */
    KisImageSP image();

    /**
     * @brief saveDeviceToStore saves the given paint device to the KoStore. If the device is not 8 bits sRGB, it will be converted to 8 bits sRGB.
     * @return true if the saving succeeds
     */
    static bool saveDeviceToStore(const QString &filename, const QRect &imageRect, const qreal xRes, const qreal yRes, KisPaintDeviceSP dev, KoStore *store, KisMetaData::Store* metaData = 0);

    static bool isColorSpaceSupported(const KoColorSpace *cs);

public Q_SLOTS:
    virtual void cancel();
private:
    void progress(png_structp png_ptr, png_uint_32 row_number, int pass);
private:
    png_uint_32 m_max_row;
    KisImageSP m_image;
    KisDocument *m_doc;
    bool m_stop;
    bool m_batchMode;
    QString m_path;
};

#endif
