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

#include <q3valuevector.h>

#include <kio/job.h>

#include <kis_progress_subject.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_annotation.h"
#include <krita_export.h>


class KisDoc2;
class KisUndoAdapter;

/**
 * Image import/export plugins can use these results to report about success or failure.
 */
enum KisImageBuilder_Result {
        KisImageBuilder_RESULT_FAILURE = -400,
        KisImageBuilder_RESULT_NOT_EXIST = -300,
        KisImageBuilder_RESULT_NOT_LOCAL = -200,
        KisImageBuilder_RESULT_BAD_FETCH = -100,
        KisImageBuilder_RESULT_INVALID_ARG = -50,
        KisImageBuilder_RESULT_OK = 0,
        KisImageBuilder_RESULT_PROGRESS = 1,
        KisImageBuilder_RESULT_EMPTY = 100,
        KisImageBuilder_RESULT_BUSY = 150,
        KisImageBuilder_RESULT_NO_URI = 200,
        KisImageBuilder_RESULT_UNSUPPORTED = 300,
        KisImageBuilder_RESULT_INTR = 400,
        KisImageBuilder_RESULT_PATH = 500,
        KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE = 600
};

/**
 * This class allows to import/export a PNG from either a file or a QIODevice.
 */
class KRITAIMAGE_EXPORT KisPNGConverter : public KisProgressSubject {
        Q_OBJECT
    public:
        /**
         * Initialize the converter.
         * @param doc the KisDoc2 related to the image, can be null if you don't have a KisDoc2
         * @param adapter the undo adapter to be used by the image, can be null if you don't want to use an undo adapter
         */
        KisPNGConverter(KisDoc2 *doc, KisUndoAdapter *adapter);
        virtual ~KisPNGConverter();
    public:
        /**
         * Load an image from an URL. If the image is not on a local drive, the image is first downloaded to a
         * temporary location.
         * @param uri the url of the image
         */
        KisImageBuilder_Result buildImage(const KUrl& uri);
        /**
         * Load an image from a QIODevice.
         * @param iod device to access the data
         */
        KisImageBuilder_Result buildImage(QIODevice* iod);
        /**
         * Save a layer to a PNG
         * @param uri the url of the destination file
         * @param device the paint device to save
         * @param annotationsStart an iterator on the first annotation
         * @param annotationsEnd an iterator on the last annotation
         * @param compression a number between 0 and 9 to specify the compression rate (9 is most compressed)
         * @param interlace set to true if you want to generate an interlaced png
         * @param alpha set to true if you want to save the alpha channel
         */
        KisImageBuilder_Result buildFile(const KUrl& uri, KisPaintDeviceSP device, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd, int compression, bool interlace, bool alpha);
        /**
         * Retrieve the constructed image
         */
        KisImageSP image();
    public slots:
        virtual void cancel();
    private:
        void progress(png_structp png_ptr, png_uint_32 row_number, int pass);
    private:
        png_uint_32 m_max_row;
        KisImageSP m_img;
        KisDoc2 *m_doc;
        KisUndoAdapter *m_adapter;
        bool m_stop;
};

#endif
