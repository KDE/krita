/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_DROPSHADOW_H_
#define _KIS_DROPSHADOW_H_

#include <kis_progress_subject.h>
#include <kis_paint_device.h>

typedef enum
{
  BLUR_IIR,
  BLUR_RLE
} BlurMethod;


class QColor;
class KisView;
class KisProgressDisplayInterface;

class KisDropshadow : public KisProgressSubject {

    Q_OBJECT

public:

    KisDropshadow(KisView * view);
    virtual ~KisDropshadow() {};

    void dropshadow(KisProgressDisplayInterface * progress, Q_INT32 xoffset, Q_INT32 yoffset, Q_INT32 blurradius, QColor color, Q_UINT8 opacity, bool allowResize);

public: // Implement KisProgressSubject
        virtual void cancel() { m_cancelRequested = true; }

private:
    void gaussianblur (KisPaintDeviceSP src, KisPaintDeviceSP dst,
                       QRect& rect, double horz, double vert,
                       BlurMethod method,
                       KisProgressDisplayInterface * progressDisplay);
    //gaussian blur helper functions
    void find_constants(double n_p[], double n_m[], double d_p[], double d_m[], double bd_p[], double bd_m[], double  std_dev);
    void transfer_pixels(double *src1, double *src2, Q_UINT8  *dest, Q_INT32 bytes, Q_INT32 width);
    Q_INT32* make_curve(double sigma, Q_INT32 *length);
    void run_length_encode (Q_UINT8 *src, Q_INT32 *dest, Q_INT32 bytes, Q_INT32 width);
    void multiply_alpha (Q_UINT8 *buf, Q_INT32 width, Q_INT32 bytes);
    void separate_alpha (Q_UINT8 *buf, Q_INT32 width, Q_INT32 bytes);

private:
    KisView * m_view;
    bool m_cancelRequested;

};

#endif
