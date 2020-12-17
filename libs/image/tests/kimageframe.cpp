/*
 * SPDX-FileCopyrightText: 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kimageframe.h"
#include <QStyle>
#include <QStyleOption>
#include <QPainter>

KImageFrame::KImageFrame(QWidget *parent) : QFrame(parent), _w(0), _h(0)
{
}

void KImageFrame::setImage(const QImage &image)
{
    _image = image;
    _w = image.width();
    _h = image.height();
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
    p.drawImage(ri, _image);

    p.end();
}

// kate: hl C++; indent-width 4; replace-tabs on;
