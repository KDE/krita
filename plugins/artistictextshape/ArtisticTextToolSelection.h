/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef ARTISTICTEXTTOOLSELECTION_H
#define ARTISTICTEXTTOOLSELECTION_H

#include <KoToolSelection.h>
#include <QPainterPath>

class ArtisticTextShape;
class KoCanvasBase;
class KoViewConverter;
class QPainter;

class ArtisticTextToolSelection : public KoToolSelection
{
public:
    explicit ArtisticTextToolSelection(KoCanvasBase *canvas, QObject *parent = 0);
    virtual ~ArtisticTextToolSelection();

    // reimplemented from KoToolSelection
    virtual bool hasSelection();

    /// Sets the currently selected text shape
    void setSelectedShape(ArtisticTextShape *textShape);

    /// Returns the currently selected text shape
    ArtisticTextShape *selectedShape() const;

    /// Selects specified range of characters
    void selectText(int from, int to);

    /// Returns the start character index of the selection
    int selectionStart() const;

    /// Returns number of selected characters
    int selectionCount() const;

    /// Clears the selection
    void clear();

    /// Paints the selection
    void paint(QPainter &painter, const KoViewConverter &converter);

    /// Triggers a repaint of the selection
    void repaintDecoration();

private:
    /// Returns the outline of the selection in document coordinates
    QPainterPath outline();

    KoCanvasBase *m_canvas;
    ArtisticTextShape *m_currentShape; ///< the currently selected text shape
    int m_selectionStart;
    int m_selectionCount;
};

#endif // ARTISTICTEXTTOOLSELECTION_H
