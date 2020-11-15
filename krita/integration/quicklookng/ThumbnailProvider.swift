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

    override func provideThumbnail(for request: QLFileThumbnailRequest, _ handler: @escaping (QLThumbnailReply?, Error?) -> Void) {

        // There are three ways to provide a thumbnail through a QLThumbnailReply. Only one of them should be used.

        // First way: Draw the thumbnail into the current context, set up with UIKit's coordinate system.
        /* handler(QLThumbnailReply(contextSize: request.maximumSize, currentContextDrawing: { () -> Bool in
            // Draw the thumbnail here.

            // Return true if the thumbnail was successfully drawn inside this block.
            return true
        }), nil) */



        // Second way: Draw the thumbnail into a context passed to your block, set up with Core Graphics's coordinate system.
        handler(QLThumbnailReply(contextSize: request.maximumSize, drawing: { (context: CGContext) -> Bool in
            // Draw the thumbnail here.

            var appPlist = UnzipTask(request.fileURL.path, "preview.png")

            // Not made with Krita. Find ORA thumbnail instead.
            if (appPlist == nil || appPlist!.isEmpty) {
              appPlist =
                UnzipTask(request.fileURL.path, "Thumbnails/thumbnail.png");
            }

            if (appPlist != nil && !appPlist!.isEmpty) {
              let appIcon = NSImage.init(data: appPlist!);

              let rep = appIcon!.representations.first!;

              var renderRect = NSMakeRect(0.0, 0.0, CGFloat(rep.pixelsWide), CGFloat(rep.pixelsHigh));

              let cgImage = appIcon?.cgImage(forProposedRect: &renderRect, context: nil, hints: nil);

              context.draw(cgImage!, in: renderRect);
            } else {
              return false;
            }

            // Return true if the thumbnail was successfully drawn inside this block.
            return true
        }), nil)

        // Third way: Set an image file URL.
        /* handler(QLThumbnailReply(imageFileURL: Bundle.main.url(forResource: "fileThumbnail", withExtension: "jpg")!), nil) */
    }
}
