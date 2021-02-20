/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintSurface.h"

#include <KoColorConversions.h>
#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <QtMath>
#include <kis_algebra_2d.h>
#include <kis_cross_device_color_sampler.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_sequential_iterator.h>
#include <kis_selection.h>
#include <qmath.h>
#include <KoCompositeOpRegistry.h>
#include <KoMixColorsOp.h>

using namespace std;

void destroy_internal_surface_callback(MyPaintSurface *surface)
{
    KisMyPaintSurface::MyPaintSurfaceInternal *ptr = static_cast<KisMyPaintSurface::MyPaintSurfaceInternal*>(surface);
    delete ptr;
}

KisMyPaintSurface::KisMyPaintSurface(KisPainter *painter, KisPaintDeviceSP paintNode, KisImageSP image)
    : m_painter(painter)
    , m_imageDevice(paintNode)
    , m_image(image)
    , m_precisePainterWrapper(painter->device())
    , m_dab(m_precisePainterWrapper.createPreciseCompositionSourceDevice())
    , m_tempPainter(new KisPainter(m_precisePainterWrapper.preciseDevice()))
    , m_backgroundPainter(new KisPainter(m_precisePainterWrapper.createPreciseCompositionSourceDevice()))

{
    m_backgroundPainter->setCompositeOp(COMPOSITE_COPY);
    m_backgroundPainter->setOpacity(OPACITY_OPAQUE_U8);
    m_tempPainter->setCompositeOp(COMPOSITE_COPY);
    m_tempPainter->setSelection(painter->selection());
    m_tempPainter->setChannelFlags(painter->channelFlags());
    m_tempPainter->copyMirrorInformationFrom(painter);
    m_surface = new MyPaintSurfaceInternal();
    mypaint_surface_init(m_surface);
    m_surface->m_owner = this;

    m_surface->draw_dab = this->draw_dab;
    m_surface->get_color = this->get_color;
    m_surface->destroy = destroy_internal_surface_callback;
    m_surface->bitDepth = m_precisePainterWrapper.preciseColorSpace()->channels()[0]->channelValueType();
}

KisMyPaintSurface::~KisMyPaintSurface()
{
    mypaint_surface_unref(m_surface);
}

int KisMyPaintSurface::draw_dab(MyPaintSurface *self, float x, float y, float radius, float color_r, float color_g,
                                float color_b, float opaque, float hardness, float color_a,
                                float aspect_ratio, float angle, float lock_alpha, float colorize) {

    MyPaintSurfaceInternal *surface = static_cast<MyPaintSurfaceInternal*>(self);

    if (surface->bitDepth == KoChannelInfo::UINT8) {
        return surface->m_owner->drawDabImpl<quint8>(self, x, y, radius, color_r, color_g,
                 color_b, opaque, hardness, color_a,
                aspect_ratio, angle, lock_alpha,  colorize);
    }
    else if (surface->bitDepth == KoChannelInfo::UINT16) {
        return surface->m_owner->drawDabImpl<quint16>(self, x, y, radius, color_r, color_g,
                 color_b, opaque, hardness, color_a,
                aspect_ratio, angle, lock_alpha,  colorize);
    }
#if defined HAVE_OPENEXR
    else if (surface->bitDepth == KoChannelInfo::FLOAT16) {
        return surface->m_owner->drawDabImpl<half>(self, x, y, radius, color_r, color_g,
                 color_b, opaque, hardness, color_a,
                aspect_ratio, angle, lock_alpha,  colorize);
    }
#endif
    else {
        return surface->m_owner->drawDabImpl<float>(self, x, y, radius, color_r, color_g,
                 color_b, opaque, hardness, color_a,
                aspect_ratio, angle, lock_alpha,  colorize);
    }
}

void KisMyPaintSurface::get_color(MyPaintSurface *self, float x, float y, float radius,
                            float * color_r, float * color_g, float * color_b, float * color_a) {

    MyPaintSurfaceInternal *surface = static_cast<MyPaintSurfaceInternal*>(self);
    if (surface->bitDepth == KoChannelInfo::UINT8) {
        surface->m_owner->getColorImpl<quint8>(self, x, y, radius, color_r, color_g, color_b, color_a);
    }
    else if (surface->bitDepth == KoChannelInfo::UINT16) {
        surface->m_owner->getColorImpl<quint16>(self, x, y, radius, color_r, color_g, color_b, color_a);
    }
#if defined HAVE_OPENEXR
    else if (surface->bitDepth == KoChannelInfo::FLOAT16) {
        surface->m_owner->getColorImpl<half>(self, x, y, radius, color_r, color_g, color_b, color_a);
    }
#endif
    else {
        surface->m_owner->getColorImpl<float>(self, x, y, radius, color_r, color_g, color_b, color_a);
    }
}


/*GIMP's draw_dab and get_color code*/
template <typename channelType>
int KisMyPaintSurface::drawDabImpl(MyPaintSurface *self, float x, float y, float radius, float color_r, float color_g,
                                float color_b, float opaque, float hardness, float color_a,
                                float aspect_ratio, float angle, float lock_alpha, float colorize) {

    Q_UNUSED(self);
    const float one_over_radius2 = 1.0f / (radius * radius);
    const double angle_rad = kisDegreesToRadians(angle);
    const float cs = cos(angle_rad);
    const float sn = sin(angle_rad);
    float normal_mode;
    float segment1_slope;
    float segment2_slope;
    float r_aa_start;

    hardness = CLAMP (hardness, 0.0f, 1.0f);
    segment1_slope = -(1.0f / hardness - 1.0f);
    segment2_slope = -hardness / (1.0f - hardness);
    aspect_ratio = max(1.0f, aspect_ratio);

    r_aa_start = radius - 1.0f;
    r_aa_start = max(r_aa_start, 0.0f);
    r_aa_start = (r_aa_start * r_aa_start) / aspect_ratio;

    normal_mode = opaque * (1.0f - colorize);
    colorize = opaque * colorize;

    const QPoint pt = QPoint(x - radius - 1, y - radius - 1);
    const QSize sz = QSize(2 * (radius+1), 2 * (radius+1));

    const QRect dabRectAligned = QRect(pt, sz);
    const QPointF center = QPointF(x, y);

    KisAlgebra2D::OuterCircle outer(center, radius);
    m_precisePainterWrapper.readRects(m_tempPainter->calculateAllMirroredRects(dabRectAligned));
    m_tempPainter->copyAreaOptimized(dabRectAligned.topLeft(), m_tempPainter->device(), m_dab, dabRectAligned);

    KisSequentialIterator it(m_dab, dabRectAligned);
    float unitValue = KoColorSpaceMathsTraits<channelType>::unitValue;
    float minValue = KoColorSpaceMathsTraits<channelType>::min;
    float maxValue = KoColorSpaceMathsTraits<channelType>::max;
    bool eraser = painter()->compositeOp()->id() == COMPOSITE_ERASE;

    while(it.nextPixel()) {

        QPoint pt(it.x(), it.y());

        if(outer.fadeSq(pt) > 1.0f)
            continue;

        float rr, base_alpha, alpha, dst_alpha, r, g, b, a;

        if (radius < 3.0) {
            rr = calculate_rr_antialiased (it.x(), it.y(), x, y, aspect_ratio, sn, cs, one_over_radius2, r_aa_start);
        }
        else {
            rr = calculate_rr (it.x(), it.y(), x, y, aspect_ratio, sn, cs, one_over_radius2);
        }

        base_alpha = calculate_alpha_for_rr (rr, hardness, segment1_slope, segment2_slope);
        m_tempPainter->selection();
        alpha = base_alpha * normal_mode;

        channelType* nativeArray = reinterpret_cast<channelType*>(it.rawData());

        b = nativeArray[0]/unitValue;
        g = nativeArray[1]/unitValue;
        r = nativeArray[2]/unitValue;
        dst_alpha = nativeArray[3]/unitValue;

        if (unitValue == 1.0f) {
            swap(b, r);
        }

        a = alpha * (color_a - dst_alpha) + dst_alpha;

        if (eraser) {
            alpha = 1 - (opaque*base_alpha);
            a = dst_alpha * alpha ;
        } else {
            if (a > 0.0f) {
                float src_term = (alpha * color_a) / a;
                float dst_term = 1.0f - src_term;
                r = color_r * src_term + r * dst_term;
                g = color_g * src_term + g * dst_term;
                b = color_b * src_term + b * dst_term;
            }

            if (colorize > 0.0f && base_alpha > 0.0f) {

                alpha = base_alpha * colorize;
                a = alpha + dst_alpha - alpha * dst_alpha;

                if (a > 0.0f) {

                    float pixel_h, pixel_s, pixel_l, out_h, out_s, out_l;
                    float out_r = r, out_g = g, out_b = b;

                    float src_term = alpha / a;
                    float dst_term = 1.0f - src_term;

                    RGBToHSL(color_r, color_g, color_b, &pixel_h, &pixel_s, &pixel_l);
                    RGBToHSL(out_r, out_g, out_b, &out_h, &out_s, &out_l);

                    out_h = pixel_h;
                    out_s = pixel_s;

                    HSLToRGB(out_h, out_s, out_l, &out_r, &out_g, &out_b);

                    r = (float)out_r * src_term + r * dst_term;
                    g = (float)out_g * src_term + g * dst_term;
                    b = (float)out_b * src_term + b * dst_term;
                }
            }
        }

        if (unitValue == 1.0f) {
            swap(b, r);
        }
        nativeArray[0] = qBound(minValue, b * unitValue, maxValue);
        nativeArray[1] = qBound(minValue, g * unitValue, maxValue);
        nativeArray[2] = qBound(minValue, r * unitValue, maxValue);
        nativeArray[3] = qBound(minValue, a * unitValue, maxValue);
    }
    m_tempPainter->bitBlt(dabRectAligned.topLeft(), m_dab, dabRectAligned);
    // Mirror mode is missing because I cannot figure out how to make a mask for the fixed paintdevice.
    const QVector<QRect> dirtyRects = m_tempPainter->takeDirtyRegion();
    m_precisePainterWrapper.writeRects(dirtyRects);
    painter()->addDirtyRects(dirtyRects);
    return 1;
}

template <typename channelType>
void KisMyPaintSurface::getColorImpl(MyPaintSurface *self, float x, float y, float radius,
                            float * color_r, float * color_g, float * color_b, float * color_a) {
    Q_UNUSED(self);
    if (radius < 1.0f)
        radius = 1.0f;

    *color_r = 0.0f;
    *color_g = 0.0f;
    *color_b = 0.0f;
    *color_a = 0.0f;

    const QPoint pt = QPoint(x - radius, y - radius);
    const QSize sz = QSize(2 * radius, 2 * radius);


    const QRect dabRectAligned = QRect(pt, sz);
    const QPointF center = QPointF(x, y);
    KisAlgebra2D::OuterCircle outer(center, radius);

    const float one_over_radius2 = 1.0f / (radius * radius);
    quint32 sum_weight = 0.0f;

    m_precisePainterWrapper.readRect(dabRectAligned);
    KisPaintDeviceSP activeDev = m_precisePainterWrapper.preciseDevice();
    if(m_image) {
        //m_image->blockUpdates();
        m_backgroundPainter->device()->clear();
        m_backgroundPainter->bitBlt(dabRectAligned.topLeft(), activeDev, dabRectAligned);
        activeDev = m_backgroundPainter->device();
        //m_image->unblockUpdates();
    } else if (m_imageDevice) {
        m_backgroundPainter->bitBlt(dabRectAligned.topLeft(), m_imageDevice, dabRectAligned);
        activeDev = m_backgroundPainter->device();
    } else {
        m_precisePainterWrapper.readRect(dabRectAligned);
    }

    KisSequentialIterator it(activeDev, dabRectAligned);
    QVector<float> surface_color_vec = {0,0,0,0};
    float unitValue = KoColorSpaceMathsTraits<channelType>::unitValue;
    float maxValue = KoColorSpaceMathsTraits<channelType>::max;

    quint32 size = dabRectAligned.width() * dabRectAligned.height();
    qint16 weights[size];
    const quint8* buffer[size];
    quint32 num_colors = 0;

    while(it.nextPixel()) {

        QPointF pt(it.x(), it.y());

        float rr = 0.0;
        if(outer.fadeSq(pt) <= 1.0) {
            /* pixel_weight == a standard dab with hardness = 0.5, aspect_ratio = 1.0, and angle = 0.0 */
            float yy = (it.y() + 0.5f - y);
            float xx = (it.x() + 0.5f - x);

            rr = qMax((yy * yy + xx * xx) * one_over_radius2, 0.0f);
        }

        weights[num_colors] = qRound((1.0f - rr) * 255);
        buffer[num_colors] = it.oldRawData();
        sum_weight += weights[num_colors];
        num_colors += 1;
    }

    KoColor color(Qt::transparent, activeDev->colorSpace());
    activeDev->colorSpace()->mixColorsOp()->mixColors(buffer, weights, size, color.data(), sum_weight);

    if (sum_weight > 0.0f) {
        qreal r, g, b, a;
        channelType* nativeArray = reinterpret_cast<channelType*>(color.data());

        if (unitValue == 1.0f) {
            *color_r = nativeArray[0];
            *color_g = nativeArray[1];
            *color_b = nativeArray[2];
            *color_a = nativeArray[3];
        } else {
            b = nativeArray[0]/maxValue;
            g = nativeArray[1]/maxValue;
            r = nativeArray[2]/maxValue;
            a = nativeArray[3]/maxValue;
            *color_r = CLAMP(r, 0.0f, 1.0f);
            *color_g = CLAMP(g, 0.0f, 1.0f);
            *color_b = CLAMP(b, 0.0f, 1.0f);
            *color_a = CLAMP(a, 0.0f, 1.0f);
        }
    }
}

KisPainter* KisMyPaintSurface::painter() {
    return m_painter;
}

MyPaintSurface* KisMyPaintSurface::surface() {
    return m_surface;
}

/*mypaint code*/
qreal KisMyPaintSurface::calculateOpacity(float angle, float hardness, float opaque, float x, float y,
                                        float xp, float yp, float aspect_ratio, float radius) {

    qreal cs = cos(angle/360*2*M_PI);
    qreal sn = sin(angle/360*2*M_PI);

    qreal dx = xp - x;
    qreal dy = yp - y;
    qreal dyr = (dy*cs-dx*sn)*aspect_ratio;
    qreal dxr = (dy*sn+dx*cs);
    qreal dd = (dyr*dyr + dxr*dxr) / (radius*radius);
    qreal opa;

    if (dd > 1)
        opa = 0;
    else if (dd < hardness)
        opa = dd + 1-(dd/hardness);
    else
        opa = hardness/(1-hardness)*(1-dd);

    qreal pixel_opacity = opa * opaque;
    return pixel_opacity;
}

inline float KisMyPaintSurface::calculate_rr (int  xp,
              int   yp,
              float x,
              float y,
              float aspect_ratio,
              float sn,
              float cs,
              float one_over_radius2) {

    const float yy = (yp + 0.5f - y);
    const float xx = (xp + 0.5f - x);
    const float yyr=(yy*cs-xx*sn)*aspect_ratio;
    const float xxr=yy*sn+xx*cs;
    const float rr = (yyr*yyr + xxr*xxr) * one_over_radius2;
    /* rr is in range 0.0..1.0*sqrt(2) */
    return rr;
}

static inline float
calculate_r_sample (float x, float y, float aspect_ratio, float sn, float cs) {

    const float yyr=(y*cs-x*sn)*aspect_ratio;
    const float xxr=y*sn+x*cs;
    const float r = (yyr*yyr + xxr*xxr);
    return r;
}

static inline float
sign_point_in_line (float px, float py, float vx, float vy) {

    return (px - vx) * (-vy) - (vx) * (py - vy);
}

static inline void
closest_point_to_line (float  lx, float  ly, float  px, float  py, float *ox, float *oy) {

    const float l2 = lx*lx + ly*ly;
    const float ltp_dot = px*lx + py*ly;
    const float t = ltp_dot / l2;
    *ox = lx * t;
    *oy = ly * t;
}


/* This works by taking the visibility at the nearest point
 * and dividing by 1.0 + delta.
 *
 * - nearest point: point where the dab has more influence
 * - farthest point: point at a fixed distance away from
 *                   the nearest point
 * - delta: how much occluded is the farthest point relative
 *          to the nearest point
 */
inline float KisMyPaintSurface::calculate_rr_antialiased (int  xp, int  yp, float x, float y,
                          float aspect_ratio, float sn, float cs, float one_over_radius2,
                          float r_aa_start) {

    /* calculate pixel position and borders in a way
     * that the dab's center is always at zero */
    float pixel_right = x - (float)xp;
    float pixel_bottom = y - (float)yp;
    float pixel_center_x = pixel_right - 0.5f;
    float pixel_center_y = pixel_bottom - 0.5f;
    float pixel_left = pixel_right - 1.0f;
    float pixel_top = pixel_bottom - 1.0f;

    float nearest_x, nearest_y; /* nearest to origin, but still inside pixel */
    float farthest_x, farthest_y; /* farthest from origin, but still inside pixel */
    float r_near, r_far, rr_near, rr_far;
    float center_sign, rad_area_1, visibilityNear, delta, delta2;

    /* Dab's center is inside pixel? */
    if( pixel_left<0 && pixel_right>0 &&
        pixel_top<0 && pixel_bottom>0 )
    {
        nearest_x = 0;
        nearest_y = 0;
        r_near = rr_near = 0;
    }
    else
    {
        closest_point_to_line( cs, sn, pixel_center_x, pixel_center_y, &nearest_x, &nearest_y );
        nearest_x = CLAMP( nearest_x, pixel_left, pixel_right );
        nearest_y = CLAMP( nearest_y, pixel_top, pixel_bottom );
        /* XXX: precision of "nearest" values could be improved
         * by intersecting the line that goes from nearest_x/Y to 0
         * with the pixel's borders here, however the improvements
         * would probably not justify the perdormance cost.
         */
        r_near = calculate_r_sample( nearest_x, nearest_y, aspect_ratio, sn, cs );
        rr_near = r_near * one_over_radius2;
    }

    /* out of dab's reach? */
    if( rr_near > 1.0f )
        return rr_near;

    /* check on which side of the dab's line is the pixel center */
    center_sign = sign_point_in_line( pixel_center_x, pixel_center_y, cs, -sn );

    /* radius of a circle with area=1
     *   A = pi * r * r
     *   r = sqrt(1/pi)
     */
    rad_area_1 = sqrtf( 1.0f / M_PI );

    /* center is below dab */
    if( center_sign < 0 )
    {
        farthest_x = nearest_x - sn*rad_area_1;
        farthest_y = nearest_y + cs*rad_area_1;
    }
    /* above dab */
    else
    {
        farthest_x = nearest_x + sn*rad_area_1;
        farthest_y = nearest_y - cs*rad_area_1;
    }

    r_far = calculate_r_sample( farthest_x, farthest_y, aspect_ratio, sn, cs );
    rr_far = r_far * one_over_radius2;

    /* check if we can skip heavier AA */
    if( r_far < r_aa_start )
        return (rr_far+rr_near) * 0.5f;

    /* calculate AA approximate */
    visibilityNear = 1.0f - rr_near;
    delta = rr_far - rr_near;
    delta2 = 1.0f + delta;
    visibilityNear /= delta2;

    return 1.0f - visibilityNear;
}
/* -- end mypaint code */

inline float KisMyPaintSurface::calculate_alpha_for_rr (float rr, float hardness, float slope1, float slope2) {

  if (rr > 1.0f)
    return 0.0f;
  else if (rr <= hardness)
    return 1.0f + rr * slope1;
  else
    return rr * slope2 - slope2;
}
