/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_processing_applicator.h"

class KoShape;

/**
 * XXX: Doc!
 */
class KRITAUI_EXPORT KisSelectionToolHelper
{
public:

    KisSelectionToolHelper(KisCanvas2* canvas, const KUndo2MagicString& name);
    virtual ~KisSelectionToolHelper();

    void selectPixelSelection(KisProcessingApplicator& applicator, KisPixelSelectionSP selection, SelectionAction action);
    void selectPixelSelection(KisPixelSelectionSP selection, SelectionAction action);

    void addSelectionShape(KoShape* shape, SelectionAction action = SELECTION_DEFAULT);
    void addSelectionShapes(QList<KoShape*> shapes, SelectionAction action = SELECTION_DEFAULT);

    bool canShortcutToDeselect(const QRect &rect, SelectionAction action);
    bool canShortcutToNoop(const QRect &rect, SelectionAction action);

    bool tryDeselectCurrentSelection(const QRectF selectionViewRect, SelectionAction action);
    static QMenu* getSelectionContextMenu(KisCanvas2* canvas);

    SelectionMode tryOverrideSelectionMode(KisSelectionSP activeSelection, SelectionMode currentMode, SelectionAction currentAction) const;


private:
    QPointer<KisCanvas2> m_canvas;
    KisImageSP m_image;
    KisLayerSP m_layer;
    KUndo2MagicString m_name;
};


#endif
