/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KARBONGRADIENTTOOL_H
#define KARBONGRADIENTTOOL_H

#include <KoToolBase.h>
#include <KoSnapGuide.h>
#include <QGradient>
#include <QMultiMap>

class GradientStrategy;
class KoGradientEditWidget;
class KUndo2Command;
class KoShape;
class KoResource;

/**
 * A tool for editing gradient backgrounds of shapes.
 * The gradients can be edited by moving gradient
 * handles directly on the canvas.
 */
class KarbonGradientTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit KarbonGradientTool(KoCanvasBase *canvas);
    ~KarbonGradientTool() override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void repaintDecorations() override;

    void mousePressEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void activate(ToolActivation toolActivation, const QSet<KoShape *> &shapes) override;
    void deactivate() override;

public Q_SLOTS:
    void documentResourceChanged(int key, const QVariant &res) override;

protected:
    /// reimplemented from KoToolBase
    QList<QPointer<QWidget> > createOptionWidgets() override;

private Q_SLOTS:
    void initialize();
    void gradientChanged();
    void gradientSelected(KoResource *);
private:
    QGradient *m_gradient;
    QMultiMap<KoShape *, GradientStrategy *> m_strategies; ///< the list of gradient strategies
    GradientStrategy *m_currentStrategy;   ///< the current editing strategy
    GradientStrategy *m_hoverStrategy;  ///< the strategy the mouse hovers over
    KoGradientEditWidget *m_gradientWidget;
    KUndo2Command *m_currentCmd;
    KoSnapGuide::Strategies m_oldSnapStrategies; ///< the previously enables snap strategies
};

#endif // KARBONGRADIENTTOOL_H
