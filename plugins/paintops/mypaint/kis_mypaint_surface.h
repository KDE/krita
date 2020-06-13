#ifndef KIS_MYPAINT_SURFACE_H
#define KIS_MYPAINT_SURFACE_H

#include <QObject>

#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_lod_transform.h>
#include <KoColor.h>
#include <kis_marker_painter.h>

#include <libmypaint/mypaint-brush.h>
#include <libmypaint/mypaint-surface.h>

class KisMyPaintSurface
{
public:
    KisMyPaintSurface(KisPainter* painter, KisNodeSP node=nullptr);

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

    static inline float
    calculate_rr_antialiased (int  xp, int  yp, float x, float y, float aspect_ratio,
                              float sn, float cs, float one_over_radius2, float r_aa_start);

    static inline float
    calculate_alpha_for_rr (float rr, float hardness, float slope1, float slope2);

    static inline float
    calculate_rr (int xp, int yp, float x, float y, float aspect_ratio,
                  float sn, float cs, float one_over_radius2);


    static KisPainter* painter();
    static void paint(KoColor *color, KoColor* bgColor);
    static qreal calculateOpacity(float angle, float hardness, float opaque, float x, float y,
                                            float xp, float yp, float aspect_ratio, float radius);

    MyPaintSurface* surface();

private:
    static KisPainter *m_painter;
    static KisNodeSP m_node;
    MyPaintSurface *m_surface;
};

#endif // KIS_MYPAINT_SURFACE_H
