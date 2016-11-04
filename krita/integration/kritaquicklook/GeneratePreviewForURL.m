#import <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options);
void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview);

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSURL *URL = (__bridge NSURL *)url;
    
    NSData *appPlist = nil;
    
    NSImage *appIcon = nil;
    NSPipe *errorPipe = [NSPipe pipe];
    
    NSTask *unzipTask = [[NSTask alloc] init];
    [unzipTask setLaunchPath:@"/usr/bin/unzip"];
    [unzipTask setStandardOutput:[NSPipe pipe]];
    [unzipTask setStandardError:errorPipe];
    [unzipTask setArguments:@[@"-p", [URL path], @"mergedimage.png"]];
    [unzipTask launch];
    //[unzipTask waitUntilExit];
    
    appPlist = [[[unzipTask standardOutput] fileHandleForReading] readDataToEndOfFile];
    
    if (QLPreviewRequestIsCancelled(preview)) {
        [pool release];
        return noErr;
    }
    
    if (appPlist != nil || appPlist.length) {
    
        appIcon = [[NSImage alloc] initWithData:appPlist];
        
        NSSize canvasSize = appIcon.size;
        NSRect renderRect = NSMakeRect(0.0, 0.0, appIcon.size.width, appIcon.size.height);
        
        CGContextRef _context = QLPreviewRequestCreateContext(preview, canvasSize, true, NULL);
        
        if (_context) {
            NSGraphicsContext* _graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:_context flipped:NO];
            
            [NSGraphicsContext setCurrentContext:_graphicsContext];
            [appIcon drawInRect:renderRect];
            
            QLPreviewRequestFlushContext(preview, _context);
            CFRelease(_context);
        }
    }
    
    [pool release];
    return noErr;
}

void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
    // Implement only if supported
}
