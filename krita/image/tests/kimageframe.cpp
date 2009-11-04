/*
 * Copyright 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kimageframe.h"
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QPainter>

KImageFrame::KImageFrame(QWidget *parent) : QFrame(parent), _w(0), _h(0)
{
}

void KImageFrame::setImage(const QImage &img)
{
    _img = img;
    _w = img.width();
    _h = img.height();
    update();
}

void KImageFrame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QStyleOptionFrame opt;
    QRect rf(frameRect()), ri(0, 0, _w, _h);

    opt.rect = rf;
    opt.state = QStyle::State_Sunken;

    style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);

    ri.moveCenter(rf.center());
    p.drawImage(ri, _img);

    p.end();
}

#include "kimageframe.moc"
// kate: hl C++; indent-width 4; replace-tabs on;
