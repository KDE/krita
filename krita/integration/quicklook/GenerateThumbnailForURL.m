#import <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

void UnzipTask(NSURL *path, NSString *target, NSData **output, NSData **error);
OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize);
void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail);

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

// Run unzip and try to obtain the target file in the archive
void UnzipTask(NSURL *path, NSString *target, NSData **output, NSData **error) {
    NSPipe *errorPipe = [NSPipe pipe];
    NSTask *unzipTask = [NSTask new];
    [unzipTask setLaunchPath:@"/usr/bin/unzip"];
    [unzipTask setStandardOutput:[NSPipe pipe]];
    [unzipTask setStandardError:errorPipe];
    [unzipTask setArguments:@[@"-p", [path path], target]];
    [unzipTask launch];
    //[unzipTask waitUntilExit];
    
    *output = [[[unzipTask standardOutput] fileHandleForReading] readDataToEndOfFile];
    *error = [[[unzipTask standardError] fileHandleForReading] readDataToEndOfFile];
}

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    // Allows you to print the name of the extension on top of the thumbnail preview. Uncomment and change NULL to properties in QLThumbnailRequestSetImage down below to use it.
    /*
    // Get the UTI properties
    NSDictionary* uti_declarations = ( NSDictionary*)UTTypeCopyDeclaration(contentTypeUTI);
    
    // Get the extensions corresponding to the image UTI, for some UTI there can be more than 1 extension (ex image.jpeg = jpeg, jpg...)
    // If it fails for whatever reason fallback to the filename extension
    id extensions = uti_declarations[(__bridge NSString*)kUTTypeTagSpecificationKey][(__bridge NSString*)kUTTagClassFilenameExtension];
    NSString* extension = ([extensions isKindOfClass:[NSArray class]]) ? extensions[0] : extensions;
    if (nil == extension)
        extension = ([(__bridge NSURL*)url pathExtension] != nil) ? [(__bridge NSURL*)url pathExtension] : @"";
    extension = [extension lowercaseString];
    
    // Create the properties dic
    CFTypeRef keys[1] = {kQLThumbnailPropertyExtensionKey};
    CFTypeRef values[1] = {(__bridge CFStringRef)extension};
    CFDictionaryRef properties = CFDictionaryCreate(kCFAllocatorDefault, (const void**)keys, (const void**)values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    */
     
    NSURL *URL = (__bridge NSURL *)url;
    NSData *appPlist = nil;
    NSData *error = nil;
    
    NSImage *appIcon = nil;
    
    UnzipTask(URL, @"preview.png", &appPlist, &error);
    
    // Not made with Krita. Find ORA thumbnail instead.
    if (error.length > 0) {
        UnzipTask(URL, @"Thumbnails/thumbnail.png", &appPlist, &error);
    }
    
    if (QLThumbnailRequestIsCancelled(thumbnail)) {
        [pool release];
        return noErr;
    }
    
    if (appPlist != nil || appPlist.length) {
    
        appIcon = [[NSImage alloc] initWithData:appPlist];
        
        NSRect renderRect = NSMakeRect(0.0, 0.0, appIcon.size.width, appIcon.size.height);
        
        CGImageRef cgImage = [appIcon CGImageForProposedRect:&renderRect context:NULL hints:nil];
        
        QLThumbnailRequestSetImage(thumbnail, cgImage, NULL);
        CGImageRelease(cgImage);
    }
    [pool release];
    return noErr;
}

void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail)
{
    // Implement only if supported
}
