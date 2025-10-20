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

import AppKit
import QuickLookThumbnailing


class ThumbnailProvider: QLThumbnailProvider {

//    override init() {
//        print("initializing thumbnailer for org.krita.kra")
//        super.init()
//    }
    override func provideThumbnail(for request: QLFileThumbnailRequest, _ handler: @escaping (QLThumbnailReply?, Error?) -> Void) {
    #if DEBUG
        NSLog("handler called org.krita.kra")
    #endif
        // There are three ways to provide a thumbnail through a QLThumbnailReply. Only one of them should be used.

        // First way: Draw the thumbnail into the current context, set up with UIKit's coordinate system.
        /* handler(QLThumbnailReply(contextSize: request.maximumSize, currentContextDrawing: { () -> Bool in
            // Draw the thumbnail here.

            // Return true if the thumbnail was successfully drawn inside this block.
            return true
        }), nil) */
        


        // Second way: Draw the thumbnail into a context passed to your block, set up with Core Graphics's coordinate system.
        
        var appPlist = UnzipTask(request.fileURL.path, "preview.png")

        // Not made with Krita. Find ORA thumbnail instead.
        if (appPlist == nil || appPlist!.isEmpty) {
            appPlist = UnzipTask(request.fileURL.path, "Thumbnails/thumbnail.png");
        }

        if (appPlist != nil && !appPlist!.isEmpty) {
            let appIcon = NSImage.init(data: appPlist!)
            
            let rep = appIcon!.representations.first!

#if DEBUG
            NSLog("kritaquicklookngs file: \(request.fileURL.path)")
            for representation in appIcon!.representations {
                if let bitmapRep = representation as? NSBitmapImageRep {
                    NSLog("kritaquicklookngs bmp: \(bitmapRep.pixelsWide)x\(bitmapRep.pixelsHigh)")
                    // Access bitmap data, properties, etc.
                } else {
                    NSLog("kritaquicklookngs: no rep")
                }
            }
#endif
            
            // asume request rect is a rectangle
            var dstwidth: Float
            var dstheight: Float
            let ratio: Float = Float(rep.pixelsHigh) / Float(rep.pixelsWide)
            if ratio > 1.0 {
                dstwidth = Float(request.maximumSize.height) * (1 / ratio)
                dstheight = Float(request.maximumSize.height)
            } else {
                dstwidth = Float(request.maximumSize.width)
                dstheight = Float(request.maximumSize.width) * ratio
            }
            var renderRect = NSMakeRect(0.0, 0.0,
                        CGFloat(dstwidth),
                        CGFloat(dstheight)
            )
#if DEBUG
            NSLog("kritaquicklookngs: \(renderRect.size) \(request.maximumSize) \(dstwidth)x\(dstheight) - \(request.maximumSize.width) x \(request.maximumSize.height)")
            NSLog("kritaquicklookngs_calc: ratio: \(ratio) scale \(request.scale)")
#endif
            
            handler(QLThumbnailReply(contextSize: renderRect.size, drawing: { (context: CGContext) -> Bool in
                //          handler(QLThumbnailReply(contextSize: request.maximumSize, currentContextDrawing: { () -> Bool in
                // Draw the thumbnail here.
                //                appIcon?.draw(in: renderRect)
                
                let cgImage = appIcon?.cgImage(forProposedRect: &renderRect, context: nil, hints: nil);
                renderRect.size.width *= request.scale
                renderRect.size.height *= request.scale
                context.draw(cgImage!, in: renderRect);
                
                // Return true if the thumbnail was successfully drawn inside this block.
                return true
            }), nil)
        }
        // Third way: Set an image file URL.
        /* handler(QLThumbnailReply(imageFileURL: Bundle.main.url(forResource: "fileThumbnail", withExtension: "jpg")!), nil) */
    }
}
