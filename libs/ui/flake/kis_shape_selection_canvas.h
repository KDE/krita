/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
