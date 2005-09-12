/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <magick/api.h>

#include <qfile.h>
#include <qstring.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include <qcolor.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_undo_adapter.h"
#include "kis_image_magick_converter.h"
#include "kis_colorspace_registry.h"
#include "kis_iterators_pixel.h"
#include "kis_strategy_colorspace.h"
#include "kis_paint_device.h"
#include "kis_profile.h"
#include "kis_annotation.h"

#include "../../../config.h"


namespace {

        const PIXELTYPE PIXEL_BLUE = 0;
        const PIXELTYPE PIXEL_GREEN = 1;
        const PIXELTYPE PIXEL_RED = 2;
        const PIXELTYPE PIXEL_ALPHA = 3;
        const PIXELTYPE PIXEL_GRAY = 0;
        const PIXELTYPE PIXEL_GRAY_ALPHA = 1;
        /**
         * Make this more flexible -- although... ImageMagick
         * isn't that flexible either.
         */
        KisStrategyColorSpaceSP getColorSpaceForColorType(ColorspaceType type) {
                if (type == GRAYColorspace) {
                        return KisColorSpaceRegistry::instance() -> get(KisID("GRAYA", ""));
                }
                else if (type == CMYKColorspace) {
                        return KisColorSpaceRegistry::instance() -> get(KisID("CMYK", ""));
                }
                else if (type == RGBColorspace || type == sRGBColorspace || type == TransparentColorspace) {
                        return KisColorSpaceRegistry::instance() -> get(KisID("RGBA", ""));
                }
                return 0;

        }

        KisProfileSP getProfileForProfileInfo(const Image * image, KisStrategyColorSpaceSP cs)
        {
#ifndef HAVE_MAGICK6
                return 0;
#else

                if (image->profiles == NULL)
                        return  0;

                const char *name;
                const StringInfo *profile;

                KisProfileSP p = 0;

                ResetImageProfileIterator(image);
                for (name = GetNextImageProfile(image); name != (char *) NULL; )
                {
                        profile = GetImageProfile(image, name);
                        if (profile == (StringInfo *) NULL)
                                continue;

                        // XXX: Hardcoded for icc type -- is that correct for us?
                        if (QString::compare(name, "icc") == 0) {
                                QByteArray rawdata;
                                rawdata.resize(profile->length);
                                memcpy(rawdata.data(), profile->datum, profile->length);

                                cmsHPROFILE hProfile = cmsOpenProfileFromMem(profile -> datum, (DWORD)profile -> length);

                                if (hProfile == (cmsHPROFILE) NULL) {
                                        return 0;
                                }

                                p = new KisProfile(hProfile, rawdata, cs -> colorSpaceType());
                                Q_CHECK_PTR(p);
                        }
                        name = GetNextImageProfile(image);
                }
                return p;
#endif
        }

        void setAnnotationsForImage(const Image * src, KisImageSP image)
        {
#ifndef HAVE_MAGICK6
                return;
#else
                if (src->profiles == NULL)
                        return;

                const char *name = 0;
                const StringInfo *profile;
                KisAnnotation* annotation = 0;

                ResetImageProfileIterator(src);
                while((name = GetNextImageProfile(src))) {
                        profile = GetImageProfile(src, name);
                        if (profile == (StringInfo *) NULL)
                                continue;

                        // XXX: icc will be written seperately?
                        if (QString::compare(name, "icc") == 0)
                                continue;

                        QByteArray rawdata;
                        rawdata.resize(profile->length);
                        memcpy(rawdata.data(), profile->datum, profile->length);

                        kdDebug() << "Loaded annotation: " << name << endl;

                        annotation = new KisAnnotation(QString(name), "", rawdata);
                        Q_CHECK_PTR(annotation);

                        image -> addAnnotation(annotation);
                }
#endif
        }

        void exportAnnotationsForImage(Image * dst, vKisAnnotationSP_it& it, vKisAnnotationSP_it& annotationsEnd)
        {
#ifndef HAVE_MAGICK6
                return;
#else
                while(it != annotationsEnd) {
                        kdDebug() << "new annotation" << endl;
                        if (!(*it) || (*it) -> type() == QString()) {
                                kdDebug() << "Warning: empty annotation" << endl;
                                ++it;
                                continue;
                        }
                        kdDebug() << "Trying to store annotation of type " << (*it) -> type() << " of size " << (*it) -> annotation() . size() << endl;
                        if (!ProfileImage(dst, (*it) -> type().ascii(),
                                          (unsigned char*)(*it) -> annotation() . data(),
                                          (*it) -> annotation() . size(), MagickFalse)) {
                                kdDebug() << "Storing failed!" << endl;
                        }
                        ++it;
                }
#endif
        }

        void InitGlobalMagick()
        {
                static bool init = false;

                if (!init) {
                        KApplication *app = KApplication::kApplication();

                        InitializeMagick(*app -> argv());
                        atexit(DestroyMagick);
                        init = true;
                }
        }

        /*
         * ImageMagick progress monitor callback.  Unfortunately it doesn't support passing in some user
         * data which complicates things quite a bit.  The plan was to allow the user start multiple
         * import/scans if he/she so wished.  However, without passing user data it's not possible to tell
         * on which task we have made progress on.
         *
         * Additionally, ImageMagick is thread-safe, not re-entrant... i.e. IM does not relinquish held
         * locks when calling user defined callbacks, this means that the same thread going back into IM
         * would deadlock since it would try to acquire locks it already holds.
         */
#ifdef HAVE_MAGICK6
        MagickBooleanType monitor(const char *text, const ExtendedSignedIntegralType, const ExtendedUnsignedIntegralType, ExceptionInfo *)
        {
                KApplication *app = KApplication::kApplication();

                Q_ASSERT(app);

                if (app -> hasPendingEvents())
                        app -> processEvents();

                printf("%s\n", text);
                return MagickTrue;
        }
#else
        unsigned int monitor(const char *text, const ExtendedSignedIntegralType, const ExtendedUnsignedIntegralType, ExceptionInfo *)
        {
                KApplication *app = KApplication::kApplication();

                Q_ASSERT(app);

                if (app -> hasPendingEvents())
                        app -> processEvents();

                printf("%s\n", text);
                return true;
        }
#endif

}

KisImageMagickConverter::KisImageMagickConverter(KisDoc *doc, KisUndoAdapter *adapter)
{
        InitGlobalMagick();
        init(doc, adapter);
        SetMonitorHandler(monitor);
        m_stop = false;
}

KisImageMagickConverter::~KisImageMagickConverter()
{
}

KisImageBuilder_Result KisImageMagickConverter::decode(const KURL& uri, bool isBlob)
{
        Image *image;
        Image *images;
        ExceptionInfo ei;
        ImageInfo *ii;

        if (m_stop) {
                m_img = 0;
                return KisImageBuilder_RESULT_INTR;
        }

        GetExceptionInfo(&ei);
        ii = CloneImageInfo(0);

        if (isBlob) {
                // TODO : Test.  Does BlobToImage even work?
                Q_ASSERT(uri.isEmpty());
                images = BlobToImage(ii, &m_data[0], m_data.size(), &ei);
        } else {
                qstrncpy(ii -> filename, QFile::encodeName(uri.path()), MaxTextExtent - 1);

                if (ii -> filename[MaxTextExtent - 1]) {
                        emit notifyProgressError(this);
                        return KisImageBuilder_RESULT_PATH;
                }

                images = ReadImage(ii, &ei);
        }

        if (ei.severity != UndefinedException)
                CatchException(&ei);

        if (images == 0) {
                DestroyImageInfo(ii);
                DestroyExceptionInfo(&ei);
                emit notifyProgressError(this);
                return KisImageBuilder_RESULT_FAILURE;
        }

        emit notifyProgressStage(this, i18n("Importing..."), 0);

        m_img = 0;

        while ((image = RemoveFirstImageFromList(&images))) {
                ViewInfo *vi = OpenCacheView(image);

                // Determine image type -- rgb, grayscale or cmyk
                KisStrategyColorSpace * cs = 0;
                ColorspaceType colorspaceType;

                // Determine image type -- rgb, grayscale or cmyk
                if (GetImageType(image, &ei) == GrayscaleType || GetImageType(image, &ei) == GrayscaleMatteType) {
                        cs = KisColorSpaceRegistry::instance() -> get("GRAYA");
                        colorspaceType = GRAYColorspace;
                }
                else {
                        colorspaceType = image->colorspace;
                        cs = getColorSpaceForColorType(image -> colorspace);
                }

                if (cs == 0) {
                        kdDebug() << "Krita does not suport profile " << image -> colorspace << "\n";
                        CloseCacheView(vi);
                        DestroyExceptionInfo(&ei);
                        DestroyImageList(images);
                        DestroyImageInfo(ii);
                        emit notifyProgressError(this);
                        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
                }
                kdDebug() << "Image has colorspace: " << cs -> id().name() << "\n";

                KisProfileSP profile = getProfileForProfileInfo(image, cs);
                if (profile)
                        kdDebug() << "Layer has profile: " << profile -> productName() << "\n";

                if( ! m_img) {
                        m_img = new KisImage(m_adapter, image -> columns, image -> rows, cs, m_doc -> nextImageName());
                        Q_CHECK_PTR(m_img);

                        if (profile)
                                m_img -> setProfile(profile);

                        // XXX I'm assuming seperate layers won't have other profile things like EXIF
                        setAnnotationsForImage(image, m_img);
                }

                if (image -> columns && image -> rows) {


                        // Opacity (set by the photoshop import filter)
                        QUANTUM opacity = OPACITY_OPAQUE;
                        const ImageAttribute * attr = GetImageAttribute(image, "[layer-opacity]");
                        if (attr != 0) {
                                opacity = QUANTUM_MAX - Downscale(QString(attr->value).toInt());
                        }

                        KisLayerSP layer = 0;

                        attr = GetImageAttribute(image, "[layer-name]");
                        if (attr != 0) {
                                layer = new KisLayer(m_img, attr->value, opacity);
                        }
                        else {
                                layer = new KisLayer(m_img, m_img -> nextLayerName(), opacity);
                        }

                        Q_ASSERT(layer);

                        // Layerlocation  (set by the photoshop import filter)
                        Q_INT32 x_offset = 0;
                        Q_INT32 y_offset = 0;

                        attr = GetImageAttribute(image, "[layer-xpos]");
                        if (attr != 0) {
                                x_offset = QString(attr->value).toInt();
                        }

                        attr = GetImageAttribute(image, "[layer-ypos]");
                        if (attr != 0) {
                                y_offset = QString(attr->value).toInt();
                        }

                        if (profile)
                                layer -> setProfile(profile);

                        m_img->add(layer, 0);

                        for (Q_UINT32 y = 0; y < image->rows; y ++)
                        {
                                const PixelPacket *pp = AcquireCacheView(vi, 0, y, image->columns, 1, &ei);

                                if(!pp)
                                {
                                        CloseCacheView(vi);
                                        DestroyImageList(images);
                                        DestroyImageInfo(ii);
                                        DestroyExceptionInfo(&ei);
                                        emit notifyProgressError(this);
                                        return KisImageBuilder_RESULT_FAILURE;
                                }




                                KisHLineIteratorPixel hiter = layer -> createHLineIterator(0, y, image->columns, true);
                                while(! hiter.isDone())
                                {

                                        if ( colorspaceType == GRAYColorspace ) {

                                                Q_UINT8 *ptr= hiter.rawData();
                                                // XXX: not colorstrategy and bitdepth independent
                                                *(ptr++) = Downscale(pp->blue);
                                                *(ptr++) = OPACITY_OPAQUE - Downscale(pp->opacity);

                                                pp++;
                                                ++hiter;

                                        }
                                        else {

                                                Q_UINT8 *ptr= hiter.rawData();
                                                // XXX: not colorstrategy and bitdepth independent
                                                *(ptr++) = Downscale(pp->blue);
                                                *(ptr++) = Downscale(pp->green);
                                                *(ptr++) = Downscale(pp->red);
                                                *(ptr++) = OPACITY_OPAQUE - Downscale(pp->opacity);

                                                pp++;
                                                ++hiter;
                                        }
                                }

                                emit notifyProgress(this, y * 100 / image->rows);

                                if (m_stop) {
                                        CloseCacheView(vi);
                                        DestroyImageList(images);
                                        DestroyImageInfo(ii);
                                        DestroyExceptionInfo(&ei);
                                        m_img = 0;
                                        return KisImageBuilder_RESULT_INTR;
                                }
                        }
                        layer->move(x_offset, y_offset);
                }

                emit notifyProgressDone(this);
                CloseCacheView(vi);
                DestroyImage(image);
        }

        emit notifyProgressDone(this);
        DestroyImageList(images);
        DestroyImageInfo(ii);
        DestroyExceptionInfo(&ei);
        return KisImageBuilder_RESULT_OK;
}

KisImageBuilder_Result KisImageMagickConverter::buildImage(const KURL& uri)
{
        if (uri.isEmpty())
                return KisImageBuilder_RESULT_NO_URI;

        if (!KIO::NetAccess::exists(uri, false, qApp -> mainWidget())) {
                return KisImageBuilder_RESULT_NOT_EXIST;
        }

#if 1
        // We're not set up to handle asynchronous loading at the moment.
        KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
        QString tmpFile;

        if (KIO::NetAccess::download(uri, tmpFile, qApp -> mainWidget())) {
                result = decode(tmpFile, false);
                KIO::NetAccess::removeTempFile(tmpFile);
        }

        return result;
#else
        if (!uri.isLocalFile()) {
                if (m_job)
                        return KisImageBuilder_RESULT_BUSY;

                m_data.resize(0);
                m_job = KIO::get(uri, false, false);
                connect(m_job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
                connect(m_job, SIGNAL(totalSize(KIO::Job*, KIO::filesize_t)), this, SLOT(ioTotalSize(KIO::Job*, KIO::filesize_t)));
                connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
                return KisImageBuilder_RESULT_PROGRESS;
        }

        return decode(uri, false);
#endif
}


KisImageSP KisImageMagickConverter::image()
{
        return m_img;
}

void KisImageMagickConverter::init(KisDoc *doc, KisUndoAdapter *adapter)
{
        m_doc = doc;
        m_adapter = adapter;
        m_job = 0;
}

KisImageBuilder_Result buildFile(const KURL&, KisImageSP)
{
        return KisImageBuilder_RESULT_UNSUPPORTED;
}

KisImageBuilder_Result KisImageMagickConverter::buildFile(const KURL& uri, KisLayerSP layer)
{
        Image *image;
        ExceptionInfo ei;
        ImageInfo *ii;

        if (!layer)
                return KisImageBuilder_RESULT_INVALID_ARG;

        KisImageSP img = layer -> image();
        if (!img)
                return KisImageBuilder_RESULT_EMPTY;

        if (uri.isEmpty())
                return KisImageBuilder_RESULT_NO_URI;

        if (!uri.isLocalFile())
                return KisImageBuilder_RESULT_NOT_LOCAL;

        GetExceptionInfo(&ei);

        ii = CloneImageInfo(0);

        qstrncpy(ii -> filename, QFile::encodeName(uri.path()), MaxTextExtent - 1);

        if (ii -> filename[MaxTextExtent - 1]) {
                emit notifyProgressError(this);
                return KisImageBuilder_RESULT_PATH;
        }

        if (!img -> width() || !img -> height())
                return KisImageBuilder_RESULT_EMPTY;

        if ( layer->colorStrategy()->id() == KisID("GRAYA", "") ) {
                ii->colorspace = GRAYColorspace;
        }
        else {
                ii->colorspace = RGBColorspace;
        }
        image = AllocateImage(ii);
        SetImageColorspace(image, ii->colorspace);

        image -> columns = img -> width();
        image -> rows = img -> height();

#ifdef HAVE_MAGICK6
        if ( layer-> alpha() )
                image -> matte = MagickTrue;
        else
                image -> matte = MagickFalse;
#else
        image -> matte = layer -> alpha();
#endif


        Q_INT32 y, height, width;

        height = img -> height();
        width = img -> width();

        bool alpha = layer -> alpha();

        bool gray = layer->colorStrategy()->id() == KisID("GRAYA", "");
        if ( gray ) SetImageType(image, GrayscaleMatteType);

        for (y = 0; y < height; y++) {

                // Allocate pixels for this scanline
                PixelPacket *pp = SetImagePixels(image, 0, y, width, 1);

                if (!pp) {
                        DestroyExceptionInfo(&ei);
                        DestroyImage(image);
                        emit notifyProgressError(this);
                        return KisImageBuilder_RESULT_FAILURE;

                }

                KisHLineIterator it = layer -> createHLineIterator(0, y, width, false);
                while (!it.isDone()) {

                        Q_UINT8 * d = it.rawData();

                        if (!gray)
                        {
                                pp -> red = Upscale(d[PIXEL_RED]);
                                pp -> green = Upscale(d[PIXEL_GREEN]);
                                pp -> blue = Upscale(d[PIXEL_BLUE]);
                                if (alpha)
                                        pp -> opacity = Upscale(OPACITY_OPAQUE - d[PIXEL_ALPHA]);

                        }
                        else {
                                pp -> red = Upscale(d[PIXEL_GRAY]);
                                pp -> green = Upscale(d[PIXEL_GRAY]);
                                pp -> blue = Upscale(d[PIXEL_GRAY]);
                                if (alpha)
                                        pp -> opacity = Upscale(OPACITY_OPAQUE - d[PIXEL_GRAY_ALPHA]);

                        }
                        pp++;
                        ++it;

                }
                emit notifyProgressStage(this, i18n("Saving..."), y * 100 / height);

#ifdef HAVE_MAGICK6
                if (SyncImagePixels(image) == MagickFalse)
                        kdDebug() << "Syncing pixels failed\n";
#else
                if (!SyncImagePixels(image))
                        kdDebug() << "Syncing pixels failed\n";
#endif
        }

        // XXX: Write to a temp file, then have Krita use KIO to copy temp
        // image to remote location.
        WriteImage(ii, image);
        DestroyExceptionInfo(&ei);
        DestroyImage(image);
        emit notifyProgressDone(this);
        return KisImageBuilder_RESULT_OK;
}

KisImageBuilder_Result KisImageMagickConverter::buildFile(const KURL& uri, KisLayerSP layer, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd)
{
        Image *image;
        ExceptionInfo ei;
        ImageInfo *ii;

        if (!layer)
                return KisImageBuilder_RESULT_INVALID_ARG;

        KisImageSP img = layer -> image();
        if (!img)
                return KisImageBuilder_RESULT_EMPTY;

        if (uri.isEmpty())
                return KisImageBuilder_RESULT_NO_URI;

        if (!uri.isLocalFile())
                return KisImageBuilder_RESULT_NOT_LOCAL;

        GetExceptionInfo(&ei);

        ii = CloneImageInfo(0);
        ii->depth = 8;

        qstrncpy(ii -> filename, QFile::encodeName(uri.path()), MaxTextExtent - 1);

        if (ii -> filename[MaxTextExtent - 1]) {
                emit notifyProgressError(this);
                return KisImageBuilder_RESULT_PATH;
        }

        if (!img -> width() || !img -> height())
                return KisImageBuilder_RESULT_EMPTY;

        if ( layer->colorStrategy()->id() == KisID("GRAYA", "") ) {
                ii->colorspace = GRAYColorspace;
        }
        else {
                ii->colorspace = RGBColorspace;
        }

        image = AllocateImage(ii);
        SetImageColorspace(image, ii->colorspace);

        kdDebug() << "Colorspace: " << ii->colorspace << ", " << ", " << image->colorspace << ", " << layer->colorStrategy()->id().id() << "\n";

        image -> columns = img -> width();
        image -> rows = img -> height();

#ifdef HAVE_MAGICK6
        if ( layer-> alpha() )
                image -> matte = MagickTrue;
        else
                image -> matte = MagickFalse;
#else
        image -> matte = layer -> alpha();
#endif


        Q_INT32 y, height, width;

        height = img -> height();
        width = img -> width();

        bool alpha = layer -> alpha();

        bool gray = layer->colorStrategy()->id() == KisID("GRAYA", "");
        if ( gray ) SetImageType(image, GrayscaleMatteType);

        for (y = 0; y < height; y++) {

                // Allocate pixels for this scanline
                PixelPacket *pp = SetImagePixels(image, 0, y, width, 1);

                if (!pp) {
                        DestroyExceptionInfo(&ei);
                        DestroyImage(image);
                        emit notifyProgressError(this);
                        return KisImageBuilder_RESULT_FAILURE;

                }


                KisHLineIterator it = layer -> createHLineIterator(0, y, width, false);
                while (!it.isDone()) {

                        Q_UINT8 * d = it.rawData();

                        if ( gray ) {

                                pp -> red = Upscale(d[PIXEL_GRAY]);
                                pp -> green = Upscale(d[PIXEL_GRAY]);
                                pp -> blue = Upscale(d[PIXEL_GRAY]);
                                if (alpha)
                                        pp -> opacity = Upscale(OPACITY_OPAQUE - d[PIXEL_GRAY_ALPHA]);

                        }
                        else {
                                pp -> red = Upscale(d[PIXEL_RED]);
                                pp -> green = Upscale(d[PIXEL_GREEN]);
                                pp -> blue = Upscale(d[PIXEL_BLUE]);
                                if (alpha)
                                        pp -> opacity = Upscale(OPACITY_OPAQUE - d[PIXEL_ALPHA]);
                        }

                        pp++;
                        ++it;

                }

                emit notifyProgressStage(this, i18n("Saving..."), y * 100 / height);

#ifdef HAVE_MAGICK6
                if (SyncImagePixels(image) == MagickFalse)
                        kdDebug() << "Syncing pixels failed\n";
#else
                if (!SyncImagePixels(image))
                        kdDebug() << "Syncing pixels failed\n";
#endif
        }

        // set the annotations
        exportAnnotationsForImage(image, annotationsStart, annotationsEnd);

        // XXX: Write to a temp file, then have Krita use KIO to copy temp
        // image to remote location.

        WriteImage(ii, image);
        DestroyExceptionInfo(&ei);
        DestroyImage(image);
        emit notifyProgressDone(this);
        return KisImageBuilder_RESULT_OK;
}

void KisImageMagickConverter::ioData(KIO::Job *job, const QByteArray& data)
{
        if (data.isNull() || data.isEmpty()) {
                emit notifyProgressStage(this, i18n("Loading..."), 0);
                return;
        }

        if (m_data.empty()) {
                Image *image;
                ImageInfo *ii;
                ExceptionInfo ei;

                ii = CloneImageInfo(0);
                GetExceptionInfo(&ei);
                image = PingBlob(ii, data.data(), data.size(), &ei);

                if (image == 0 || ei.severity == BlobError) {
                        DestroyExceptionInfo(&ei);
                        DestroyImageInfo(ii);
                        job -> kill();
                        emit notifyProgressError(this);
                        return;
                }

                DestroyImage(image);
                DestroyExceptionInfo(&ei);
                DestroyImageInfo(ii);
                emit notifyProgressStage(this, i18n("Loading..."), 0);
        }

        Q_ASSERT(data.size() + m_data.size() <= m_size);
        memcpy(&m_data[m_data.size()], data.data(), data.count());
        m_data.resize(m_data.size() + data.count());
        emit notifyProgressStage(this, i18n("Loading..."), m_data.size() * 100 / m_size);

        if (m_stop)
                job -> kill();
}

void KisImageMagickConverter::ioResult(KIO::Job *job)
{
        m_job = 0;

        if (job -> error())
                emit notifyProgressError(this);

        decode(KURL(), true);
}

void KisImageMagickConverter::ioTotalSize(KIO::Job * /*job*/, KIO::filesize_t size)
{
        m_size = size;
        m_data.reserve(size);
        emit notifyProgressStage(this, i18n("Loading..."), 0);
}

void KisImageMagickConverter::cancel()
{
        m_stop = true;
}

/**
 * @name readFilters
 * @return Provide a list of file formats the application can read.
 */
QString KisImageMagickConverter::readFilters()
{
        QString s;
        QString all;
        QString name;
        QString description;
        unsigned long matches;

#ifdef HAVE_MAGICK6
#ifdef HAVE_OLD_GETMAGICKINFOLIST
        const MagickInfo **mi;
        mi = GetMagickInfoList("*", &matches);
#else // HAVE_OLD_GETMAGICKINFOLIST
        ExceptionInfo ei;
        GetExceptionInfo(&ei);
        const MagickInfo **mi;
        mi = GetMagickInfoList("*", &matches, &ei);
        DestroyExceptionInfo(&ei);
#endif // HAVE_OLD_GETMAGICKINFOLIST
#else // HAVE_MAGICK6
        const MagickInfo *mi;
        ExceptionInfo ei;
        GetExceptionInfo(&ei);
        mi = GetMagickInfo("*", &ei);
        DestroyExceptionInfo(&ei);
#endif // HAVE_MAGICK6

        if (!mi)
                return s;

#ifdef HAVE_MAGICK6
        for (unsigned long i = 0; i < matches; i++) {
                const MagickInfo *info = mi[i];
                if (info -> stealth)
                        continue;

                if (info -> decoder) {
                        name = info -> name;
                        description = info -> description;
                        kdDebug() << "Found import filter for: " << name << "\n";

                        if (!description.isEmpty() && !description.contains('/')) {
                                all += "*." + name.lower() + " *." + name + " ";
                                s += "*." + name.lower() + " *." + name + "|";
                                s += i18n(description.utf8());
                                s += "\n";
                        }
                }
        }
#else
        for (; mi; mi = reinterpret_cast<const MagickInfo*>(mi -> next)) {
                if (mi -> stealth)
                        continue;
                if (mi -> decoder) {
                        name = mi -> name;
                        description = mi -> description;
                        kdDebug() << "Found import filter for: " << name << "\n";

                        if (!description.isEmpty() && !description.contains('/')) {
                                all += "*." + name.lower() + " *." + name + " ";
                                s += "*." + name.lower() + " *." + name + "|";
                                s += i18n(description.utf8());
                                s += "\n";
                        }
                }
        }
#endif

        all += "|" + i18n("All Images");
        all += "\n";

        return all + s;
}

QString KisImageMagickConverter::writeFilters()
{
        QString s;
        QString all;
        QString name;
        QString description;
        unsigned long matches;

#ifdef HAVE_MAGICK6
#ifdef HAVE_OLD_GETMAGICKINFOLIST
        const MagickInfo **mi;
        mi = GetMagickInfoList("*", &matches);
#else // HAVE_OLD_GETMAGICKINFOLIST
        ExceptionInfo ei;
        GetExceptionInfo(&ei);
        const MagickInfo **mi;
        mi = GetMagickInfoList("*", &matches, &ei);
        DestroyExceptionInfo(&ei);
#endif // HAVE_OLD_GETMAGICKINFOLIST
#else // HAVE_MAGICK6
        const MagickInfo *mi;
        ExceptionInfo ei;
        GetExceptionInfo(&ei);
        mi = GetMagickInfo("*", &ei);
        DestroyExceptionInfo(&ei);
#endif // HAVE_MAGICK6

        if (!mi) {
                kdDebug() << "Eek, no magick info!\n";
                return s;
        }

#ifdef HAVE_MAGICK6
        for (unsigned long i = 0; i < matches; i++) {
                const MagickInfo *info = mi[i];
                kdDebug() << "Found export filter for: " << info -> name << "\n";
                if (info -> stealth)
                        continue;

                if (info -> encoder) {
                        name = info -> name;

                        description = info -> description;

                        if (!description.isEmpty() && !description.contains('/')) {
                                all += "*." + name.lower() + " *." + name + " ";
                                s += "*." + name.lower() + " *." + name + "|";
                                s += i18n(description.utf8());
                                s += "\n";
                        }
                }
        }
#else
        for (; mi; mi = reinterpret_cast<const MagickInfo*>(mi -> next)) {
                kdDebug() << "Found export filter for: " << mi -> name << "\n";
                if (mi -> stealth)
                        continue;

                if (mi -> encoder) {
                        name = mi -> name;

                        description = mi -> description;

                        if (!description.isEmpty() && !description.contains('/')) {
                                all += "*." + name.lower() + " *." + name + " ";
                                s += "*." + name.lower() + " *." + name + "|";
                                s += i18n(description.utf8());
                                s += "\n";
                        }
                }
        }
#endif


        all += "|" + i18n("All Images");
        all += "\n";

        return all + s;
}

#include "kis_image_magick_converter.moc"

