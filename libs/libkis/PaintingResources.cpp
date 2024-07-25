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
#include "KisToolShapeUtils.h"


const QStringList StrokeStyle = {
    "None",             // 0 = KisToolShapeUtils::StrokeStyle::StrokeStyleNone
    "ForegroundColor",  //     KisToolShapeUtils::StrokeStyle::StrokeStyleForeground
    "BackgroundColor"   //     KisToolShapeUtils::StrokeStyle::StrokeStyleBackground
};

const QStringList FillStyle = {
    "None",             // 0 = KisToolShapeUtils::FillStyle::FillStyleNone
    "ForegroundColor",  //     KisToolShapeUtils::FillStyle::FillStyleForegroundColor
    "BackgroundColor",  //     KisToolShapeUtils::FillStyle::FillStyleBackgroundColor
    "Pattern"           //     KisToolShapeUtils::FillStyle::FillStylePattern
};

KisFigurePaintingToolHelper PaintingResources::createHelper(KisImageWSP image,
                                                            const QString strokeStyleString,
                                                            const QString fillStyleString)
{
    // need to grab the resource provider
    KisView *activeView = KisPart::instance()->currentMainwindow()->activeView();
    KoCanvasResourceProvider *resourceManager = activeView->viewManager()->canvasResourceProvider()->resourceManager();

    // grab the image and current layer
    KisNodeSP node = activeView->currentNode();

    int strokeIndex = StrokeStyle.indexOf(strokeStyleString);
    if (strokeIndex == -1) {
        dbgScript << "Script tried to paint with invalid strokeStyle" << strokeStyleString << ", ignoring and using" << defaultStrokeStyle << ".";
        strokeIndex = StrokeStyle.indexOf(defaultStrokeStyle);
        if (strokeIndex == -1) {
            warnScript << "PaintingResources::createHelper(): defaultStrokeStyle" << defaultStrokeStyle << "is invalid!";
            strokeIndex = 1;
        }
    }
    KisToolShapeUtils::StrokeStyle strokeStyle = (KisToolShapeUtils::StrokeStyle) strokeIndex;

    int fillIndex = FillStyle.indexOf(fillStyleString);
    if (fillIndex == -1) {
        dbgScript << "Script tried to paint with invalid fillStyle" << fillStyleString << ", ignoring and using" << defaultFillStyle << ".";
        fillIndex = FillStyle.indexOf(defaultFillStyle);
        if (fillIndex == -1) {
            warnScript << "PaintingResources::createHelper(): defaultFillStyle" << defaultFillStyle << " is invalid!";
            fillIndex = 0;
        }
    }
    KisToolShapeUtils::FillStyle fillStyle = (KisToolShapeUtils::FillStyle) fillIndex;

    const KUndo2MagicString name = kundo2_i18n("Scripted Brush Stroke");
    KisFigurePaintingToolHelper helper(
        name,
        image,
        node, resourceManager,
        strokeStyle,
        fillStyle
    );

    return helper;
}
