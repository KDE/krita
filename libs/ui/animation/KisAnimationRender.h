/*
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISANIMATIONRENDERUTILS_H
#define KISANIMATIONRENDERUTILS_H

#include "KisAnimationRenderingOptions.h"

#include "kritaui_export.h"

class KisDocument;
class KisViewManager;

namespace KisAnimationRender {
    /** Render an animation to file on disk.
     *  returns TRUE on success, FALSE on error or cancellation.
    **/
    KRITAUI_EXPORT bool render(KisDocument *doc, KisViewManager* viewManager, KisAnimationRenderingOptions encoderOptions);

    bool mustHaveEvenDimensions(const QString &mimeType, KisAnimationRenderingOptions::RenderMode renderMode);
    bool hasEvenDimensions(int width, int height);

}

#endif // KISANIMATIONRENDERUTILS_H
