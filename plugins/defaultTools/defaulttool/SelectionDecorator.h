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

class KoSelection;

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
    SelectionDecorator(KoFlake::SelectionHandle arrows, bool rotationHandles, bool shearHandles);
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

    /// Sets the hot position to highlight
    static void setHotPosition(KoFlake::Position hotPosition);

    /// Returns the hot position
    static KoFlake::Position hotPosition();

private:
    bool m_rotationHandles, m_shearHandles;
    KoFlake::SelectionHandle m_arrows;
    static KoFlake::Position m_hotPosition;
    KoSelection *m_selection;
    int m_handleRadius;
    int m_lineWidth;
};

#endif
