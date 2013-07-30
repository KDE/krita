/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef STYLEDOCKER_H
#define STYLEDOCKER_H

// Base classes
#include <QDockWidget>
#include <KoCanvasObserverBase.h>

// Qt
#include <QTime>
#include <QList>
#include <QSharedPointer>

class StrokeFillWidget;
class KoShapeStrokeModel;
class KoShapeStrokeCommand;
class KoShapeBackground;
class KoShapeBackgroundCommand;
class KoColorBackground;

class KoCanvasBase;
class KoResource;
class KoShape;
class KoColor;
class KoPathShape;


class StyleDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    explicit StyleDocker(QWidget * parent = 0L);
    virtual ~StyleDocker();

    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
    
private slots:
    // Slots connected to the canvas
    void selectionChanged();
    void selectionContentChanged();
    void canvasResourceChanged(int key, const QVariant&);

    // Slots connected from the widget.
    void aspectSelected(int aspect);
    void noColorSelected();
    void updateColor(const KoColor &c);
    void updateGradient(KoResource *item);
    void updatePattern(KoResource *item);
    void updateFillRule(Qt::FillRule fillRule);
    void updateOpacity(qreal opacity);

    /// Called when the docker changes area
    void locationChanged(Qt::DockWidgetArea area);

 private:
    void updateColor(const QColor &c, const QList<KoShape*> & selectedShapes);
    /// Sets the shape stroke and fill to display
    void updateWidget();
    void updateWidget(KoShapeStrokeModel * stroke, QSharedPointer<KoShapeBackground> fill, int opacity);

    /// Resets color related commands which are used to combine multiple color changes
    void resetColorCommands();

    static QSharedPointer<KoShapeBackground> applyFillGradientStops(KoShape *shape, const QGradientStops &stops);
    static QBrush applyStrokeGradientStops(KoShape *shape, const QGradientStops &stops);

    /// Returns list of selected path shapes
    QList<KoPathShape*> selectedPathShapes();

    // ----------------------------------------------------------------

    KoCanvasBase     *m_canvas;
    StrokeFillWidget *m_mainWidget;
    
    QTime                      m_lastColorChange;
    KoShapeBackgroundCommand  *m_lastFillCommand;
    KoShapeStrokeCommand      *m_lastStrokeCommand;
    QSharedPointer<KoColorBackground> m_lastColorFill;
    QList<KoShapeStrokeModel*> m_lastColorStrokes;
};

#endif // STYLEDOCKER_H
