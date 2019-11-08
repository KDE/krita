/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "ivfenc.h"

static void mem_put_le16(char* ptr, unsigned int data)
{
    *ptr = static_cast<char>(data & 0xff);
    *(ptr + 1) = static_cast<char>((data & 0xff00) >> 8);
}

static void mem_put_le32(char* ptr, unsigned int data)
{
    *ptr = static_cast<char>(data & 0xff);
    *(ptr + 1) = static_cast<char>((data & 0xff00) >> 8);
    *(ptr + 2) = static_cast<char>((data & 0xff0000) >> 16);
    *(ptr + 3) = static_cast<char>((data & 0xff000000) >> 24);
}

void ivf_write_file_header(FILE* outfile, unsigned int width, unsigned int height, int num, int den,
                           unsigned int fourcc, int frame_cnt)
{
    char header[32];

    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header + 4, 0); // version
    mem_put_le16(header + 6, 32); // header size
    mem_put_le32(header + 8, fourcc); // fourcc
    mem_put_le16(header + 12, width); // width
    mem_put_le16(header + 14, height); // height
    mem_put_le32(header + 16, den); // rate
    mem_put_le32(header + 20, num); // scale
    mem_put_le32(header + 24, frame_cnt); // length
    mem_put_le32(header + 28, 0); // unused

    fwrite(header, 1, 32, outfile);
}

void ivf_write_frame_header(FILE* outfile, int64_t pts, size_t frame_size)
{
    char header[12];

    mem_put_le32(header, (int)frame_size);
    mem_put_le32(header + 4, (int)(pts & 0xFFFFFFFF));
    mem_put_le32(header + 8, (int)(pts >> 32));
    fwrite(header, 1, 12, outfile);
}

void ivf_write_frame_size(FILE* outfile, size_t frame_size)
{
    char header[4];

    mem_put_le32(header, (int)frame_size);
    fwrite(header, 1, 4, outfile);
}
