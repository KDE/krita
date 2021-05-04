/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MYPAINT_SURFACE_H
#define KIS_MYPAINT_SURFACE_H

#include <QObject>

#include <kis_paint_device.h>
#include <kis_fixed_paint_device.h>
#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_lod_transform.h>
#include <KoColor.h>
#include <kis_marker_painter.h>
#include <kis_sequential_iterator.h>
#include <KisPrecisePaintDeviceWrapper.h>

#include <libmypaint/mypaint-brush.h>
#include <libmypaint/mypaint-surface.h>

class KisMyPaintSurface
{
public:

    struct MyPaintSurfaceInternal: public MyPaintSurface {
          KisMyPaintSurface *m_owner;
          KoChannelInfo::enumChannelValueType bitDepth;
    };

public:
    KisMyPaintSurface(KisPainter* painter, KisPaintDeviceSP paintNode=nullptr, KisImageSP image = nullptr);
    ~KisMyPaintSurface();

    /**
      * mypaint_surface_draw_dab:
      *
      * Draw a dab onto the surface.
      */
    static int draw_dab(MyPaintSurface *self, float x, float y, float radius,
                           float color_r, float color_g, float color_b, float opaque, float hardness,
                           float color_a, float aspect_ratio, float angle, float lock_alpha,
                           float colorize);

    static void get_color(MyPaintSurface *self, float x, float y, float radius,
                            float * color_r, float * color_g, float * color_b, float * color_a);

    template <typename channelType>
    int drawDabImpl(MyPaintSurface *self, float x, float y, float radius, float color_r, float color_g,
                                    float color_b, float opaque, float hardness, float color_a,
                                    float aspect_ratio, float angle, float lock_alpha, float colorize);

    template <typename channelType>
    void getColorImpl(MyPaintSurface *self, float x, float y, float radius,
                                float * color_r, float * color_g, float * color_b, float * color_a);

    inline float
    calculate_rr_antialiased (int  xp, int  yp, float x, float y, float aspect_ratio,
                              float sn, float cs, float one_over_radius2, float r_aa_start);

    inline float
    calculate_alpha_for_rr (float rr, float hardness, float slope1, float slope2);

    inline float
    calculate_rr (int xp, int yp, float x, float y, float aspect_ratio,
                  float sn, float cs, float one_over_radius2);


    KisPainter* painter();
    void paint(KoColor *color, KoColor* bgColor);
    qreal calculateOpacity(float angle, float hardness, float opaque, float x, float y,
                                            float xp, float yp, float aspect_ratio, float radius);

    MyPaintSurface* surface();

private:
    KisPainter *m_painter;
    KisPaintDeviceSP m_imageDevice;
    MyPaintSurfaceInternal *m_surface;
    KisImageSP m_image;
    KisPrecisePaintDeviceWrapper m_precisePainterWrapper;
    KisPaintDeviceSP m_dab;
    QScopedPointer<KisPainter> m_tempPainter;
    QScopedPointer<KisPainter> m_backgroundPainter;
    KisFixedPaintDeviceSP m_blendDevice;
    KisFixedPaintDeviceSP m_maskDevice;

};

#endif // KIS_MYPAINT_SURFACE_H
