/*
 * Little cms
 * Copyright (C) 1998-2004 Marti Maria <x@uknown.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * iccprofile.h
 *
 * This file provides code to read and write International Color Consortium
 * (ICC) device profiles embedded in JFIF JPEG image files.  The ICC has
 * defined a standard format for including such data in JPEG "APP2" markers.
 * The code given here does not know anything about the internal structure
 * of the ICC profile data; it just knows how to put the profile data into
 * a JPEG file being written, or get it back out when reading.
 *
 * This code depends on new features added to the IJG JPEG library as of
 * IJG release 6b; it will not compile or work with older IJG versions.
 *
 * NOTE: this code would need surgery to work on 16-bit-int machines
 * with ICC profiles exceeding 64K bytes in size.  See iccprofile.c
 * for details.
 */
#ifndef ICCJPEG
#define ICCJPEG

#include <stdio.h>      /* needed to define "FILE", "NULL" */
#include "jpeglib.h"


/*
 * This routine writes the given ICC profile data into a JPEG file.
 * It *must* be called AFTER calling jpeg_start_compress() and BEFORE
 * the first call to jpeg_write_scanlines().
 * (This ordering ensures that the APP2 marker(s) will appear after the
 * SOI and JFIF or Adobe markers, but before all else.)
 */

extern void write_icc_profile JPP((j_compress_ptr cinfo,
                                   const JOCTET *icc_data_ptr,
                                   unsigned int icc_data_len));


/*
 * Reading a JPEG file that may contain an ICC profile requires two steps:
 *
 * 1. After jpeg_create_decompress() but before jpeg_read_header(),
 *    call setup_read_icc_profile().  This routine tells the IJG library
 *    to save in memory any APP2 markers it may find in the file.
 *
 * 2. After jpeg_read_header(), call read_icc_profile() to find out
 *    whether there was a profile and obtain it if so.
 */


/*
 * Prepare for reading an ICC profile
 */

extern void setup_read_icc_profile JPP((j_decompress_ptr cinfo));


/*
 * See if there was an ICC profile in the JPEG file being read;
 * if so, reassemble and return the profile data.
 *
 * true is returned if an ICC profile was found, false if not.
 * If true is returned, *icc_data_ptr is set to point to the
 * returned data, and *icc_data_len is set to its length.
 *
 * IMPORTANT: the data at **icc_data_ptr has been allocated with malloc()
 * and must be freed by the caller with free() when the caller no longer
 * needs it.  (Alternatively, we could write this routine to use the
 * IJG library's memory allocator, so that the data would be freed implicitly
 * at jpeg_finish_decompress() time.  But it seems likely that many apps
 * will prefer to have the data stick around after decompression finishes.)
 */

extern boolean read_icc_profile JPP((j_decompress_ptr cinfo,
                                     JOCTET **icc_data_ptr,
                                     unsigned int *icc_data_len));

#endif
