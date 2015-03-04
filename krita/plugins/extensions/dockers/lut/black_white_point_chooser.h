/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __BLACK_WHITE_POINT_CHOOSER_H
#define __BLACK_WHITE_POINT_CHOOSER_H

#include <QFrame>

class KisDoubleSliderSpinBox;


class BlackWhitePointChooser : public QFrame
{
    Q_OBJECT
public:
    BlackWhitePointChooser(QWidget* parent);
    ~BlackWhitePointChooser();

    void showPopup(const QPoint &basePoint);

    qreal blackPoint() const;
    void setBlackPoint(qreal bp);

    qreal whitePoint() const;
    void setWhitePoint(qreal wp);

Q_SIGNALS:
    void sigBlackPointChanged(qreal value);
    void sigWhitePointChanged(qreal value);

private:
    KisDoubleSliderSpinBox *m_black;
    KisDoubleSliderSpinBox *m_white;
};

#endif /* __BLACK_WHITE_POINT_CHOOSER_H */
