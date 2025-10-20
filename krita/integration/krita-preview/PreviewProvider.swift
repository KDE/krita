//
//  PreviewProvider.swift
//  krita-preview
//
//  Created by Iván Yossi on 20/10/25.
//  Copyright © 2025 Stichting Krita Foundation. All rights reserved.
//

import Cocoa
import Quartz

class PreviewProvider: QLPreviewProvider, QLPreviewingController {
    

    /*
     Use a QLPreviewProvider to provide data-based previews.
     
     To set up your extension as a data-based preview extension:

     - Modify the extension's Info.plist by setting
       <key>QLIsDataBasedPreview</key>
       <true/>
     
     - Add the supported content types to QLSupportedContentTypes array in the extension's Info.plist.

     - Change the NSExtensionPrincipalClass to this class.
       e.g.
       <key>NSExtensionPrincipalClass</key>
       <string>$(PRODUCT_MODULE_NAME).PreviewProvider</string>
     
     - Implement providePreview(for:)
     */
    
    func providePreview(for request: QLFilePreviewRequest) async throws -> QLPreviewReply {
    
        //You can create a QLPreviewReply in several ways, depending on the format of the data you want to return.
        //To return Data of a supported content type:
        
        var appImgData: Data
        
        var appPlistRaw: Data? = UnzipTask(request.fileURL.path, "mergedimage.png")
        if let appPlist = appPlistRaw {
            appImgData = appPlist
        } else {
            appPlistRaw = UnzipTask(request.fileURL.path, "preview.png");
            if let appPlist = appPlistRaw {
                appImgData = appPlist
            } else {
                throw CocoaError.error(CocoaError.fileReadNoSuchFile)
            }
        }
        
        let appIcon = NSImage.init(data: appImgData)
        let rep = appIcon!.representations.first

#if DEBUG
        NSLog("krita-preview loaded file: \(request.fileURL.path)")
        for representation in appIcon!.representations {
            if let bitmapRep = representation as? NSBitmapImageRep {
                NSLog("krita-preview bmp: \(bitmapRep.pixelsWide)x\(bitmapRep.pixelsHigh)")
                // Access bitmap data, properties, etc.
            } else {
                NSLog("krita-preview: no representations")
            }
        }
#endif
        
        let contentType = UTType.png
        
        let reply = QLPreviewReply.init(dataOfContentType: contentType, contentSize: CGSize.init(width: rep!.pixelsWide, height: rep!.pixelsHigh)) { (replyToUpdate : QLPreviewReply) in
            return appImgData
        }
        
        return reply
        

        /*
        var appPlist = UnzipTask(request.fileURL.path, "mergedimage.png")
        // Fallback to use preview is mergedimage is not present
        if (appPlist == nil || appPlist!.isEmpty) {
            appPlist = UnzipTask(request.fileURL.path, "preview.png");
        }
        
        if (appPlist != nil && !appPlist!.isEmpty) {
            let appIcon = NSImage.init(data: appPlist!)
            let rep = appIcon!.representations.first!
            
#if DEBUG
            NSLog("krita-preview loaded file: \(request.fileURL.path)")
            for representation in appIcon!.representations {
                if let bitmapRep = representation as? NSBitmapImageRep {
                    NSLog("krita-preview bmp: \(bitmapRep.pixelsWide)x\(bitmapRep.pixelsHigh)")
                    // Access bitmap data, properties, etc.
                } else {
                    NSLog("krita-preview: no representations")
                }
            }
#endif
            
            
            let contentType = UTType.png
            
            let reply = QLPreviewReply.init(dataOfContentType: contentType, contentSize: CGSize.init(width: rep.pixelsWide, height: rep.pixelsHigh)) { (replyToUpdate : QLPreviewReply) in
                return appPlist!
            }
            return reply
        }
        
        // If nothing has been returned, return default code
        // TODO: return krita generic image.
        
        let contentType = UTType.plainText // replace with your data type
        let reply = QLPreviewReply.init(dataOfContentType: contentType, contentSize: CGSize.init(width: 800, height: 800)) { (replyToUpdate : QLPreviewReply) in

            let data = Data("Hello world".utf8)
            
            //setting the stringEncoding for text and html data is optional and defaults to String.Encoding.utf8
            replyToUpdate.stringEncoding = .utf8
            
            //initialize your data here
            return data
        }
        return reply
         */
    }
}
