/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Frederic Coiffier <fcoiffie@gmail.com>
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

#ifndef KGRADIENTSLIDER_H
#define KGRADIENTSLIDER_H

// Qt includes.

#include <qwidget.h>
#include <qcolor.h>
#include <qptrlist.h>
#include <qpair.h>

class KGradientSlider : public QWidget
{
Q_OBJECT

    typedef enum {
        BlackCursor,
        GammaCursor,
        WhiteCursor
    } eCursor;

public:
    KGradientSlider(QWidget *parent = 0, const char *name = 0, WFlags f = 0);

    virtual ~KGradientSlider();

public slots:
    void modifyBlack(int);
    void modifyWhite(int);
    void modifyGamma(double);

signals:

    void modifiedBlack(int);
    void modifiedWhite(int);
    void modifiedGamma(double);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent (QMouseEvent * e);
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    void leaveEvent ( QEvent * );

public:
    void enableGamma(bool b);
    double getGamma(void);

private:
    unsigned int m_leftmost;
    unsigned int m_rightmost;
    eCursor m_grab_cursor;
    unsigned int m_grab_index;
    bool m_dragging;

    unsigned int m_blackcursor;
    unsigned int m_whitecursor;
    unsigned int m_gammacursor;

    bool m_gammaEnabled;
    double m_gamma;
};


#endif /* KGRADIENTSLIDER_H */
