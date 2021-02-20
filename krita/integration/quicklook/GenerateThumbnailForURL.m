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

OSStatus GenerateThumbnailForURL(void *thisInterface,
                                 QLThumbnailRequestRef thumbnail, CFURLRef url,
                                 CFStringRef contentTypeUTI,
                                 CFDictionaryRef options, CGSize maxSize);
void CancelThumbnailGeneration(void *thisInterface,
                               QLThumbnailRequestRef thumbnail);

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as
   possible
   -----------------------------------------------------------------------------
 */

OSStatus GenerateThumbnailForURL(void *thisInterface,
                                 QLThumbnailRequestRef thumbnail, CFURLRef url,
                                 CFStringRef contentTypeUTI,
                                 CFDictionaryRef options, CGSize maxSize) {
  @autoreleasepool {
    // Allows you to print the name of the extension on top of the thumbnail
    // preview. Uncomment and change NULL to properties in
    // QLThumbnailRequestSetImage down below to use it.
    /*
     // Get the UTI properties
     NSDictionary* uti_declarations = (
     NSDictionary*)UTTypeCopyDeclaration(contentTypeUTI);

     // Get the extensions corresponding to the image UTI, for some UTI there
     can be more than 1 extension (ex image.jpeg = jpeg, jpg...)
     // If it fails for whatever reason fallback to the filename extension
     id extensions = uti_declarations[(__bridge
     NSString*)kUTTypeTagSpecificationKey][(__bridge
     NSString*)kUTTagClassFilenameExtension]; NSString* extension = ([extensions
     isKindOfClass:[NSArray class]]) ? extensions[0] : extensions; if (nil ==
     extension) extension = ([(__bridge NSURL*)url pathExtension] != nil) ?
     [(__bridge NSURL*)url pathExtension] : @""; extension = [extension
     lowercaseString];

     // Create the properties dic
     CFTypeRef keys[1] = {kQLThumbnailPropertyExtensionKey};
     CFTypeRef values[1] = {(__bridge CFStringRef)extension};
     CFDictionaryRef properties = CFDictionaryCreate(kCFAllocatorDefault, (const
     void**)keys, (const void**)values, 1, &kCFTypeDictionaryKeyCallBacks,
     &kCFTypeDictionaryValueCallBacks);
     */

    NSData *appPlist = UnzipTask([(__bridge NSURL *)url path], @"preview.png");

    // Not made with Krita. Find ORA thumbnail instead.
    if (appPlist == nil || [appPlist length] == 0) {
      appPlist =
          UnzipTask([(__bridge NSURL *)url path], @"Thumbnails/thumbnail.png");
    }

    if (QLThumbnailRequestIsCancelled(thumbnail)) {
      return noErr;
    }

    if ([appPlist length]) {
      NSImage *appIcon = [[NSImage alloc] initWithData:appPlist];

      NSImageRep *rep = [[appIcon representations] objectAtIndex:0];

      NSRect renderRect = NSMakeRect(0.0, 0.0, rep.pixelsWide, rep.pixelsHigh);

      CGImageRef cgImage =
          [appIcon CGImageForProposedRect:&renderRect context:NULL hints:nil];

      QLThumbnailRequestSetImage(thumbnail, cgImage, NULL);
    } else {
      return NSFileNoSuchFileError;
    }

    return noErr;
  }
}

void CancelThumbnailGeneration(void *thisInterface,
                               QLThumbnailRequestRef thumbnail) {
  // Implement only if supported
}
