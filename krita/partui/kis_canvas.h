/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_CANVAS_H
#define KIS_CANVAS_H

#include <QWidget>
#include <KoCanvasBase.h>

class KisViewConverter;

/**
 *
 */
class KisCanvas : public QWidget, public KoCanvasBase
{
public:
    KisCanvas(KisViewConverter *, QWidget * canvasWidget);

    virtual ~KisCanvas();

    void setCanvasWidget(QWidget * widget);

public: // KoCanvasBase implementation

    virtual void gridSize(double *horizontal, double *vertical) const;

    virtual bool snapToGrid() const;

    virtual void addCommand(KCommand *command, bool execute = true);

    virtual KoShapeManager *shapeManager() const;

    virtual void updateCanvas(const QRectF& rc);

    virtual KoTool* tool();

    virtual void setTool(KoTool *tool);

    virtual KoViewConverter *viewConverter();

    virtual QWidget* canvasWidget();

    virtual KoUnit::Unit unit();

private:
    KisViewConverter * m_viewConverter;
    QWidget * m_canvasWidget;

};

#endif
