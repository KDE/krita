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

#include "kis_jpeg_destination.h"

#include <jerror.h>

#include <QIODevice>

#include "kis_debug.h"

namespace
{

const qint64 OUTPUT_BUFFER_SIZE = 4096;

struct KisJPEGDestinationManager : public jpeg_destination_mgr
{
    void writeData(j_compress_ptr cinfo, const qint64 numBytesToWrite)
    {
        if (output->write(reinterpret_cast<const char*>(buffer), numBytesToWrite) != numBytesToWrite) {
            ERREXIT(cinfo, JERR_FILE_WRITE);
        }
    }
    
    QIODevice* output;
    JOCTET* buffer;
};

typedef KisJPEGDestinationManager* KisJPEGDestinationManagerPtr;

extern "C"
{

void init_destination(j_compress_ptr cinfo)
{
    KisJPEGDestinationManagerPtr dest = (KisJPEGDestinationManagerPtr)cinfo->dest;
    
    dest->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                  OUTPUT_BUFFER_SIZE*sizeof(JOCTET));
    
    dest->next_output_byte = dest->buffer;
    dest->free_in_buffer = OUTPUT_BUFFER_SIZE;
}

boolean empty_output_buffer(j_compress_ptr cinfo)
{
    KisJPEGDestinationManagerPtr dest = (KisJPEGDestinationManagerPtr)cinfo->dest;
    
    dest->writeData(cinfo, OUTPUT_BUFFER_SIZE);
    dest->next_output_byte = dest->buffer;
    dest->free_in_buffer = OUTPUT_BUFFER_SIZE;
    
    return TRUE;
}

void term_destination(j_compress_ptr cinfo)
{
    KisJPEGDestinationManagerPtr dest = (KisJPEGDestinationManagerPtr)cinfo->dest;
    const qint64 numBytesToWrite = OUTPUT_BUFFER_SIZE-(qint64)dest->free_in_buffer;
    
    if (numBytesToWrite > 0) {
        dest->writeData(cinfo, numBytesToWrite);
    }
}

}
}

namespace KisJPEGDestination
{

void setDestination(j_compress_ptr cinfo, QIODevice* destinationDevice)
{
    if (cinfo->dest == 0) {
        cinfo->dest = (struct jpeg_destination_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_PERMANENT,
                                        sizeof(KisJPEGDestinationManager));
    }
    
    KisJPEGDestinationManagerPtr dest = (KisJPEGDestinationManagerPtr)cinfo->dest;

    dest->init_destination = init_destination;
    dest->empty_output_buffer = empty_output_buffer;
    dest->term_destination = term_destination;
    dest->output = destinationDevice;
}

}

