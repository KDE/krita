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
#import <CoreData/CoreData.h>
#include <CoreFoundation/CoreFoundation.h>

void colorSpaceAndDepthForID(NSString *_Nonnull const colorSpaceId,
                             NSString *_Nonnull *_Nonnull colorSpace,
                             NSInteger *_Nonnull bitDepth) {
    /*

      Color model id comparison through the ages:

    2.4        2.5          2.6         ideal

    ALPHA      ALPHA        ALPHA       ALPHAU8

    CMYK       CMYK         CMYK        CMYKAU8
               CMYKAF32     CMYKAF32
    CMYKA16    CMYKAU16     CMYKAU16

    GRAYA      GRAYA        GRAYA       GRAYAU8
    GrayF32    GRAYAF32     GRAYAF32
    GRAYA16    GRAYAU16     GRAYAU16

    LABA       LABA         LABA        LABAU16
               LABAF32      LABAF32
               LABAU8       LABAU8

    RGBA       RGBA         RGBA        RGBAU8
    RGBA16     RGBA16       RGBA16      RGBAU16
    RgbAF32    RGBAF32      RGBAF32
    RgbAF16    RgbAF16      RGBAF16

    XYZA16     XYZA16       XYZA16      XYZAU16
               XYZA8        XYZA8       XYZAU8
    XyzAF16    XyzAF16      XYZAF16
    XyzAF32    XYZAF32      XYZAF32

    YCbCrA     YCBCRA8      YCBCRA8     YCBCRAU8
    YCbCrAU16  YCBCRAU16    YCBCRAU16
               YCBCRF32     YCBCRF32
     */

  NSString *normalizedColorSpaceId = [colorSpaceId uppercaseString];

  if ([normalizedColorSpaceId hasPrefix:@"ALPHA"]) {
    *colorSpace = @"Alpha";
  } else if ([normalizedColorSpaceId hasPrefix:@"CMYK"]) {
    *colorSpace = @"CMYK";
  } else if ([normalizedColorSpaceId hasPrefix:@"GRAYA"]) {
    *colorSpace = @"Gray";
  } else if ([normalizedColorSpaceId hasPrefix:@"LABA"]) {
    *colorSpace = @"Lab";
  } else if ([normalizedColorSpaceId hasPrefix:@"RGBA"]) {
    *colorSpace = @"RGB";
  } else if ([normalizedColorSpaceId hasPrefix:@"XYZA"]) {
    *colorSpace = @"XYZ";
  } else if ([normalizedColorSpaceId hasPrefix:@"YCBCR"]) {
    *colorSpace = @"YCbCr";
  }

  if ([normalizedColorSpaceId hasSuffix:@"U8"] ||
      [normalizedColorSpaceId hasSuffix:@"8"]) {
    *bitDepth = 8;
  } else if ([normalizedColorSpaceId hasSuffix:@"U16"] ||
             [normalizedColorSpaceId hasSuffix:@"F16"] ||
             [normalizedColorSpaceId hasSuffix:@"16"]) {
    *bitDepth = 16;
  } else if ([normalizedColorSpaceId hasSuffix:@"F32"]) {
    *bitDepth = 32;
  } else {
    *bitDepth = 8;
  }
}

//==============================================================================
//
//  Get metadata attributes from document files
//
//  The purpose of this function is to extract useful information from the
//  file formats for your document, and set the values into the attribute
//  dictionary for Spotlight to include.
//
//==============================================================================

Boolean GetMetadataForFile(void *thisInterface,
                           CFMutableDictionaryRef attributes,
                           CFStringRef contentTypeUTI, CFStringRef pathToFile) {
  // Pull any available metadata from the file at the specified path
  // Return the attribute keys and attribute values in the dict
  // Return TRUE if successful, FALSE if there was no data provided
  // The path could point to either a Core Data store file in which
  // case we import the store's metadata, or it could point to a Core
  // Data external record file for a specific record instances

  Boolean ok = FALSE;

  @autoreleasepool {
    NSMutableDictionary *properties =
        (__bridge NSMutableDictionary *)attributes;
    NSString *URL = (__bridge NSString *)pathToFile;

#ifdef DEBUG
    NSLog(@"Test... initializing data for %@", pathToFile);
#endif

    @autoreleasepool {
      // maindoc.xml -> DOC get IMAGE attrib width, height
#ifdef DEBUG
      NSLog(@"Test... about to get maindoc metadata for %@", pathToFile);
#endif
      NSData *metaData = UnzipTask(URL, @"maindoc.xml");

      if ([metaData length] == 0) {
#ifdef DEBUG
        NSLog(@"Test... %@ has no metadata available", pathToFile);
#endif
        ok = FALSE;
      } else {
#ifdef DEBUG
        NSLog(@"Test... maindoc metadata for %@ returned %lu bytes", URL,
              (unsigned long)[metaData length]);
#endif
        NSXMLDocument *xmlDoc = [[NSXMLDocument alloc]
            initWithData:metaData
                 options:(NSXMLDocumentTidyXML)error:nil];

        if (xmlDoc) {
#ifdef DEBUG
          NSLog(@"Test... we've got a Calligra manifest @ %@", URL);
#endif
          NSXMLElement *rootNode = [xmlDoc rootElement];

          NSXMLElement *metaNode =
              [xmlDoc nodesForXPath:@"//IMAGE[1]" error:nil].firstObject;

          NSInteger width =
              [[[metaNode attributeForName:@"width"] stringValue] integerValue];
          NSInteger height = [[[metaNode attributeForName:@"height"]
              stringValue] integerValue];

          NSString *xRes = [[metaNode attributeForName:@"x-res"] stringValue];
          NSString *yRes = [[metaNode attributeForName:@"y-res"] stringValue];

          NSString *colorSpaceId =
              [[metaNode attributeForName:@"colorspacename"] stringValue];
          NSString *colorSpace = nil;
          NSInteger bitDepth;
          colorSpaceAndDepthForID(colorSpaceId, &colorSpace, &bitDepth);

          properties[(__bridge NSString *)kMDItemPixelWidth] = @(width);
          properties[(__bridge NSString *)kMDItemPixelHeight] = @(height);
          properties[(__bridge NSString *)kMDItemPixelCount] =
              @(width * height);
          properties[(__bridge NSString *)kMDItemBitsPerSample] = @(bitDepth);
          properties[(__bridge NSString *)kMDItemColorSpace] = colorSpace;
          properties[(__bridge NSString *)kMDItemHasAlphaChannel] =
              (__bridge id _Nullable)(kCFBooleanTrue);
          properties[(__bridge NSString *)kMDItemResolutionHeightDPI] =
              @([xRes integerValue]);
          properties[(__bridge NSString *)kMDItemResolutionWidthDPI] =
              @([yRes integerValue]);

          NSString *profileName =
              [[metaNode attributeForName:@"profile"] stringValue];

          properties[(__bridge NSString *)kMDItemProfileName] = profileName;

          NSString *editor =
              [[rootNode attributeForName:@"editor"] stringValue];
          NSString *version =
              [[rootNode attributeForName:@"kritaVersion"] stringValue];
          NSString *creator =
              [[NSString alloc] initWithFormat:@"%@ %@", editor, version];
          properties[(__bridge NSString *)kMDItemCreator] = creator;

          NSArray<NSXMLElement *> *layers =
              [xmlDoc nodesForXPath:@"//layers[1]/layer" error:nil];
          if (layers) {
            NSMutableArray *layerNames =
                [NSMutableArray arrayWithCapacity:[layers count]];
            for (NSXMLElement *layer in layers) {
              [layerNames
                  addObject:[[layer attributeForName:@"name"] stringValue]];
            }
            properties[(__bridge NSString *)kMDItemLayerNames] = layerNames;
          }

          ok = TRUE;
        }
      }
    }

    @autoreleasepool {
      // documentinfo.xml -> document-info get about get title

#ifdef DEBUG
      NSLog(@"Test... about to get document information for %@", URL);
#endif
      NSData *metaData = UnzipTask(URL, @"documentinfo.xml");

      if ([metaData length] == 0) {
#ifdef DEBUG
        NSLog(@"Test... %@ has no document information available", URL);
#endif
        ok = FALSE;
      } else {
#ifdef DEBUG
        NSLog(@"Test... maindoc metadata for %@ returned %lu bytes", URL,
              (unsigned long)[metaData length]);
#endif
        NSXMLDocument *xmlDoc = [[NSXMLDocument alloc]
            initWithData:metaData
                 options:(NSXMLDocumentTidyXML)error:nil];

        if (xmlDoc) {
#ifdef DEBUG
          NSLog(@"Test... we've got a Krita document info metadata @ %@", URL);
#endif
          NSXMLElement *title =
              [xmlDoc nodesForXPath:@"//about/title[1]" error:nil].firstObject;
          if (title) {
            properties[(__bridge NSString *)kMDItemTitle] = [title stringValue];
          }

          NSArray<NSXMLElement *> *authors =
              [xmlDoc nodesForXPath:@"//author/full-name" error:nil];
          if (authors) {
            NSMutableArray *authorsNames =
                [NSMutableArray arrayWithCapacity:[authors count]];
            for (NSXMLElement *author in authors) {
              [authorsNames addObject:[author stringValue]];
            }
            properties[(__bridge NSString *)kMDItemAuthors] = authorsNames;
          }

          NSXMLElement *abstract =
              [xmlDoc nodesForXPath:@"//about/description[1]" error:nil]
                  .firstObject;
          if (abstract) {
            properties[(__bridge NSString *)kMDItemHeadline] =
                [abstract stringValue];
          }

          ok = TRUE;
        }
      }
    }
#ifdef DEBUG
    NSLog(@"[%@] %@", URL, properties);
#endif
  }

  // Return the status
  return ok;
}
