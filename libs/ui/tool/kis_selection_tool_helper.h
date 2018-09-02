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
#include <QMenu>
#include <QPointer>

#include "kundo2magicstring.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_canvas2.h"

class KoShape;

/**
 * XXX: Doc!
 */
class KRITAUI_EXPORT KisSelectionToolHelper
{
public:

    KisSelectionToolHelper(KisCanvas2* canvas, const KUndo2MagicString& name);
    virtual ~KisSelectionToolHelper();

    void selectPixelSelection(KisPixelSelectionSP selection, SelectionAction action);
    void addSelectionShape(KoShape* shape, SelectionAction action = SELECTION_DEFAULT);
    void addSelectionShapes(QList<KoShape*> shapes, SelectionAction action = SELECTION_DEFAULT);

    bool canShortcutToDeselect(const QRect &rect, SelectionAction action);
    bool canShortcutToNoop(const QRect &rect, SelectionAction action);

    bool tryDeselectCurrentSelection(const QRectF selectionViewRect, SelectionAction action);
    static QMenu* getSelectionContextMenu(KisCanvas2* canvas);


private:
    QPointer<KisCanvas2> m_canvas;
    KisImageSP m_image;
    KisLayerSP m_layer;
    KUndo2MagicString m_name;
};


#endif
