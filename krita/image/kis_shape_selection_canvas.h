/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SHAPE_SELECTION_CANVAS_H
#define KIS_SHAPE_SELECTION_CANVAS_H

#include <KoCanvasBase.h>
#include <kis_types.h>

#include "kis_selection_shape_manager.h"

class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class QUndoCommand;
class QWidget;
class KoUnit;

/**
 * KisShapeSelectionCanvas is container for KisSelectionShapeManager, nothing else
 * TODO remove if possible
 */
class KisShapeSelectionCanvas : public QObject, public KoCanvasBase
{
    Q_OBJECT
public:

    KisShapeSelectionCanvas(KisSelection *selection, KoViewConverter * viewConverter);
    virtual ~KisShapeSelectionCanvas();

    void gridSize(double *horizontal, double *vertical) const;
    bool snapToGrid() const;
    void addCommand(QUndoCommand *command);
    KoShapeManager *shapeManager() const;
    void updateCanvas(const QRectF& rc) {}
    KoToolProxy * toolProxy() const;
    const KoViewConverter *viewConverter() const;
    QWidget* canvasWidget();
    KoUnit unit() const;
    virtual void updateInputMethodInfo() {}

private:

    KoViewConverter * m_viewConverter;
    KisSelectionShapeManager * m_shapeManager;
};

#endif
