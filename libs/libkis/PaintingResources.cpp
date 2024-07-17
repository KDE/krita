/*
 *  SPDX-FileCopyrightText: 2020 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "PaintingResources.h"

#include "KisView.h"
#include "KisPart.h"
#include "kis_canvas_resource_provider.h"
#include "KisViewManager.h"
#include "KisMainWindow.h"
#include "kis_image.h"


KisFigurePaintingToolHelper PaintingResources::createHelper(KisImageWSP image)
{
    // need to grab the resource provider
    KisView *activeView = KisPart::instance()->currentMainwindow()->activeView();
    KoCanvasResourceProvider *resourceManager = activeView->viewManager()->canvasResourceProvider()->resourceManager();

    // grab the image and current layer
    KisNodeSP node = activeView->currentNode();

    const KUndo2MagicString name = kundo2_noi18n("python_stroke");
    KisFigurePaintingToolHelper helper(
        name,
        image,
        node, resourceManager,
        KisToolShapeUtils::StrokeStyle::StrokeStyleForeground,
        KisToolShapeUtils::FillStyle::FillStyleNone
    );

    return helper;
}
