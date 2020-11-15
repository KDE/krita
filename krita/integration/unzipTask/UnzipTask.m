/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include "UnzipTask.h"
#include "unzip.h"

NSData *UnzipTask(NSString *_Nonnull path, NSString *_Nonnull target) {
  // Spotlight plugins are sandboxed, trying to spawn unzip results in sandboxd
  // killing us

  NSData *output = nil;

  const unzFile zip =
      unzOpen64([path cStringUsingEncoding:NSUTF8StringEncoding]);

  if (zip != NULL) {
#ifdef DEBUG
    NSLog(@"Test... opened file successfully (%@)", path);
#endif
    int f = unzLocateFile(
        zip, [target cStringUsingEncoding:NSUTF8StringEncoding], 0);

    if (f == UNZ_OK) {
#ifdef DEBUG
      NSLog(@"Test... located %@ successfully (%@)", target, path);
#endif
      f = unzOpenCurrentFile(zip);

      if (f == UNZ_OK) {
#ifdef DEBUG
        NSLog(@"Test... opened %@ successfully (%@)", target, path);
#endif
        struct unz_file_info64_s fileInfo;

        f = unzGetCurrentFileInfo64(zip, &fileInfo, NULL, 0, NULL, 0, NULL, 0);

        if (f == UNZ_OK) {
#ifdef DEBUG
          NSLog(@"Test... %@ size: %llu", target, fileInfo.uncompressed_size);
#endif
          void *buf = malloc(fileInfo.uncompressed_size);

          f = unzReadCurrentFile(zip, buf,
                                 (unsigned)fileInfo.uncompressed_size);

          if (f == fileInfo.uncompressed_size) {
#ifdef DEBUG
            NSLog(@"Test... read %@ successfully, copying contents to buffer "
                  @"(%@)",
                  target, path);
#endif
            output = [[NSData alloc] initWithBytes:buf
                                            length:fileInfo.uncompressed_size];
          }
#ifdef DEBUG
          else {
            NSLog(@"Test... failed to read (errno=%d) (%@)", f, target);
          }
#endif

          unzCloseCurrentFile(zip);

          free(buf);
        }
      }

      unzClose(zip);
    }
  }

  return output;
}
