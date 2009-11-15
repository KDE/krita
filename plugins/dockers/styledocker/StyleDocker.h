/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Jan Hambrecht <jaham@gmx.net>
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

#include <KoCanvasObserver.h>
#include <QtGui/QDockWidget>
#include <QtCore/QTime>

class StylePreview;
class StyleButtonBox;
class KoShapeBorderModel;
class KoShapeBorderCommand;
class KoShapeBackground;
class KoShapeBackgroundCommand;
class KoColorBackground;
class KoCanvasBase;
class KoResource;
class KoShape;
class KoColor;
class QToolButton;
class QStackedWidget;
class KoColorPopupAction;
class KoPathShape;
class QSpacerItem;
class QGridLayout;

class StyleDocker : public QDockWidget, public KoCanvasObserver
{
    Q_OBJECT
public:
    explicit StyleDocker(QWidget * parent = 0L);
    virtual ~StyleDocker();

    /// reimplemented from KoCanvasObserver
    virtual void setCanvas(KoCanvasBase *canvas);

private slots:
    void fillSelected();
    void strokeSelected();
    void selectionChanged();
    void selectionContentChanged();
    void resourceChanged(int key, const QVariant&);
    void styleButtonPressed(int buttonId);
    void updateColor(const KoColor &c);
    void updateGradient(KoResource * item);
    void updatePattern(KoResource * item);
    void updateFillRule(Qt::FillRule fillRule);
    /// Called when the docker changes area
    void locationChanged(Qt::DockWidgetArea area);

private:
    void updateColor(const QColor &c, const QList<KoShape*> & selectedShapes);
    /// Sets the shape border and fill to display
    void updateStyle();
    void updateStyle(KoShapeBorderModel * stroke, KoShapeBackground * fill);

    /// Resets color related commands which are used to combine multiple color changes
    void resetColorCommands();

    static KoShapeBackground *applyFillGradientStops(KoShape *shape, const QGradientStops &stops);
    static QBrush applyStrokeGradientStops(KoShape *shape, const QGradientStops &stops);

    /// Returns list of selected path shapes
    QList<KoPathShape*> selectedPathShapes();

    void updateStyleButtons(int activeStyle);

    StylePreview * m_preview;
    StyleButtonBox * m_buttons;
    QStackedWidget * m_stack;
    KoCanvasBase * m_canvas;
    QToolButton * m_colorSelector;
    KoColorPopupAction *m_actionColor;
    QSpacerItem *m_spacer;
    QGridLayout *m_layout;

    QTime m_lastColorChange;
    KoShapeBackgroundCommand * m_lastFillCommand;
    KoShapeBorderCommand * m_lastStrokeCommand;
    KoColorBackground * m_lastColorFill;
    QList<KoShapeBorderModel*> m_lastColorStrokes;
};

#endif // STYLEDOCKER_H
