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

#ifndef KIMAGEFRAME_H
#define KIMAGEFRAME_H

#include <QtGui/QFrame>

class KImageFrame : public QFrame
{
    Q_OBJECT
public:
    KImageFrame(QWidget* parent = 0);
    virtual ~KImageFrame() {}

public Q_SLOTS:
    void setImage(const QImage&);

protected Q_SLOTS:
    void paintEvent(QPaintEvent*);

protected:
    QImage _image;
    int _w, _h;
};

#endif
// kate: hl C++; indent-width 4; replace-tabs on;
