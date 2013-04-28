/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2007 Eric Lamarque <eric.lamarque@free.fr>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

// Initial code from Marco Lamberto abr2gbr (http://the.sunnyspot.org/gimp/)
// Ported code from http://registry.gimp.org/node/126
 
#include <QColor>
#include <QtEndian>
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QFile>
#include <netinet/in.h>
#include <QImage>

struct AbrInfo{
    //big endian
    short version;
    short subversion;
    // count of the images (brushes) in the abr file
    short count;
};

/// save the QImages as png files to directory image_tests
static void saveToQImage(char * buffer,qint32 width, qint32 height, const QString fileName){
    // create 8-bit indexed image
    QImage img(width, height, QImage::Format_Indexed8);
    
    // every image needs color index table
    QVector<QRgb> table;
    for (int i = 0; i < 255; ++i) table.append(qRgb(i, i, i));
    img.setColorTable(table);

    for (int y = 0; y < height;y++){
        memcpy(img.scanLine(y), buffer + width * y, width);
    }

    QImage out(width, height, QImage::Format_ARGB32);
    QColor black(Qt::black);
    out.fill(black.rgb());
    out.setAlphaChannel(img);
    out.save("image_tests/" + fileName + ".png");
} 

static qint32 rle_decode(QDataStream & abr, char *buffer, qint32 height)
{
    qint32 n;
    char ptmp;
    char ch;
    int i, j, c;
    short *cscanline_len;
    char *data = buffer;

    // read compressed size foreach scanline
    cscanline_len = new short[ height ];
    for (i = 0; i < height; i++){
        // short
        abr >> cscanline_len[i];
    }
    
    // unpack each scanline data 
    for (i = 0; i < height; i++) {
        for (j = 0; j < cscanline_len[i];) {
            // char
            abr.device()->getChar(&ptmp);
            n = ptmp;
            
            j++;
            if (n >= 128)     // force sign 
                n -= 256;
            if (n < 0) {      // copy the following char -n + 1 times 
                if (n == -128)  // it's a nop 
                continue;
                n = -n + 1;
                // char
                abr.device()->getChar(&ch);
                
                j++;
                for (c = 0; c < n; c++, data++){
                    *data = ch;
                }
            } else 
            {          
                // read the following n + 1 chars (no compr) 
                for (c = 0; c < n + 1; c++, j++, data++){
                    // char
                    abr.device()->getChar(data);
                }
            }
        }
    }
    delete [] cscanline_len;
    return 0;
}


static QString abr_v1_brush_name (const QString filename, qint32 id)
{
    QString result = filename;
    int pos = filename.lastIndexOf(".");
    result.remove(pos,4);
    QTextStream(&result) << "_" << id;
    return result;
}

static bool abr_supported_content(AbrInfo *abr_hdr)
{
  switch(abr_hdr->version)
    {
    case 1:
    case 2:
      return true;
      break;
    case 6:
      if (abr_hdr->subversion == 1 || abr_hdr->subversion == 2)
        return true;
      break;
    }
  return false;
}

static bool abr_reach_8BIM_section(QDataStream & abr, const QString name)
{
    char tag[4];
    char tagname[5];
    qint32 section_size = 0;
    int r;
  
    // find 8BIMname section
    while(!abr.atEnd()) {
        r = abr.readRawData(tag,4);
        
        if (r != 4) {
            qDebug() << "Error: Cannot read 8BIM tag ";
            return false;
        }
        
        if (strncmp(tag, "8BIM", 4)) {
            qDebug() << "Error: Start tag not 8BIM but " << (int)tag[0] << (int)tag[1] << (int)tag[2] << (int)tag[3] << " at position " << abr.device()->pos();
            return false;
        }
        
        r = abr.readRawData(tagname,4);
        
        if (r != 4) {
            qDebug() << "Error: Cannot read 8BIM tag name";
            return false;
        }
        tagname[4] = '\0';

        QString s1 = QString::fromLatin1(tagname,4);
        
        if (!s1.compare(name)){
            return true;
        }
        
        // long
        abr >> section_size;
        abr.device()->seek( abr.device()->pos() + section_size);
    }
    return true;
}



static qint32 find_sample_count_v6(QDataStream & abr, AbrInfo *abr_info)
{
    qint64 origin;
    qint32 sample_section_size;
    qint32 sample_section_end;
    qint32 samples=0;
    qint32 data_start;
    
    qint32 brush_size;
    qint32 brush_end;

    if (!abr_supported_content(abr_info))
        return 0;

    origin = abr.device()->pos();

    if (!abr_reach_8BIM_section (abr, "samp")) {
        // reset to origin 
        abr.device()->seek(origin);
        return 0;
    }

    // long
    abr >> sample_section_size;
    sample_section_end = sample_section_size + abr.device()->pos();
    
  
  
  data_start = abr.device()->pos();

  while ( ( !abr.atEnd() ) && (abr.device()->pos() < sample_section_end) ) {
    // read long
    abr >> brush_size;
    brush_end = brush_size;
    // complement to 4 
    while (brush_end % 4 != 0) brush_end++;
    abr.device()->seek(abr.device()->pos() + brush_end);
    samples++;
  }

  // set stream to samples data 
  abr.device()->seek(data_start);
  
  //qDebug() <<"samples : "<< samples;
  return samples;
}



static bool abr_read_content(QDataStream & abr, AbrInfo *abr_hdr)
{

    abr >> abr_hdr->version;
    abr_hdr->subversion = 0;
    abr_hdr->count = 0;

    switch (abr_hdr->version)
        {
        case 1:
        case 2:
            abr >> abr_hdr->count;
        break;
        case 6:
            abr >> abr_hdr->subversion;
            abr_hdr->count = find_sample_count_v6(abr, abr_hdr);
        break;
        default:
            // unknown versions 
        break;
        }
    // next bytes in abr are samples data 
    
    return true;
}


static QString abr_read_ucs2_text (QDataStream & abr)
{
    quint64 name_size;
    quint64 buf_size;
    uint   i;
   /* two-bytes characters encoded (UCS-2)
    *  format:
    *   long : size - number of characters in string
    *   data : zero terminated UCS-2 string
    */

    // long
    abr >> name_size;
    if (name_size == 0)
    {
        return QString();
    }
    
    //buf_size = name_size * 2;
    buf_size = name_size;
    
    //name_ucs2 = (char*) malloc (buf_size * sizeof (char));
    //name_ucs2 = new char[buf_size];
    
    ushort * name_ucs2 = new ushort[buf_size];
    for (i = 0; i < buf_size ; i++)
        {
            //* char*/
            //abr >> name_ucs2[i];
            
            // I will use ushort as that is input to fromUtf16
            abr >>  name_ucs2[i];
        }
    QString name_utf8 = QString::fromUtf16(name_ucs2, buf_size);
    delete [] name_ucs2;
    
    return name_utf8;
}


static quint32 abr_brush_load_v6 (QDataStream & abr, AbrInfo *abr_hdr, const QString filename, qint32 image_ID, qint32 id)
{
    Q_UNUSED(image_ID);
    qint32 brush_size;
    qint32 brush_end;
    qint32 complement_to_4;
    qint32 next_brush;

    qint32 top, left, bottom, right;
    short depth;
    char compression;

    qint32 width, height;
    qint32 size;

    qint32 layer_ID = -1;

    char *buffer;

    abr >> brush_size;
    brush_end = brush_size;
    // complement to 4 
    while (brush_end % 4 != 0){ 
        brush_end++;
    }
    
    complement_to_4 = brush_end - brush_size;
    next_brush = abr.device()->pos() + brush_end;

    // discard key
    abr.device()->seek( abr.device()->pos() + 37 );
    if (abr_hdr->subversion == 1)
        // discard short coordinates and unknown short 
        abr.device()->seek( abr.device()->pos() + 10 );
    else
        // discard unknown bytes 
        abr.device()->seek( abr.device()->pos() + 264 );

    // long
    abr >> top;
    abr >> left;
    abr >> bottom;
    abr >> right;
    // short
    abr >> depth;
    // char
    abr.device()->getChar(&compression);

    width = right - left;
    height = bottom - top;
    size = width * (depth >> 3) * height;

    // remove .abr and add some id, so something like test.abr -> test_12345
    QString name = abr_v1_brush_name (filename, id);
    
    buffer = (char*)malloc(size);

    // data decoding 
    if (!compression) {
        // not compressed - read raw bytes as brush data 
        //fread (buffer, size, 1, abr);
        abr.readRawData(buffer,size);
    } else {
        rle_decode (abr, buffer, height);
    }

    saveToQImage(buffer, width, height , name);
    
    free (buffer);
    abr.device()->seek(next_brush);

    layer_ID = id;
    return layer_ID;
}


static qint32 abr_brush_load_v12 (QDataStream & abr, AbrInfo *abr_hdr,const QString filename, qint32 image_ID, qint32 id)
{
    Q_UNUSED(image_ID);
    short brush_type;
    qint32 brush_size;
    qint32 next_brush;

    qint32 top, left, bottom, right;
    qint16 depth;
    char compression;
    QString name;

    qint32 width, height;
    qint32 size;

    qint32            layer_ID = -1;
    char           *buffer;
  
    // short
    abr >> brush_type;
    // long
    abr >> brush_size;
    next_brush = abr.device()->pos() + brush_size;

    switch (brush_type) {
        case 1: 
            // computed brush 
            // FIXME: support it! 
            qDebug() << "WARNING: computed brush unsupported, skipping.";
            abr.device()->seek(abr.device()->pos() + next_brush);
            // TODO: test also this one abr.skipRawData(next_brush);
            break;
        case 2: 
            // sampled brush 
            // discard 4 misc bytes and 2 spacing bytes 
            abr.device()->seek(abr.device()->pos() + 6);
            
            if (abr_hdr->version == 2)
            name = abr_read_ucs2_text (abr);
            if (name.isNull()){
                name = abr_v1_brush_name (filename, id);
            }

            // discard 1 byte for antialiasing and 4 x short for short bounds 
            abr.device()->seek(abr.device()->pos() + 9);

            // long
            abr >> top;
            abr >> left;
            abr >> bottom;
            abr >> right;
            // short
            abr >> depth;
            // char
            abr.device()->getChar(&compression);

            width = right - left;
            height = bottom - top;
            size = width * (depth >> 3) * height;

            /* FIXME: support wide brushes */
            if (height > 16384)
            {
                qDebug() << "WARNING: wide brushes not supported";
                abr.device()->seek(next_brush);
                break;
            }

            buffer = (char*)malloc(size);

            if (!compression)
            // not compressed - read raw bytes as brush data 
            //fread (buffer, size, 1, abr);
            abr.readRawData(buffer,size);
            else{
                rle_decode (abr, buffer, height);
            }

            saveToQImage(buffer,width,height,name);
            
            free (buffer);
            break;

        default:
            qDebug() << "WARNING: unknown brush type, skipping.";
            abr.device()->seek(next_brush);
    }

  return layer_ID;
}


static qint32 abr_brush_load (QDataStream & abr, AbrInfo *abr_hdr,const QString filename, qint32 image_ID, qint32 id)
{
  qint32 layer_ID = -1;
  switch (abr_hdr->version)
    {
    case 1:
    case 2:
      layer_ID = abr_brush_load_v12 (abr, abr_hdr, filename, image_ID, id);
      break;
    case 6:
      layer_ID = abr_brush_load_v6 (abr, abr_hdr, filename, image_ID, id);
      break;
    }

  return layer_ID;
}


static qint32 abr_load (const QString filename)
{
    QFile file(filename);
    AbrInfo abr_hdr;
    qint32 image_ID;
    int i;
    qint32 layer_ID;

  // check if the file is open correctly
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Can't open file " << filename;
        return -1;
    }
    QDataStream abr(&file); 

    if (!abr_read_content (abr, &abr_hdr))
    {
        qDebug() << "Error: cannot parse ABR file: " << filename;
        return -1;
    }

    if (!abr_supported_content (&abr_hdr))
    {
      qDebug() << "ERROR: unable to decode abr format version " << abr_hdr.version << "(subver " << abr_hdr.subversion << ")";
      return -1;
    }

    if (abr_hdr.count == 0)
    {
      qDebug() << "ERROR: no sample brush found in " << filename;
      return -1;
    }

    image_ID = 123456;
    
    for (i = 0; i < abr_hdr.count; i++)
    {
        layer_ID = abr_brush_load(abr, &abr_hdr, filename, image_ID, i + 1);
        if (layer_ID == -1){
            qDebug() << "Warning: problem loading brush #" << i << " in " << filename;
        }
        qDebug() << i + 1 << " / " << abr_hdr.count;
    }
    file.close();
  
    return image_ID;
}


int main(int argc, const char * argv[] ){
    QString fileName;
    if (argc != 2) {
        fileName = "test.abr";
    }else{
        fileName = QString::fromLatin1(argv[1]);
    }
    abr_load(fileName);
    return 0;
}
