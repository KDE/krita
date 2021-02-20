/*
 * This file is part of Krita
 *
 * Copyright (c) 2015 Algorithmus <angelus.tenebrae@gmail.com>
 * Copyright (c) 2015 beelzy <alvina.lee@innoactive.de>
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
#import <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview,
                               CFURLRef url, CFStringRef contentTypeUTI,
                               CFDictionaryRef options);
void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview);

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   -----------------------------------------------------------------------------
 */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview,
                               CFURLRef url, CFStringRef contentTypeUTI,
                               CFDictionaryRef options) {
  @autoreleasepool {
    NSData *appPlist =
        UnzipTask([(__bridge NSURL *)url path], @"mergedImage.png");

    if (QLPreviewRequestIsCancelled(preview)) {
      return noErr;
    }

    if ([appPlist length]) {
      NSImage *appIcon = [[NSImage alloc] initWithData:appPlist];

      NSImageRep *rep = [[appIcon representations] objectAtIndex:0];

      NSSize canvasSize = NSMakeSize(rep.pixelsWide, rep.pixelsHigh);
      NSRect renderRect = NSMakeRect(0.0, 0.0, rep.pixelsWide, rep.pixelsHigh);

      CGContextRef _context =
          QLPreviewRequestCreateContext(preview, canvasSize, true, NULL);

      if (_context) {
        NSGraphicsContext *_graphicsContext =
            [NSGraphicsContext graphicsContextWithGraphicsPort:_context
                                                       flipped:NO];

        [NSGraphicsContext setCurrentContext:_graphicsContext];
        [appIcon drawInRect:renderRect];

        QLPreviewRequestFlushContext(preview, _context);
        CFRelease(_context);
      }

      return noErr;
    }

    return NSFileNoSuchFileError;
  }
}

void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview) {
  // Implement only if supported
}
