#ifndef VPX_IVFENC_H
#define VPX_IVFENC_H

#include <cstdio>
#include <cstdint>

void ivf_write_file_header(FILE *outfile, unsigned int width, unsigned int height,
                           int num, int den,
                           unsigned int fourcc, int frame_cnt);

void ivf_write_frame_header(FILE *outfile, int64_t pts, size_t frame_size);

void ivf_write_frame_size(FILE *outfile, size_t frame_size);

#endif  // IVFENC_H
