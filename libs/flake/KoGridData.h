/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>

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

#ifndef KOGRIDDATA_H
#define KOGRIDDATA_H

#include "flake_export.h"
#include <KoXmlReaderForward.h>
#include <QColor>
#include <QWidget>

class QPainter;
class QRectF;
class KoViewConverter;
class KoXmlWriter;
class KToggleAction;

/**
 * This class stores application-data for display-grids.
 * Things like grid colors, distances and snap to grid are saved here.
 * \sa KoDocument::gridData()
 */
class FLAKE_EXPORT KoGridData
{
public:
    KoGridData();
    ~KoGridData();

    /// return the grid width
    qreal gridX() const;
    /// return the grid height
    qreal gridY() const;
    /**
     * Set the size grid to a new value
     * @param x the width of a grid unit
     * @param y the height of a grid unit
     * @see snapToGrid()
     * @see gridX()
     * @see gridY()
     */
    void setGrid(qreal x, qreal y);

    /**
     * return if snap to grid is enabled.
     * @return if snap to grid is enabled.
     * @see setGrid()
     */
    bool snapToGrid() const;
    /**
     * Set the snap to grid, forcing objects being moved/scaled to the grid.
     * @param on when true, all moving and scaling will be on the grid.
     * @see setGrid()
     */
    void setSnapToGrid(bool on);

    /**
     * return color of grid.
     * @return color of grid.
     * @see setGridColor()
     */
    QColor gridColor() const;

    /**
     * Set the color of grid.
     * @param color the color of grid.
     * @see gridColor()
     */
    void setGridColor(const QColor & color);

    /**
     * return if grid is visible.
     * @return if grid is visible.
     * @see setShowGrid()
     */
    bool showGrid() const;

    /**
     * Set the show grid status.
     * @param showGrid set if grid will be visible.
     * @see showGrid()
     */
    void setShowGrid(bool showGrid);

    /// return if grid is marked to be painted in the background
    bool paintGridInBackground() const;

    /// set if grid is marked to be painted in the background
    void setPaintGridInBackground(bool inBackground);

    /// Loads grid data from the given setting xml document
    bool loadOdfSettings(const KoXmlDocument & settingsDoc);

    /// Saves grid data to the given settings xml writer
    void saveOdfSettings(KoXmlWriter &settingsWriter);

    /**
      * Paint the grid
      * @param painter the painter
      * @param converter the view converter
      * @param area the area in need of updating
     */
    void paintGrid(QPainter &painter, const KoViewConverter &converter, const QRectF &area) const;

    /**
     * Returns a toggle action that can be added to the menu of your views.
     * @param by passing the canvas the toggle action will automatically update the canvas when its toggled.
     */
    KToggleAction *gridToggleAction(QWidget* canvas = 0);

private:
    class Private;
    Private * const d;
};


#endif

