/*
 * KoReport Lirary
 * Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KOREPORTSCRIPTCHECK_H
#define KOREPORTSCRIPTCHECK_H

#include <QObject>
#include "KoReportItemCheck.h"

namespace Scripting
{
    /**
    @brief Check (box) script interface

    The user facing interface for scripting report checkbox items
    */
    class Check : public QObject
    {
        Q_OBJECT
        public:
            explicit Check(KoReportItemCheck *);

            ~Check();

        public Q_SLOTS:

            //! @returns the value of the checkbox as a boolean
            bool value() const;

            //! Sets the value of the checkbox.
            //! Defaults to true if no value given
            void setValue(bool val = true);

            //! @returns the style of the checkbox as a string
            //! Possible values are Cross, Tick, Dot
            QString checkStyle() const;

            //! Sets the style of the checkbox to one of
            //! Cross, Tick, Dot
            void setCheckStyle(const QString&);

            //! @returns the foreground color of the checkbox
            QColor foregroundColor() const;

            //! Sets the foreground color of the checkbox to the given color
            void setForegroundColor(const QColor&);

            //! @returns the line color of the checkbox
            QColor lineColor() const;

            //! Sets the line color of the checkbox to the given color
            void setLineColor(const QColor&);

            //! @returns the line weight (width) of the checkbox
            int lineWeight() const;

            //! Sets the line weight (width) of the checkbox
            void setLineWeight(int);

            //! @return the border line style of the label.  Values are from Qt::Penstyle range 0-5
            int lineStyle() const;

            //! Sets the border line style of the label to the given style in the range 0-5
            void setLineStyle(int);

            //! @returns the position of the label in points
            QPointF position() const;

            //! Sets the position of the label to the given point coordinates
            void setPosition(const QPointF&);

            //! @returns the size of the label in points
            QSizeF size() const;

            //! Sets the size of the label to the given size in points
            void setSize(const QSizeF&);


        private:
            KoReportItemCheck *m_check;
            };

}
#endif // KOREPORTSCRIPTCHECK_H
