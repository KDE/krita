/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISANIMATIONRENDERUTILS_H
#define KISANIMATIONRENDERUTILS_H

#include "KisAnimationRenderingOptions.h"

#include "kritaui_export.h"

class KisDocument;
class KisViewManager;

namespace KisAnimationRender {

    KRITAUI_EXPORT void render(KisDocument *doc, KisViewManager* viewManager, KisAnimationRenderingOptions encoderOptions);

    QString getNameForFrame(const QString &basename, const QString &extension, int sequenceStart, int frame);

    QStringList getNamesForFrames(const QString &basename, const QString &extension, int sequenceStart, const QList<int> &frames);

    bool mustHaveEvenDimensions(const QString &mimeType, KisAnimationRenderingOptions::RenderMode renderMode);
    bool hasEvenDimensions(int width, int height);

}

#endif // KISANIMATIONRENDERUTILS_H
