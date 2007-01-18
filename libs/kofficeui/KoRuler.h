/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>

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
   Boston, MA 02110-1301, USA.
*/

// Description: Ruler (header)

/******************************************************************/

#ifndef koRuler_h
#define koRuler_h

#include <QWidget>

#include <kofficeui_export.h>
#include <KoUnit.h>

class QPainter;
class QPaintEvent;

class KoViewConverter;

class KoRulerPrivate;

/**
 * Decorator widget to draw a single ruler around a canvas.
 */
class KOFFICEUI_EXPORT KoRuler : public QWidget
{
    Q_OBJECT
    public:
        /**
         * Creates a ruler with the orientation @p orientation
         * @param parent parent widget
         * @param orientation the orientation of the ruler
         * @param viewConverter the view converter used to convert from point to pixel
         */
        KoRuler(QWidget* parent, Qt::Orientation orientation, KoViewConverter* viewConverter);
        ~KoRuler();

        /// The ruler's unit
        KoUnit unit() const;

        /// The length of the ruler in points (pt)
        double rulerLength() const;

        /// The orientation of the ruler
        Qt::Orientation orientation() const;

        virtual QSize minimumSizeHint() const;
        virtual QSize sizeHint() const;

    public Q_SLOTS:
        /// Set the unit of the ruler
        void setUnit(KoUnit unit);

        /** Set the offset. Use this function to sync the ruler with
          * the canvas' position on screen
          * @param offset The offset in pixels
          */
        void setOffset(int offset);

        /// Sets the length of the ruler to @p length in points (pt)
        void setRulerLength(double length);

        /** Set the active range, ie the part of the ruler that is most likely used.
          * set to 0, 0 when there is no longer any active range
          * @param start the start of the range in pt
          * @param end the end of the range in pt
          */
        void setActiveRange(double start, double end);

        /** Set wether the ruler should show the current mouse position.
          * Update the position with updateMouseCoordinate().
          * @param show show mouse position if true. Default is false.
          */
        void setShowMousePosition(bool show);

        /** Update the current position of the mouse pointer, repainting if changed.
          * The ruler offset will be applied before painting.
          * @param coordinate Either the x or y coordinate of the mouse depending
          *                   of the orientation of the ruler.
          */
        void updateMouseCoordinate(int coordinate);

        /**
         * Set whether the ruler should show the selection borders
         * @param show show selection borders if true, default is false.
         */
        void setShowSelectionBorders(bool show);

        /**
         * Update the selection borders
         * @param first the first selection border in points
         * @param second the other selection border in points
         */
        void updateSelectionBorders(double first, double second);

    protected:
        virtual void paintEvent(QPaintEvent* event);

        /// @return The step in unit between numbers on the ruler
        double numberStepForUnit() const;

    private:
        KoRulerPrivate* d;
};

#endif
