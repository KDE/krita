/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_AUTOBRUSH_RESOURCE_H_
#define _KIS_AUTOBRUSH_RESOURCE_H_

#include "kis_brush.h"

class KisAutobrushShape {
    public:
        KisAutobrushShape(Q_INT32 w, Q_INT32 h, double fh, double fv) : m_w(w), m_h(h), m_fh(fh), m_fv(fv)
        { };
        void createBrush( QImage* img);
    protected:
        virtual Q_INT8 valueAt(Q_INT32 x, Q_INT32 y) =0;
        Q_INT32 m_w, m_h;
        double m_fh, m_fv;
};

class KisAutobrushCircleShape : public KisAutobrushShape {
    public:
        KisAutobrushCircleShape(Q_INT32 w, Q_INT32 h, double fh, double fv);
    public:
        virtual Q_INT8 valueAt(Q_INT32 x, Q_INT32 y);
    private:
        double norme(double a, double b)
        {
            return a*a + b * b;
        }
    private:
        double m_xcentre, m_ycentre;
        double m_xcoef, m_ycoef;
        double m_xfadecoef, m_yfadecoef;
};

class KisAutobrushRectShape : public KisAutobrushShape {
    public:
        KisAutobrushRectShape(Q_INT32 w, Q_INT32 h, double fh, double fv);
    protected:
        virtual Q_INT8 valueAt(Q_INT32 x, Q_INT32 y);
    private:
        double m_xcentre, m_ycentre, m_c;
};

class KisAutobrushResource : public KisBrush
{
    public:
        KisAutobrushResource(QImage& img) : KisBrush("")
        {
            setImage(img);
            setBrushType(MASK);
        };
    public:
        virtual bool load() { return false; };
};
#endif // _KIS_AUTOBRUSH_RESOURCE_H_
