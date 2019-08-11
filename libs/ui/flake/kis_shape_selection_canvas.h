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

#include <QScopedPointer>
#include <KoCanvasBase.h>

#include <kis_types.h>

class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class KUndo2Command;
class QWidget;
class KoUnit;
class KisShapeController;

/**
 * Dummy canvas just to have a shapemanager for the shape selection
 */
class KisShapeSelectionCanvas : public KoCanvasBase
{
    Q_OBJECT
public:

    KisShapeSelectionCanvas(KoShapeControllerBase *shapeController);
    ~KisShapeSelectionCanvas() override;

    void gridSize(QPointF *offset, QSizeF *spacing) const override;
    bool snapToGrid() const override;
    void addCommand(KUndo2Command *command) override;
    KoShapeManager *shapeManager() const override;
    KoSelectedShapesProxy *selectedShapesProxy() const override;
    void updateCanvas(const QRectF& rc) override;
    KoToolProxy * toolProxy() const override;
    KoViewConverter *viewConverter() const override;
    QWidget* canvasWidget() override;
    const QWidget* canvasWidget() const override;
    KoUnit unit() const override;
    void updateInputMethodInfo() override {}
    void setCursor(const QCursor &) override {}
private:
    QScopedPointer<KoShapeManager> m_shapeManager;
    QScopedPointer<KoSelectedShapesProxy> m_selectedShapesProxy;
};

#endif
