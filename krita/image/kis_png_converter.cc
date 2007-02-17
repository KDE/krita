/*
 *  Copyright (c) 2005-2007 Cyrille Berger <cberger@cberger.net>
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

 // A big thank to Glenn Randers-Pehrson for his wonderful documentation of libpng available at http://www.libpng.org/pub/png/libpng-1.2.5-manual.html
#include "kis_png_converter.h"

#include <config-endian.h> // WORDS_BIGENDIAN

#include <limits.h>
#include <stdio.h>

#include <QFile>

#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoDocumentInfo.h>
#include <KoID.h>
#include <KoColorSpaceRegistry.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>
#include <KoColorProfile.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>

namespace {

    const quint8 PIXEL_BLUE = 0;
    const quint8 PIXEL_GREEN = 1;
    const quint8 PIXEL_RED = 2;
    const quint8 PIXEL_ALPHA = 3;


    int getColorTypeforColorSpace( KoColorSpace * cs , bool alpha)
    {
        if ( KoID(cs->id()) == KoID("GRAYA") || KoID(cs->id()) == KoID("GRAYA16") )
        {
            return alpha ? PNG_COLOR_TYPE_GRAY_ALPHA : PNG_COLOR_TYPE_GRAY;
        }
        if ( KoID(cs->id()) == KoID("RGBA") || KoID(cs->id()) == KoID("RGBA16") )
        {
            return alpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB;
        }

        KMessageBox::error(0, i18n("Cannot export images in %1.\n",cs->name()) ) ;
        return -1;

    }


    QString getColorSpaceForColorType(int color_type,int color_nb_bits) {
        if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        {
            switch(color_nb_bits)
            {
                case 8:
                    return "GRAYA";
                case 16:
                    return "GRAYA16";
            }
        } else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA || color_type == PNG_COLOR_TYPE_RGB) {
            switch(color_nb_bits)
            {
                case 8:
                    return "RGBA";
                case 16:
                    return "RGBA16";
            }
        } else if(color_type ==  PNG_COLOR_TYPE_PALETTE) {
            return "RGBA"; // <-- we will convert the index image to RGBA
        }
        return "";
    }


    void fillText(png_text* p_text, const char* key, QString& text)
    {
        p_text->compression = PNG_TEXT_COMPRESSION_zTXt;
        p_text->key = const_cast<char *>(key);
        char* textc = new char[text.length()+1];
        strcpy(textc, text.toAscii());
        p_text->text = textc;
        p_text->text_length = text.length()+1;
    }

}

KisPNGConverter::KisPNGConverter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    Q_ASSERT(doc);
    Q_ASSERT(adapter);

    m_doc = doc;
    m_adapter = adapter;
    m_stop = false;
    m_max_row = 0;
    m_img = 0;
}

KisPNGConverter::~KisPNGConverter()
{
}

class KisPNGStream {
    public:
        KisPNGStream(quint8* buf,  quint32 depth ) : m_posinc(8),m_depth(depth), m_buf(buf) { *m_buf = 0;};
        int nextValue()
        {
            if( m_posinc == 0)
            {
                m_posinc = 8;
                m_buf++;
            }
            m_posinc -= m_depth;
            return (( (*m_buf) >> (m_posinc) ) & ( ( 1 << m_depth ) - 1 ) );
        }
        void setNextValue(int v)
        {
            if( m_posinc == 0)
            {
                m_posinc = 8;
                m_buf++;
                *m_buf = 0;
            }
            m_posinc -= m_depth;
            *m_buf = (v << m_posinc) | *m_buf;
        }
    private:
        quint32 m_posinc, m_depth;
        quint8* m_buf;
};

static
void /*CALLBACK_CALL_TYPE */_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QIODevice *in = (QIODevice *)png_get_io_ptr(png_ptr);

    while (length) {
        int nr = in->read((char*)data, length);
        if (nr <= 0) {
            png_error(png_ptr, "Read Error");
            return;
        }
        length -= nr;
    }
}


KisImageBuilder_Result KisPNGConverter::buildImage(QIODevice* iod)
{
    kDebug(41008) << "Start decoding PNG File" << endl;
    if(not iod->open(QIODevice::ReadOnly))
    {
        kDebug(41008) << "Failed to open PNG File" << endl;
        return (KisImageBuilder_RESULT_FAILURE);
    }
    
    png_byte signature[8];
    iod->peek((char*)signature, 8);
    if (!png_check_sig(signature, 8))
    {
        iod->close();
        return (KisImageBuilder_RESULT_BAD_FETCH);
    }

    // Initialize the internal structures
    png_structp png_ptr =  png_create_read_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
    if (!png_ptr)
    {
        iod->close();
        return (KisImageBuilder_RESULT_FAILURE);
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        iod->close();
        return (KisImageBuilder_RESULT_FAILURE);
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        iod->close();
        return (KisImageBuilder_RESULT_FAILURE);
    }

    // Catch errors
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        iod->close();
        return (KisImageBuilder_RESULT_FAILURE);
    }

    // Initialize the special 
    png_set_read_fn(png_ptr, iod, _read_fn);
    
    // Ignore signature
//     png_set_sig_bytes(png_ptr, 8);
    // read all PNG info up to image data
    png_read_info(png_ptr, info_ptr);

    // Read information about the png
    png_uint_32 width, height;
    int color_nb_bits, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &color_nb_bits, &color_type, &interlace_type, NULL, NULL);

    // swap byteorder on little endian machines.
    #ifndef WORDS_BIGENDIAN
    if (color_nb_bits > 8 )
        png_set_swap(png_ptr);
    #endif

    // Determine the colorspace
    QString csName = getColorSpaceForColorType(color_type, color_nb_bits);
    if(csName.isEmpty()) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        iod->close();
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    bool hasalpha = (color_type == PNG_COLOR_TYPE_RGB_ALPHA || color_type == PNG_COLOR_TYPE_GRAY_ALPHA);

    // Read image profile
    png_charp profile_name, profile_data;
    int compression_type;
    png_uint_32 proflen;
    int number_of_passes = 1;

    if (interlace_type == PNG_INTERLACE_ADAM7)
        number_of_passes = png_set_interlace_handling(png_ptr);

    KoColorProfile* profile = 0;
    if(png_get_iCCP(png_ptr, info_ptr, &profile_name, &compression_type, &profile_data, &proflen))
    {
        QByteArray profile_rawdata;
        // XXX: Hardcoded for icc type -- is that correct for us?
        if (QString::compare(profile_name, "icc") == 0) {
            profile_rawdata.resize(proflen);
            memcpy(profile_rawdata.data(), profile_data, proflen);
            profile = new KoColorProfile(profile_rawdata);
            Q_CHECK_PTR(profile);
            if (profile) {
                kDebug(41008) << "profile name: " << profile->productName() << " profile description: " << profile->productDescription() << " information sur le produit: " << profile->productInfo() << endl;
                if(!profile->isSuitableForOutput())
                {
                    kDebug(41008) << "the profile is not suitable for output and therefore cannot be used in krita, we need to convert the image to a standard profile" << endl; // TODO: in ko2 popup a selection menu to inform the user
                }
            }
        }
    }

    // Retrieve a pointer to the colorspace
    KoColorSpace* cs;
    if (profile && profile->isSuitableForOutput())
    {
        kDebug(41008) << "image has embedded profile: " << profile -> productName() << "\n";
        cs = KoColorSpaceRegistry::instance()->colorSpace(csName, profile);
    }
    else
        cs = KoColorSpaceRegistry::instance()->colorSpace(KoID(csName,""),"");

    if(cs == 0)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }

    // Create the cmsTransform if needed
    cmsHTRANSFORM transform = 0;
    if(profile && !profile->isSuitableForOutput())
    {
        transform = cmsCreateTransform(profile->profile(), cs->colorSpaceType(),
                                       cs->profile()->profile() , cs->colorSpaceType(),
                                       INTENT_PERCEPTUAL, 0);
    }

    // Read comments/texts...
    png_text* text_ptr;
    int num_comments;
    png_get_text(png_ptr, info_ptr, &text_ptr, &num_comments);
    if(m_doc)
    {
        KoDocumentInfo * info = m_doc->documentInfo();
        kDebug(41008) << "There are " << num_comments << " comments in the text" << endl;
        for(int i = 0; i < num_comments; i++)
        {
            kDebug(41008) << "key is " << text_ptr[i].key << " containing " << text_ptr[i].text << endl;
            if(QString::compare(text_ptr[i].key, "title") == 0)
            {
                    info->setAboutInfo("title", text_ptr[i].text);
            } else if(QString::compare(text_ptr[i].key, "abstract")  == 0)
            {
                    info->setAboutInfo("description", text_ptr[i].text);
            } else if(QString::compare(text_ptr[i].key, "author") == 0)
            {
                    info->setAuthorInfo("creator", text_ptr[i].text);
            }
        }
    }

    // Read image data
    png_bytep row_pointer = 0;
    try
    {
        png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
        row_pointer = new png_byte[rowbytes];
    }
    catch(std::bad_alloc& e)
    {
        // new png_byte[] may raise such an exception if the image
        // is invalid / to large.
        kDebug(41008) << "bad alloc: " << e.what() << endl;
        // Free only the already allocated png_byte instances.
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return (KisImageBuilder_RESULT_FAILURE);
    }

    // Read the palette if the file is indexed
    png_colorp palette ;
    int num_palette;
    if(color_type == PNG_COLOR_TYPE_PALETTE) {
        png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
    }
//     png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL );
//     png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr); // By using this function libpng will take care of freeing memory
//    png_read_image(png_ptr, row_pointers);

    // Finish reading the file
//    png_read_end(png_ptr, end_info);
//    fclose(fp);

    // Creating the KisImageSP
    if( m_img) {
        m_img = new KisImage(m_adapter, width, height, cs, "built image");
        Q_CHECK_PTR(m_img);
        if(profile && !profile->isSuitableForOutput())
        {
            KisAnnotationSP annotation;
            // XXX we hardcode icc, this is correct for lcms?
            // XXX productName(), or just "ICC Profile"?
            if (!profile->rawData().isEmpty())
                annotation = new  KisAnnotation("icc", profile->productName(), profile->rawData());
            m_img -> addAnnotation( annotation );
        }
    }

    KisPaintLayer* layer = new KisPaintLayer(m_img.data(), m_img -> nextLayerName(), UCHAR_MAX);
    for (int i = 0; i < number_of_passes; i++)
    {
        for (png_uint_32 y = 0; y < height; y++) {
            KisHLineIterator it = layer -> paintDevice() -> createHLineIterator(0, y, width);
            png_read_rows(png_ptr, &row_pointer, NULL, 1);

            switch(color_type)
            {
                case PNG_COLOR_TYPE_GRAY:
                case PNG_COLOR_TYPE_GRAY_ALPHA:
                    if(color_nb_bits == 16)
                    {
                        quint16 *src = reinterpret_cast<quint16 *>(row_pointer);
                        while (!it.isDone()) {
                            quint16 *d = reinterpret_cast<quint16 *>(it.rawData());
                            d[0] = *(src++);
                            if(transform) cmsDoTransform(transform, d, d, 1);
                            if(hasalpha) d[1] = *(src++);
                            else d[1] = quint16_MAX;
                            ++it;
                        }
                    } else {
                        Q_UINT8 *src = row_pointer;
                        while (!it.isDone()) {
                            Q_UINT8 *d = it.rawData();
                            d[0] = *(src++);
                            if(transform) cmsDoTransform(transform, d, d, 1);
                            if(hasalpha) d[1] = *(src++);
                            else d[1] = UCHAR_MAX;
                            ++it;
                        }
                    }
                //FIXME:should be able to read 1 and 4 bits depth and scale them to 8 bits
                    break;
                case PNG_COLOR_TYPE_RGB:
                case PNG_COLOR_TYPE_RGB_ALPHA:
                    if(color_nb_bits == 16)
                    {
                        quint16 *src = reinterpret_cast<quint16 *>(row_pointer);
                        while (!it.isDone()) {
                            quint16 *d = reinterpret_cast<quint16 *>(it.rawData());
                            d[2] = *(src++);
                            d[1] = *(src++);
                            d[0] = *(src++);
                            if(transform) cmsDoTransform(transform, d, d, 1);
                            if(hasalpha) d[3] = *(src++);
                            else d[3] = quint16_MAX;
                            ++it;
                        }
                    } else {
                        Q_UINT8 *src = row_pointer;
                        while (!it.isDone()) {
                            Q_UINT8 *d = it.rawData();
                            d[2] = *(src++);
                            d[1] = *(src++);
                            d[0] = *(src++);
                            if(transform) cmsDoTransform(transform, d, d, 1);
                            if(hasalpha) d[3] = *(src++);
                            else d[3] = UCHAR_MAX;
                            ++it;
                        }
                    }
                    break;
                case PNG_COLOR_TYPE_PALETTE:
                    {
                        KisPNGStream stream(row_pointer, color_nb_bits);
                        while (!it.isDone()) {
                            Q_UINT8 *d = it.rawData();
                            png_color c = palette[ stream.nextValue() ];
                            d[2] = c.red;
                            d[1] = c.green;
                            d[0] = c.blue;
                            d[3] = UCHAR_MAX;
                            ++it;
                        }
                    }
                    break;
                default:
                    return KisImageBuilder_RESULT_UNSUPPORTED;
            }
        }
    }
    m_img->addLayer(KisLayerSP( layer ), m_img->rootLayer(), KisLayerSP( 0 ));

    png_read_end(png_ptr, end_info);
    iod->close();

    // Freeing memory
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    delete [] row_pointer;

    return KisImageBuilder_RESULT_OK;

}

KisImageBuilder_Result KisPNGConverter::buildImage(const KUrl& uri)
{
    kDebug(41008) << QFile::encodeName(uri.path()) << " " << uri.path() << " " << uri << endl;
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, false, qApp -> activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp -> activeWindow())) {
        KUrl uriTF;
        uriTF.setPath( tmpFile );
        
        // open the file
        kDebug(41008) << QFile::encodeName(uriTF.path()) << " " << uriTF.path() << " " << uriTF << endl;
        QFile *fp = new QFile(QFile::encodeName(uriTF.path()) );
        if (fp->exists())
        {
            result = buildImage(fp);
        } else {
            result = (KisImageBuilder_RESULT_NOT_EXIST);
        }

        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageSP KisPNGConverter::image()
{
    return m_img;
}


KisImageBuilder_Result KisPNGConverter::buildFile(const KUrl& uri, KisPaintDeviceSP device, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd, int compression, bool interlace, bool alpha)
{
    kDebug(41008) << "Start writing PNG File" << endl;
    if (!device)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageSP img = device -> image();
    if (!img)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;
    // Open file for writing
    FILE *fp = fopen(QFile::encodeName(uri.path()), "wb");
    if (!fp)
    {
        return (KisImageBuilder_RESULT_FAILURE);
    }
    int height = img->height();
    int width = img->width();
    // Initialize structures
    png_structp png_ptr =  png_create_write_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
    if (!png_ptr)
    {
        KIO::del(uri); // async
        return (KisImageBuilder_RESULT_FAILURE);
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        KIO::del(uri); // async
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return (KisImageBuilder_RESULT_FAILURE);
    }

    // If an error occurs during writing, libpng will jump here
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        KIO::del(uri); // async
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return (KisImageBuilder_RESULT_FAILURE);
    }
    // Initialize the writing
    png_init_io(png_ptr, fp);
    // Setup the progress function
// FIXME    png_set_write_status_fn(png_ptr, progress);
//     setProgressTotalSteps(100/*height*/);


    /* set the zlib compression level */
    png_set_compression_level(png_ptr, compression);

    /* set other zlib parameters */
    png_set_compression_mem_level(png_ptr, 8);
    png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
    png_set_compression_window_bits(png_ptr, 15);
    png_set_compression_method(png_ptr, 8);
    png_set_compression_buffer_size(png_ptr, 8192);

    int color_nb_bits = 8 * device->pixelSize() / device->channelCount();
    int color_type = getColorTypeforColorSpace(device->colorSpace(), alpha);

    if(color_type == -1)
    {
        return KisImageBuilder_RESULT_UNSUPPORTED;
    }

    // Try to compute a table of color if the colorspace is RGB8f
    png_colorp palette = 0;
    int num_palette = 0;
    if(!alpha && KoID(device->colorSpace()->id()) == KoID("RGBA") )
    { // png doesn't handle indexed images and alpha, and only have indexed for RGB8
        palette = new png_color[255];
        KisRectConstIteratorPixel it = device->createRectConstIterator(0,0, img->width(), img->height());
        bool toomuchcolor = false;
        while( !it.isDone() )
        {
            const Q_UINT8* c = it.rawData();
            bool findit = false;
            for(int i = 0; i < num_palette; i++)
            {
                if(palette[i].red == c[2] &&
                   palette[i].green == c[1] &&
                   palette[i].blue == c[0] )
                {
                    findit = true;
                    break;
                }
            }
            if(!findit)
            {
                if( num_palette == 255)
                {
                    toomuchcolor = true;
                    break;
                }
                palette[num_palette].red = c[2];
                palette[num_palette].green = c[1];
                palette[num_palette].blue = c[0];
                num_palette++;
            }
            ++it;
        }
        if(!toomuchcolor)
        {
            kDebug(41008) << "Found a palette of " << num_palette << " colors" << endl;
            color_type = PNG_COLOR_TYPE_PALETTE;
            if( num_palette <= 2)
            {
                color_nb_bits = 1;
            } else if( num_palette <= 4)
            {
                color_nb_bits = 2;
            } else if( num_palette <= 16)
            {
                color_nb_bits = 4;
            } else {
                color_nb_bits = 8;
            }
        } else {
            delete [] palette;
        }
    }

    int interlacetype = interlace ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;

    png_set_IHDR(png_ptr, info_ptr,
                 width,
                 height,
                 color_nb_bits,
                 color_type, interlacetype,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_sRGB(png_ptr, info_ptr, PNG_sRGB_INTENT_ABSOLUTE);
    // set the palette
    if( color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_PLTE(png_ptr, info_ptr, palette, num_palette);
    }
    // Save annotation
    vKisAnnotationSP_it it = annotationsStart;
    while(it != annotationsEnd) {
        if (!(*it) || (*it) -> type() == QString()) {
            kDebug(41008) << "Warning: empty annotation" << endl;
            ++it;
            continue;
        }

        kDebug(41008) << "Trying to store annotation of type " << (*it) -> type() << " of size " << (*it) -> annotation() . size() << endl;

        if ((*it) -> type().startsWith("krita_attribute:")) { // Attribute
            // FIXME: it should be possible to save krita_attributes in the "CHUNKs"
            kDebug(41008) << "can't save this annotation : " << (*it) -> type() << endl;
        } else { // Profile
            char* name = new char[(*it)->type().length()+1];
            strcpy(name, (*it)->type().toAscii());
            png_set_iCCP(png_ptr, info_ptr, name, PNG_COMPRESSION_TYPE_BASE, (char*)(*it)->annotation().data(), (*it) -> annotation() . size());
        }
        ++it;
    }

    // read comments from the document information
    if(m_doc)
    {
        png_text texts[3];
        int nbtexts = 0;
        KoDocumentInfo * info = m_doc->documentInfo();
        QString title = info->aboutInfo("creator");
        if(!title.isEmpty())
        {
            fillText(texts+nbtexts, "title", title);
            nbtexts++;
        }
        QString abstract = info->aboutInfo("description");
        if(!abstract.isEmpty())
        {
            fillText(texts+nbtexts, "abstract", abstract);
            nbtexts++;
        }
        QString author = info->authorInfo("creator");
        if(!author.isEmpty())
        {
            fillText(texts+nbtexts, "author", author);
            nbtexts++;
        }
    
        png_set_text(png_ptr, info_ptr, texts, nbtexts);
    }
    // Save the information to the file
    png_write_info(png_ptr, info_ptr);
    png_write_flush(png_ptr);

    // swap byteorder on little endian machines.
    #ifndef WORDS_BIGENDIAN
    if (color_nb_bits > 8 )
        png_set_swap(png_ptr);
    #endif

    // Write the PNG
//     png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    // Fill the data structure
    png_byte** row_pointers= new png_byte*[height];

    for (int y = 0; y < height; y++) {
        KisHLineConstIterator it = device->createHLineConstIterator(0, y, width);
        row_pointers[y] = new png_byte[width*device->pixelSize()];
        switch(color_type)
        {
            case PNG_COLOR_TYPE_GRAY:
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                if(color_nb_bits == 16)
                {
                    quint16 *dst = reinterpret_cast<quint16 *>(row_pointers[y]);
                    while (!it.isDone()) {
                        const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                        *(dst++) = d[0];
                        if(alpha) *(dst++) = d[1];
                        ++it;
                    }
                } else {
                    quint8 *dst = row_pointers[y];
                    while (!it.isDone()) {
                        const quint8 *d = it.rawData();
                        *(dst++) = d[0];
                        if(alpha) *(dst++) = d[1];
                        ++it;
                    }
                }
                break;
            case PNG_COLOR_TYPE_RGB:
            case PNG_COLOR_TYPE_RGB_ALPHA:
                if(color_nb_bits == 16)
                {
                    quint16 *dst = reinterpret_cast<quint16 *>(row_pointers[y]);
                    while (!it.isDone()) {
                        const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                        *(dst++) = d[2];
                        *(dst++) = d[1];
                        *(dst++) = d[0];
                        if(alpha) *(dst++) = d[3];
                        ++it;
                    }
                } else {
                    quint8 *dst = row_pointers[y];
                    while (!it.isDone()) {
                        const quint8 *d = it.rawData();
                        *(dst++) = d[2];
                        *(dst++) = d[1];
                        *(dst++) = d[0];
                        if(alpha) *(dst++) = d[3];
                        ++it;
                    }
                }
                break;
            case PNG_COLOR_TYPE_PALETTE:
            {
                Q_UINT8 *dst = row_pointers[y];
                KisPNGStream writestream(dst, color_nb_bits);
                while (!it.isDone()) {
                    const Q_UINT8 *d = it.rawData();
                    int i;
                    for(i = 0; i < num_palette; i++)
                    {
                        if(palette[i].red == d[2] &&
                           palette[i].green == d[1] &&
                           palette[i].blue == d[0] )
                        {
                            break;
                        }
                    }
                    writestream.setNextValue(i);
                    ++it;
                }
            }
            break;
            default:
                KIO::del(uri); // async
#ifdef __GNUC__
#warning Leaks row_pointers and all rows so far allocated (CID 3087) but that code is never called unless someone either introduce a bug/hack in a png and or the png specification gets extended
#endif
                return KisImageBuilder_RESULT_UNSUPPORTED;
        }
    }

    png_write_image(png_ptr, row_pointers);

    // Writing is over
    png_write_end(png_ptr, info_ptr);

    // Free memory
    png_destroy_write_struct(&png_ptr, &info_ptr);
    for (int y = 0; y < height; y++) {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    if( color_type == PNG_COLOR_TYPE_PALETTE)
    {
        delete [] palette;
    }

    fclose(fp);

    return KisImageBuilder_RESULT_OK;
}


void KisPNGConverter::cancel()
{
    m_stop = true;
}

void KisPNGConverter::progress(png_structp png_ptr, png_uint_32 row_number, int pass)
{
    if(png_ptr == NULL || row_number > PNG_MAX_UINT || pass > 7) return;
//     setProgress(row_number);
}


#include "kis_png_converter.moc"

