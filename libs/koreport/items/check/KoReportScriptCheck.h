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

    class Check : public QObject
    {
        Q_OBJECT
        public:
            explicit Check(KoReportItemCheck *);

            ~Check();

        public slots:
            bool value();
            void setValue(bool val = true);

            /**Gets/sets the check style eg, Cross, Tick Dot*/
            QString checkStyle();
            void setCheckStyle(const QString&);

            QColor foregroundColor();
            void setForegroundColor(const QColor&);

            QColor lineColor();
            void setLineColor(const QColor&);

            int lineWeight();
            void setLineWeight(int);

            /**Gets/sets the line style.  Valid values are those from Qt::PenStyle (0-5)*/
            int lineStyle();
            void setLineStyle(int);

            QPointF position();
            void setPosition(const QPointF&);

            QSizeF size();
            void setSize(const QSizeF&);

        private:
            KoReportItemCheck *m_check;
            };

}
#endif // KOREPORTSCRIPTCHECK_H
