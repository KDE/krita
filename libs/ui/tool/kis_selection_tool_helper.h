/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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
#ifndef KIS_SELECTION_TOOL_HELPER_H
#define KIS_SELECTION_TOOL_HELPER_H

#include <kritaui_export.h>

#include "kundo2magicstring.h"
#include "kis_layer.h"
#include "kis_selection.h"

class KoShape;
class KisCanvas2;

/**
 * XXX: Doc!
 */
class KRITAUI_EXPORT KisSelectionToolHelper
{
public:

    KisSelectionToolHelper(KisCanvas2* canvas, const KUndo2MagicString& name);
    virtual ~KisSelectionToolHelper();

    void selectPixelSelection(KisPixelSelectionSP selection, SelectionAction action);
    void addSelectionShape(KoShape* shape);
    void addSelectionShapes(QList<KoShape*> shapes);

    void cropRectIfNeeded(QRect *rect);
    void cropPathIfNeeded(QPainterPath *path);

private:
    KisCanvas2* m_canvas;
    KisImageWSP m_image;
    KisLayerSP m_layer;
    KUndo2MagicString m_name;
};


#endif
