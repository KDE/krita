/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "reportpropertiesbutton.h"
#include <QPainter>
#include <KLocale>

ReportPropertiesButton::ReportPropertiesButton(QWidget* parent) :  QCheckBox(parent)
{
    setWhatsThis(i18n("A button that allows a report's surface to be selected, allowing its properties to be seen."));
}

void ReportPropertiesButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QPen pen(palette().windowText(), 2.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    QColor c(pen.color());
    c.setAlpha(120);
    pen.setColor(c);
    painter.setPen(pen);
    painter.drawRect(4, 4, width() - 8, height() - 8);

    if (isChecked()) {
        painter.fillRect(7, 7, width() - 14, height() - 14, pen.brush());
    }
}

