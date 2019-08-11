/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef SELECTIONDECORATOR_H
#define SELECTIONDECORATOR_H

#include <KoViewConverter.h>
#include <KoFlake.h>

#include <QPainter>
#include <QPointer>

class KoSelection;
class KoCanvasResourceProvider;

static const struct DecoratorIconPositions {
    QPoint uiOffset = QPoint(0, 40);
} decoratorIconPositions;

/**
 * The SelectionDecorator is used to paint extra user-interface items on top of a selection.
 */
class SelectionDecorator
{
public:
    /**
     * Constructor.
     * @param arrows the direction that needs highlighting. (currently unused)
     * @param rotationHandles if true; the rotation handles will be drawn
     * @param shearHandles if true; the shearhandles will be drawn
     */
    SelectionDecorator(KoCanvasResourceProvider *resourceManager);
    ~SelectionDecorator() {}

    /**
     * paint the decortations.
     * @param painter the painter to paint to.
     * @param converter to convert between internal and view coordinates.
     */
    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * set the selection that is to be painted.
     * @param selection the current selection.
     */
    void setSelection(KoSelection *selection);

    /**
     * set the radius of the selection handles
     * @param radius the new handle radius
     */
    void setHandleRadius(int radius);

    /**
     * Set true if you want to render gradient handles on the canvas.
     * Default value: false
     */
    void setShowFillGradientHandles(bool value);

    /**
     * Set true if you want to render gradient handles on the canvas.
     * Default value: false
     */
    void setShowStrokeFillGradientHandles(bool value);

    void setForceShapeOutlines(bool value);

private:
    void paintGradientHandles(KoShape *shape, KoFlake::FillVariant fillVariant, QPainter &painter, const KoViewConverter &converter);

private:
    KoFlake::AnchorPosition m_hotPosition;
    KoSelection *m_selection;
    int m_handleRadius;
    int m_lineWidth;
    bool m_showFillGradientHandles;
    bool m_showStrokeFillGradientHandles;
    bool m_forceShapeOutlines;
};

#endif
