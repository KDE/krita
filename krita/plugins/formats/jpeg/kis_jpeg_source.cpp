/*
 *  Copyright (c) 2009 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_jpeg_source.h"

#include <jerror.h>

#include <QIODevice>

#include "kis_debug.h"

namespace
{

const qint64 INPUT_BUFFER_SIZE = 4096;

struct KisJPEGSourceManager : public jpeg_source_mgr
{
    QIODevice* input;
    JOCTET* buffer;
    bool anyDataReceived;
};

typedef KisJPEGSourceManager* KisJPEGSourceManagerPtr;

extern "C"
{

void init_source(j_decompress_ptr cinfo)
{
    KisJPEGSourceManagerPtr src = (KisJPEGSourceManagerPtr)cinfo->src;
    src->anyDataReceived = false;
}

boolean fill_input_buffer(j_decompress_ptr cinfo)
{
    KisJPEGSourceManagerPtr src = (KisJPEGSourceManagerPtr)cinfo->src;
    qint64 numBytesRead = src->input->read(reinterpret_cast<char*>(src->buffer), INPUT_BUFFER_SIZE);

    if (numBytesRead <= 0) {
        if (!src->anyDataReceived)	{
            /* Treat empty input file as fatal error */
            ERREXIT(cinfo, JERR_INPUT_EMPTY);
        }
        WARNMS(cinfo, JWRN_JPEG_EOF);

        /* Insert a fake EOI marker */
        src->buffer[0] = (JOCTET)0xFF;
        src->buffer[1] = (JOCTET)JPEG_EOI;
        numBytesRead = 2;
    }

    src->next_input_byte = src->buffer;
    src->bytes_in_buffer = numBytesRead;
    src->anyDataReceived = true;

    return TRUE;
}

void skip_input_data(j_decompress_ptr cinfo, long numBytes)
{
    KisJPEGSourceManagerPtr src = (KisJPEGSourceManagerPtr)cinfo->src;

    if (numBytes > 0) {
        while (numBytes > (long)src->bytes_in_buffer) {
            numBytes -= (long)src->bytes_in_buffer;
            (void)fill_input_buffer(cinfo);
        }
        src->next_input_byte += (size_t)numBytes;
        src->bytes_in_buffer -= (size_t)numBytes;
    }
}

void term_source(j_decompress_ptr cinfo)
{
    Q_UNUSED(cinfo);
}

}
}

namespace KisJPEGSource
{

void setSource(j_decompress_ptr cinfo, QIODevice* inputDevice)
{
    KisJPEGSourceManagerPtr src = 0;

    if (cinfo->src == 0) {
        cinfo->src = (struct jpeg_source_mgr*)
                     (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT,
                                                 sizeof(KisJPEGSourceManager));
        src = (KisJPEGSourceManagerPtr)cinfo->src;
        src->buffer = (JOCTET*)
                      (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT,
                                                  INPUT_BUFFER_SIZE*sizeof(JOCTET));
    }

    src = (KisJPEGSourceManagerPtr)cinfo->src;
    src->init_source = init_source;
    src->fill_input_buffer = fill_input_buffer;
    src->skip_input_data = skip_input_data;
    src->resync_to_restart = jpeg_resync_to_restart;
    src->term_source = term_source;
    src->input = inputDevice;
    src->bytes_in_buffer = 0;
    src->next_input_byte = 0;
}

}

