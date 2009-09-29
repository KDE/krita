/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include "kis_image_magick_converter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <magick/api.h>

#include <qcolor.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>


#include <kis_debug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <colorprofiles/KoIccColorProfile.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_pixel.h"
#include "kis_annotation.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_device.h"

// XXX_PROGRESS: fix progress reporting in this (and other?) file filters

namespace
{

const quint8 PIXEL_BLUE = 0;
const quint8 PIXEL_GREEN = 1;
const quint8 PIXEL_RED = 2;
const quint8 PIXEL_ALPHA = 3;

static const quint8 PIXEL_CYAN = 0;
static const quint8 PIXEL_MAGENTA = 1;
static const quint8 PIXEL_YELLOW = 2;
static const quint8 PIXEL_BLACK = 3;
static const quint8 PIXEL_CMYK_ALPHA = 4;

static const quint8 PIXEL_GRAY = 0;
static const quint8 PIXEL_GRAY_ALPHA = 1;

/**
 * Make this more flexible -- although... ImageMagick
 * isn't that flexible either.
 */
QString colorSpaceName(ColorspaceType type, unsigned long imageDepth = 8)
{

    if (type == GRAYColorspace) {
        if (imageDepth == 8)
            return "GRAYA";
        else if (imageDepth == 16)
            return "GRAYA16" ;
    } else if (type == CMYKColorspace) {
        if (imageDepth == 8)
            return "CMYK";
        else if (imageDepth == 16) {
            return "CMYK16";
        }
    } else if (type == LABColorspace) {
        dbgFile << "Lab!";
        return "LABA";
    } else if (type == RGBColorspace || type == sRGBColorspace || type == TransparentColorspace) {
        if (imageDepth == 8)
            return "RGBA";
        else if (imageDepth == 16)
            return "RGBA16";
    }
    return "";

}

ColorspaceType getColorTypeforColorSpace(const KoColorSpace * cs)
{
    if (KoID(cs->id()) == KoID("GRAYA") || KoID(cs->id()) == KoID("GRAYA16")) return GRAYColorspace;
    if (KoID(cs->id()) == KoID("RGBA") || KoID(cs->id()) == KoID("RGBA16")) return RGBColorspace;
    if (KoID(cs->id()) == KoID("CMYK") || KoID(cs->id()) == KoID("CMYK16")) return CMYKColorspace;
    if (KoID(cs->id()) == KoID("LABA")) return LABColorspace;

//         dbgFile <<"Cannot export images in" + cs->id().name() +" yet.";
    return RGBColorspace;

}

KoColorProfile * profile(const Image * image)
{
    size_t length;

    const unsigned char * profiledata = GetImageProfile(image, "ICM", &length);
    if (profiledata == NULL)
        return 0;
    QByteArray rawdata;
    rawdata.resize(length);
    memcpy(rawdata.data(), profiledata, length);

    KoColorProfile* p = new KoIccColorProfile(rawdata);
    return p;

#if 0
    return 0;

    if (image->profiles == NULL)
        return  0;

    const char *name;
    const StringInfo *profile;

    KoColorProfile * p = 0;

    ResetImageProfileIterator(image);
    for (name = GetNextImageProfile(image); name != (char *) NULL;) {
        profile = GetImageProfile(image, name);
        if (profile == (StringInfo *) NULL)
            continue;

        // XXX: Hardcoded for icc type -- is that correct for us?
        if (QString::compare(name, "icc") == 0) {
            QByteArray rawdata;
            rawdata.resize(profile->length);
            memcpy(rawdata.data(), profile->datum, profile->length);

            p = new KoColorProfile(rawdata);
            if (p == 0)
                return 0;
        }
        name = GetNextImageProfile(image);
    }
    return p;
#endif
}

void setAnnotationsForImage(const Image * src, KisImageWSP image)
{
    size_t length;

    const unsigned char * profiledata = GetImageProfile(src, "IPTC", &length);
    if (profiledata != NULL) {
        QByteArray rawdata;
        rawdata.resize(length);
        memcpy(rawdata.data(), profiledata, length);

        KisAnnotation* annotation = new KisAnnotation(QString("IPTC"), "", rawdata);
        Q_CHECK_PTR(annotation);

            image -> addAnnotation(annotation);
        }
// WARNING Graphics Magick 1.2 has a smaller version number than 1.1
#if MagickLibVersion >= 0x020000 && MagickLibVersion < 0x0100000
        {
          ImageProfileIterator profile_iterator = AllocateImageProfileIterator( src );
          const char * name;
          const unsigned char * profile;
          size_t length;
          while( NextImageProfile( profile_iterator, &name, &profile, &length ) != MagickFail )
          {
              QByteArray rawdata;
              rawdata.resize(length);
              memcpy(rawdata.data(), profile, length);
  
              KisAnnotation* annotation = new KisAnnotation(QString(name), "", rawdata);
              Q_CHECK_PTR(annotation);
  
              image -> addAnnotation(annotation);
          }
        }
#else
        for(int i = 0; i < src->generic_profiles; i++)
        {
            QByteArray rawdata;
            rawdata.resize(src->generic_profile[i].length);
            memcpy(rawdata.data(), src->generic_profile[i].info, src->generic_profile[i].length);

        KisAnnotation* annotation = new KisAnnotation(QString(src->generic_profile[i].name), "", rawdata);
        Q_CHECK_PTR(annotation);

        image -> addAnnotation(annotation);
    }
#endif
    const ImageAttribute* imgAttr = GetImageAttribute(src, NULL);
    while (imgAttr) {
        QByteArray rawdata;
        int len = strlen(imgAttr -> value) + 1;
        rawdata.resize(len);
        memcpy(rawdata.data(), imgAttr -> value, len);

        KisAnnotation* annotation = new KisAnnotation(QString("krita_attribute:%1").arg(QString(imgAttr -> key)), "", rawdata);
        Q_CHECK_PTR(annotation);

        image -> addAnnotation(annotation);

        imgAttr = imgAttr->next;
    }
#if 0
    return;
    if (src->profiles == NULL)
        return;

    const char *name = 0;
    const StringInfo *profile;
    KisAnnotation* annotation = 0;

    // Profiles and so
    ResetImageProfileIterator(src);
    while ((name = GetNextImageProfile(src))) {
        profile = GetImageProfile(src, name);
        if (profile == (StringInfo *) NULL)
            continue;

        // XXX: icc will be written separately?
        if (QString::compare(name, "icc") == 0)
            continue;

        QByteArray rawdata;
        rawdata.resize(profile->length);
        memcpy(rawdata.data(), profile->datum, profile->length);

        annotation = new KisAnnotation(QString(name), "", rawdata);
        Q_CHECK_PTR(annotation);

        image -> addAnnotation(annotation);
    }

    // Attributes, since we have no hint on if this is an attribute or a profile
    // annotation, we prefix it with 'krita_attribute:'. XXX This needs to be rethought!
    // The joys of imagemagick. From at version 6.2.1 (dfaure has 6.2.0 and confirms the
    // old way of doing things) they changed the src -> attributes
    // to void* and require us to use the iterator functions. So we #if around that, *sigh*
#if MagickLibVersion >= 0x621
    const ImageAttribute * attr;
    ResetImageAttributeIterator(src);
    while ((attr = GetNextImageAttribute(src))) {
#else
    ImageAttribute * attr = src -> attributes;
    while (attr) {
#endif
        QByteArray rawdata;
        int len = strlen(attr -> value) + 1;
        rawdata.resize(len);
        memcpy(rawdata.data(), attr -> value, len);

        annotation = new KisAnnotation(
            QString("krita_attribute:%1").arg(QString(attr -> key)), "", rawdata);
        Q_CHECK_PTR(annotation);

        image -> addAnnotation(annotation);
#if MagickLibVersion < 0x620
        attr = attr -> next;
#endif
    }

#endif
}
}

void exportAnnotationsForImage(Image * dst, vKisAnnotationSP_it& it, vKisAnnotationSP_it& annotationsEnd)
{
    while (it != annotationsEnd) {
        if (!(*it) || (*it)->type().isEmpty()) {
            dbgFile << "Warning: empty annotation";
            ++it;
            continue;
        }

        dbgFile << "Trying to store annotation of type" << (*it) -> type() << " of size" << (*it) -> annotation() . size();

        if ((*it) -> type().startsWith(QLatin1String("krita_attribute:"))) { // Attribute
            if (!SetImageAttribute(dst,
                                   (*it) -> type().mid(strlen("krita_attribute:")).ascii(),
                                   (*it) -> annotation() . data())) {
                dbgFile << "Storing of attribute" << (*it) -> type() << "failed!";
            }
        } else { // Profile
            unsigned char * profiledata = new unsigned char[(*it) -> annotation() . size()];
            memcpy(profiledata, (*it) -> annotation() . data(), (*it) -> annotation() . size());
            if (!ProfileImage(dst, (*it) -> type().ascii(),
                              profiledata,
                              (*it) -> annotation() . size(), MagickFalse)) {
                dbgFile << "Storing failed!";
            }
        }
        ++it;
    }
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
#if 0
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



KisImageMagickConverter::KisImageMagickConverter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    InitGlobalMagick();
    init(doc, adapter);
    SetMonitorHandler(monitor);
    m_stop = false;
}

KisImageMagickConverter::~KisImageMagickConverter()
{
}

KisImageBuilder_Result KisImageMagickConverter::decode(const KUrl& uri, bool isBlob)
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
            // XXX_PROGRESS (was: emit notifyProgressError();
            return KisImageBuilder_RESULT_PATH;
        }

        images = ReadImage(ii, &ei);

    }

    if (ei.severity != UndefinedException)
        CatchException(&ei);

    if (images == 0) {
        DestroyImageInfo(ii);
        DestroyExceptionInfo(&ei);
        // XXX_PROGRESS (was: emit notifyProgressError();
        return KisImageBuilder_RESULT_FAILURE;
    }

//     emit notifyProgressStage(i18n("Importing..."), 0);

    m_img = 0;

    while ((image = RemoveFirstImageFromList(&images))) {
        if (image->rows == 0 || image->columns == 0) return KisImageBuilder_RESULT_FAILURE;
        ViewInfo *vi = OpenCacheView(image);

        // Determine image depth -- for now, all channels of an imported image are of the same depth
        unsigned long imageDepth = image->depth;
        dbgFile << "Image depth:" << imageDepth << "";

        QString csName;
        const KoColorSpace * cs = 0;
        ColorspaceType colorspaceType;

        // Determine image type -- rgb, grayscale or cmyk
        if (GetImageType(image, &ei) == GrayscaleType || GetImageType(image, &ei) == GrayscaleMatteType) {
            if (imageDepth == 8)
                csName = "GRAYA";
            else if (imageDepth == 16)
                csName = "GRAYA16" ;
            colorspaceType = GRAYColorspace;
        } else {
            colorspaceType = image->colorspace;
            csName = colorSpaceName(image -> colorspace, imageDepth);
        }

        dbgFile << "image has" << csName << " colorspace";

        KoColorProfile * colorProfile = profile(image);
        if (colorProfile) {
            dbgFile << "image has embedded profile:" << colorProfile -> name() << "";
            cs = KoColorSpaceRegistry::instance()->colorSpace(csName, colorProfile);
        } else
            cs = KoColorSpaceRegistry::instance()->colorSpace(KoID(csName, ""), "");

        if (!cs) {
            dbgFile << "Krita does not support colorspace" << image -> colorspace << "";
            CloseCacheView(vi);
            DestroyImage(image);
            DestroyExceptionInfo(&ei);
            DestroyImageList(images);
            DestroyImageInfo(ii);
            // XXX_PROGRESS (was: emit notifyProgressError();
            return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
        }

        if (! m_img) {
            m_img = new KisImage(m_doc->undoAdapter(), image -> columns, image -> rows, cs, "built image");
            Q_CHECK_PTR(m_img);
            m_img->lock();
            // XXX I'm assuming separate layers won't have other profile things like EXIF
            setAnnotationsForImage(image, m_img);
        }

        if (image -> columns && image -> rows) {

            // Opacity (set by the photoshop import filter)
            quint8 opacity = OPACITY_OPAQUE;
            const ImageAttribute * attr = GetImageAttribute(image, "[layer-opacity]");
            if (attr != 0) {
                opacity = quint8_MAX - ScaleQuantumToChar(QString(attr->value).toInt());
            }

            KisPaintLayerSP layer = 0;

            attr = GetImageAttribute(image, "[layer-name]");
            if (attr != 0) {
                layer = new KisPaintLayer(m_img.data(), attr->value, opacity);
            } else {
                layer = new KisPaintLayer(m_img.data(), m_img -> nextLayerName(), opacity);
            }

            Q_ASSERT(layer);

            // Layerlocation  (set by the photoshop import filter)
            qint32 x_offset = 0;
            qint32 y_offset = 0;

            attr = GetImageAttribute(image, "[layer-xpos]");
            if (attr != 0) {
                x_offset = QString(attr->value).toInt();
            }

            attr = GetImageAttribute(image, "[layer-ypos]");
            if (attr != 0) {
                y_offset = QString(attr->value).toInt();
            }


            for (quint32 y = 0; y < image->rows; y ++) {
                const PixelPacket *pp = AcquireCacheView(vi, 0, y, image->columns, 1, &ei);

                if (!pp) {
                    CloseCacheView(vi);
                    DestroyImageList(images);
                    DestroyImageInfo(ii);
                    DestroyExceptionInfo(&ei);
                    // XXX_PROGRESS (was: emit notifyProgressError();
                    return KisImageBuilder_RESULT_FAILURE;
                }

                IndexPacket * indexes = GetCacheViewIndexes(vi);

                KisHLineIteratorPixel hiter = layer->paintDevice()->createHLineIterator(0, y, image->columns);

                if (colorspaceType == CMYKColorspace) {
                    if (imageDepth == 8) {
                        int x = 0;
                        while (!hiter.isDone()) {
                            quint8 *ptr = hiter.rawData();
                            *(ptr++) = ScaleQuantumToChar(pp->red); // cyan
                            *(ptr++) = ScaleQuantumToChar(pp->green); // magenta
                            *(ptr++) = ScaleQuantumToChar(pp->blue); // yellow
                            *(ptr++) = ScaleQuantumToChar(indexes[x]); // Black
// XXX: Warning! This ifdef messes up the paren matching big-time!
#ifdef HAVE_MAGICK6
                            if (image->matte != MagickFalse) {
#else
                            if (image->matte == true) {
#endif
                                *(ptr++) = OPACITY_OPAQUE - ScaleQuantumToChar(pp->opacity);
                            } else {
                                *(ptr++) = OPACITY_OPAQUE;
                            }
                            ++x;
                            pp++;
                            ++hiter;
                        }
                    }
                } else if (colorspaceType == LABColorspace) {
                    while (! hiter.isDone()) {
                        quint16 *ptr = reinterpret_cast<quint16 *>(hiter.rawData());

                        *(ptr++) = ScaleQuantumToShort(pp->red);
                        *(ptr++) = ScaleQuantumToShort(pp->green);
                        *(ptr++) = ScaleQuantumToShort(pp->blue);
                        *(ptr++) = 65535/*OPACITY_OPAQUE*/ - ScaleQuantumToShort(pp->opacity);

                        pp++;
                        ++hiter;
                    }
                } else if (colorspaceType == RGBColorspace ||
                           colorspaceType == sRGBColorspace ||
                           colorspaceType == TransparentColorspace) {
                    if (imageDepth == 8) {
                        while (! hiter.isDone()) {
                            quint8 *ptr = hiter.rawData();
                            // XXX: not colorstrategy and bitdepth independent
                            *(ptr++) = ScaleQuantumToChar(pp->blue);
                            *(ptr++) = ScaleQuantumToChar(pp->green);
                            *(ptr++) = ScaleQuantumToChar(pp->red);
                            *(ptr++) = OPACITY_OPAQUE - ScaleQuantumToChar(pp->opacity);

                            pp++;
                            ++hiter;
                        }
                    } else if (imageDepth == 16) {
                        while (! hiter.isDone()) {
                            quint16 *ptr = reinterpret_cast<quint16 *>(hiter.rawData());
                            // XXX: not colorstrategy independent
                            *(ptr++) = ScaleQuantumToShort(pp->blue);
                            *(ptr++) = ScaleQuantumToShort(pp->green);
                            *(ptr++) = ScaleQuantumToShort(pp->red);
                            *(ptr++) = 65535/*OPACITY_OPAQUE*/ - ScaleQuantumToShort(pp->opacity);

                            pp++;
                            ++hiter;
                        }
                    }
                } else if (colorspaceType == GRAYColorspace) {
                    if (imageDepth == 8) {
                        while (! hiter.isDone()) {
                            quint8 *ptr = hiter.rawData();
                            // XXX: not colorstrategy and bitdepth independent
                            *(ptr++) = ScaleQuantumToChar(pp->blue);
                            *(ptr++) = OPACITY_OPAQUE - ScaleQuantumToChar(pp->opacity);

                            pp++;
                            ++hiter;
                        }
                    } else if (imageDepth == 16) {
                        while (! hiter.isDone()) {
                            quint16 *ptr = reinterpret_cast<quint16 *>(hiter.rawData());
                            // XXX: not colorstrategy independent
                            *(ptr++) = ScaleQuantumToShort(pp->blue);
                            *(ptr++) = 65535/*OPACITY_OPAQUE*/ - ScaleQuantumToShort(pp->opacity);

                            pp++;
                            ++hiter;
                        }
                    }
                }

//                    emit notifyProgress(y * 100 / image->rows);

                if (m_stop) {
                    CloseCacheView(vi);
                    DestroyImage(image);
                    DestroyImageList(images);
                    DestroyImageInfo(ii);
                    DestroyExceptionInfo(&ei);
                    m_img = 0;
                    return KisImageBuilder_RESULT_INTR;
                }
            }
            m_img->addNode(layer.data(), m_img->rootLayer());
            layer->paintDevice()->move(x_offset, y_offset);
            layer->setDirty();
        }

//             emit notifyProgressDone();
        CloseCacheView(vi);
        DestroyImage(image);
    }

//         emit notifyProgressDone();
    DestroyImageList(images);
    DestroyImageInfo(ii);
    DestroyExceptionInfo(&ei);
    m_img->unlock();
    return KisImageBuilder_RESULT_OK;
}

KisImageBuilder_Result KisImageMagickConverter::buildImage(const KUrl& uri) {
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, false, qApp -> mainWidget())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp -> mainWidget())) {
        KUrl uriTF;
        uriTF.setPath(tmpFile);
        result = decode(uriTF, false);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageWSP KisImageMagickConverter::image() {
    return m_img;
}

void KisImageMagickConverter::init(KisDoc2 *doc, KisUndoAdapter *adapter) {
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
}

KisImageBuilder_Result KisImageMagickConverter::buildFile(const KUrl& uri, KisPaintLayerSP layer, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd) {
    Image *image;
    ExceptionInfo ei;
    ImageInfo *ii;

    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageWSP img = layer->image();
    if (!img)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;


    quint32 layerBytesPerChannel = layer->paintDevice()->pixelSize() / layer->paintDevice()->channelCount();

    GetExceptionInfo(&ei);

    ii = CloneImageInfo(0);

    qstrncpy(ii -> filename, QFile::encodeName(uri.path()), MaxTextExtent - 1);

    if (ii -> filename[MaxTextExtent - 1]) {
        // XXX_PROGRESS (was: emit notifyProgressError();
        return KisImageBuilder_RESULT_PATH;
    }

    if (!img -> width() || !img -> height())
        return KisImageBuilder_RESULT_EMPTY;

    if (layerBytesPerChannel < 2) {
        ii->depth = 8;
    } else {
        ii->depth = 16;
    }

    ii->colorspace = getColorTypeforColorSpace(layer->paintDevice()->colorSpace());

    image = AllocateImage(ii);
//         SetImageColorspace(image, ii->colorspace);
    image -> columns = img -> width();
    image -> rows = img -> height();

    dbgFile << "Saving with colorspace" << image->colorspace << ", (" << layer->paintDevice()->colorSpace()->id() << ")";
    dbgFile << "IM Image thinks it has depth:" << image->depth << "";

#ifdef HAVE_MAGICK6
    //    if ( layer-> hasAlpha() )
    image -> matte = MagickTrue;
    //    else
    //        image -> matte = MagickFalse;
#else
    //    image -> matte = layer -> hasAlpha();
    image -> matte = true;
#endif

    qint32 y, height, width;

    height = img -> height();
    width = img -> width();

    bool alpha = true;
    QString ext = QFileInfo(QFile::encodeName(uri.path())).extension(false).upper();
    if (ext == "BMP") {
        alpha = false;
        qstrncpy(ii->magick, "BMP2", MaxTextExtent - 1);
    } else if (ext == "RGB") {
        qstrncpy(ii->magick, "SGI", MaxTextExtent - 1);
    }

    for (y = 0; y < height; y++) {

        // Allocate pixels for this scanline
        PixelPacket * pp = SetImagePixels(image, 0, y, width, 1);

        if (!pp) {
            DestroyExceptionInfo(&ei);
            DestroyImage(image);
            // XXX_PROGRESS (was: emit notifyProgressError();
            return KisImageBuilder_RESULT_FAILURE;

        }

        KisHLineConstIterator it = layer->paintDevice()->createHLineConstIterator(0, y, width);
        if (alpha)
            SetImageType(image, TrueColorMatteType);
        else
            SetImageType(image,  TrueColorType);

        if (image->colorspace == CMYKColorspace) {

            IndexPacket * indexes = GetIndexes(image);
            int x = 0;
            if (layerBytesPerChannel == 2) {
                while (!it.isDone()) {

                    const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                    pp -> red = ScaleShortToQuantum(d[PIXEL_CYAN]);
                    pp -> green = ScaleShortToQuantum(d[PIXEL_MAGENTA]);
                    pp -> blue = ScaleShortToQuantum(d[PIXEL_YELLOW]);
                    if (alpha)
                        pp -> opacity = ScaleShortToQuantum(65535/*OPACITY_OPAQUE*/ - d[PIXEL_CMYK_ALPHA]);
                    indexes[x] = ScaleShortToQuantum(d[PIXEL_BLACK]);
                    x++;
                    pp++;
                    ++it;
                }
            } else {
                while (!it.isDone()) {

                    const quint8 * d = it.rawData();
                    pp -> red = ScaleCharToQuantum(d[PIXEL_CYAN]);
                    pp -> green = ScaleCharToQuantum(d[PIXEL_MAGENTA]);
                    pp -> blue = ScaleCharToQuantum(d[PIXEL_YELLOW]);
                    if (alpha)
                        pp -> opacity = ScaleCharToQuantum(OPACITY_OPAQUE - d[PIXEL_CMYK_ALPHA]);

                    indexes[x] = ScaleCharToQuantum(d[PIXEL_BLACK]);

                    x++;
                    pp++;
                    ++it;
                }
            }
        } else if (image->colorspace == RGBColorspace ||
                   image->colorspace == sRGBColorspace ||
                   image->colorspace == TransparentColorspace) {
            if (layerBytesPerChannel == 2) {
                while (!it.isDone()) {

                    const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                    pp -> red = ScaleShortToQuantum(d[PIXEL_RED]);
                    pp -> green = ScaleShortToQuantum(d[PIXEL_GREEN]);
                    pp -> blue = ScaleShortToQuantum(d[PIXEL_BLUE]);
                    if (alpha)
                        pp -> opacity = ScaleShortToQuantum(65535/*OPACITY_OPAQUE*/ - d[PIXEL_ALPHA]);

                    pp++;
                    ++it;
                }
            } else {
                while (!it.isDone()) {

                    const quint8 * d = it.rawData();
                    pp -> red = ScaleCharToQuantum(d[PIXEL_RED]);
                    pp -> green = ScaleCharToQuantum(d[PIXEL_GREEN]);
                    pp -> blue = ScaleCharToQuantum(d[PIXEL_BLUE]);
                    if (alpha)
                        pp -> opacity = ScaleCharToQuantum(OPACITY_OPAQUE - d[PIXEL_ALPHA]);

                    pp++;
                    ++it;
                }
            }
        } else if (image->colorspace == GRAYColorspace) {
            SetImageType(image, GrayscaleMatteType);
            if (layerBytesPerChannel == 2) {
                while (!it.isDone()) {

                    const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                    pp -> red = ScaleShortToQuantum(d[PIXEL_GRAY]);
                    pp -> green = ScaleShortToQuantum(d[PIXEL_GRAY]);
                    pp -> blue = ScaleShortToQuantum(d[PIXEL_GRAY]);
                    if (alpha)
                        pp -> opacity = ScaleShortToQuantum(65535/*OPACITY_OPAQUE*/ - d[PIXEL_GRAY_ALPHA]);

                    pp++;
                    ++it;
                }
            } else {
                while (!it.isDone()) {
                    const quint8 * d = it.rawData();
                    pp -> red = ScaleCharToQuantum(d[PIXEL_GRAY]);
                    pp -> green = ScaleCharToQuantum(d[PIXEL_GRAY]);
                    pp -> blue = ScaleCharToQuantum(d[PIXEL_GRAY]);
                    if (alpha)
                        pp -> opacity = ScaleCharToQuantum(OPACITY_OPAQUE - d[PIXEL_GRAY_ALPHA]);

                    pp++;
                    ++it;
                }
            }
        } else {
            dbgFile << "Unsupported image format";
            return KisImageBuilder_RESULT_INVALID_ARG;
        }

//             emit notifyProgressStage(i18n("Saving..."), y * 100 / height);

#ifdef HAVE_MAGICK6
        if (SyncImagePixels(image) == MagickFalse)
            dbgFile << "Syncing pixels failed";
#else
        if (!SyncImagePixels(image))
            dbgFile << "Syncing pixels failed";
#endif
    }

    // set the annotations
    exportAnnotationsForImage(image, annotationsStart, annotationsEnd);

    // XXX: Write to a temp file, then have Krita use KIO to copy temp
    // image to remote location.

    WriteImage(ii, image);
    DestroyExceptionInfo(&ei);
    DestroyImage(image);
//         emit notifyProgressDone();
    return KisImageBuilder_RESULT_OK;
}

void KisImageMagickConverter::ioData(KIO::Job *job, const QByteArray& data) {
    if (data.isNull() || data.isEmpty()) {
//             emit notifyProgressStage(i18n("Loading..."), 0);
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
            // XXX_PROGRESS (was: emit notifyProgressError();
            return;
        }

        DestroyImage(image);
        DestroyExceptionInfo(&ei);
        DestroyImageInfo(ii);
//             emit notifyProgressStage(i18n("Loading..."), 0);
    }

    Q_ASSERT(data.size() + m_data.size() <= m_size);
    memcpy(&m_data[m_data.size()], data.data(), data.count());
    m_data.resize(m_data.size() + data.count());
//         emit notifyProgressStage(i18n("Loading..."), m_data.size() * 100 / m_size);

    if (m_stop)
        job -> kill();
}

void KisImageMagickConverter::ioResult(KIO::Job *job) {
    m_job = 0;

//        if (job -> error())
    // XXX_PROGRESS (was: emit notifyProgressError();

    decode(KUrl(), true);
}

void KisImageMagickConverter::ioTotalSize(KIO::Job * /*job*/, KIO::filesize_t size) {
    m_size = size;
    m_data.reserve(size);
//         emit notifyProgressStage(i18n("Loading..."), 0);
}

void KisImageMagickConverter::cancel() {
    m_stop = true;
}

/**
 * @name readFilters
 * @return Provide a list of file formats the application can read.
 */
QString KisImageMagickConverter::readFilters() {
    QString s;
    QString all;
    QString name;
    QString description;
    unsigned long matches;

    /*#ifdef HAVE_MAGICK6
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
    #else // HAVE_MAGICK6*/
    const MagickInfo *mi;
    ExceptionInfo ei;
    GetExceptionInfo(&ei);
    mi = GetMagickInfo("*", &ei);
    DestroyExceptionInfo(&ei);
// #endif // HAVE_MAGICK6

    if (!mi)
        return s;

    /*#ifdef HAVE_MAGICK6
            for (unsigned long i = 0; i < matches; i++) {
                const MagickInfo *info = mi[i];
                if (info -> stealth)
                    continue;

                if (info -> decoder) {
                    name = info -> name;
                    description = info -> description;
                    dbgFile <<"Found import filter for:" << name <<"";

                    if (!description.isEmpty() && !description.contains('/')) {
                        all += "*." + name.lower() + " *." + name + " ";
                        s += "*." + name.lower() + " *." + name + "|";
                        s += i18n(description.utf8());
                        s += "\n";
                    }
                }
            }
    #else*/
    for (; mi; mi = reinterpret_cast<const MagickInfo*>(mi -> next)) {
        if (mi -> stealth)
            continue;
        if (mi -> decoder) {
            name = mi -> name;
            description = mi -> description;
            dbgFile << "Found import filter for:" << name << "";

            if (!description.isEmpty() && !description.contains('/')) {
                all += "*." + name.lower() + " *." + name + ' ';
                s += "*." + name.lower() + " *." + name + '|';
                s += i18n(description.utf8());
                s += '\n';
            }
        }
    }
// #endif

    all += '|' + i18n("All Images");
    all += '\n';

    return all + s;
}

QString KisImageMagickConverter::writeFilters() {
    QString s;
    QString all;
    QString name;
    QString description;
    unsigned long matches;

    /*#ifdef HAVE_MAGICK6
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
    #else // HAVE_MAGICK6*/
    const MagickInfo *mi;
    ExceptionInfo ei;
    GetExceptionInfo(&ei);
    mi = GetMagickInfo("*", &ei);
    DestroyExceptionInfo(&ei);
// #endif // HAVE_MAGICK6

    if (!mi) {
        dbgFile << "Eek, no magick info!";
        return s;
    }

    /*#ifdef HAVE_MAGICK6
            for (unsigned long i = 0; i < matches; i++) {
                const MagickInfo *info = mi[i];
                dbgFile <<"Found export filter for:" << info -> name <<"";
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
    #else*/
    for (; mi; mi = reinterpret_cast<const MagickInfo*>(mi -> next)) {
        dbgFile << "Found export filter for:" << mi -> name << "";
        if (mi -> stealth)
            continue;

        if (mi -> encoder) {
            name = mi -> name;

            description = mi -> description;

            if (!description.isEmpty() && !description.contains('/')) {
                all += "*." + name.lower() + " *." + name + ' ';
                s += "*." + name.lower() + " *." + name + '|';
                s += i18n(description.utf8());
                s += '\n';
            }
        }
    }
// #endif


    all += '|' + i18n("All Images");
    all += '\n';

    return all + s;
}

#include "kis_image_magick_converter.moc"

