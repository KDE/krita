/*
    Copyright (C) 2011 Silvio Heinrich <plassy@web.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef H_KIS_COLOR_SELECTOR_H
#define H_KIS_COLOR_SELECTOR_H

#include <QWidget>
#include <QVector>
#include <QImage>
#include <QPainterPath>

#include "kis_color.h"
#include "kis_radian.h"
#include "kis_acs_types.h"
#include "kis_signal_compressor_with_param.h"

#include <resources/KoGamutMask.h>
#include <KisGamutMaskViewConverter.h>

class QPainter;
class KisDisplayColorConverter;

class KisColorSelector: public QWidget
{
    Q_OBJECT

    typedef KisRadian<qreal> Radian;

    struct ColorRing
    {
        ColorRing()
            : saturation(0)
            , outerRadius(0)
            , innerRadius(0)
        { }

        qreal                 saturation;
        qreal                 outerRadius;
        qreal                 innerRadius;
        QVector<QPainterPath> pieced;
    };

public:
    KisColorSelector(QWidget* parent, KisColor::Type type=KisColor::HSL);

    void setColorSpace(KisColor::Type type);
    void setColorConverter(KisDisplayColorConverter* colorConverter);
    void setNumPieces(int num);
    void setNumLightPieces(int num) __attribute__((optimize(0)));
    void setNumRings(int num);

    void setLight(qreal light=0.0f);

    void setLumaCoefficients(qreal lR, qreal lG, qreal lB, qreal lGamma);
    inline qreal lumaR() const { return m_lumaR; }
    inline qreal lumaG() const { return m_lumaG; }
    inline qreal lumaB() const { return m_lumaB; }
    inline qreal lumaGamma() const { return m_lumaGamma; }

    void setInverseSaturation(bool inverse);
    void selectColor(const KisColor& color);
    void setFgColor(const KoColor& fgColor);
    void setBgColor(const KoColor& bgColor);

    void setDefaultHueSteps(int num);
    void setDefaultSaturationSteps(int num);
    void setDefaultValueScaleSteps(int num);
    void setShowBgColor(bool value);
    void setShowValueScaleNumbers(bool value);
    void setGamutMask(KoGamutMaskSP gamutMask);
    void setDirty();
    bool gamutMaskOn();
    void setGamutMaskOn(bool gamutMaskOn);
    void setEnforceGamutMask(bool enforce);
    KoGamutMaskSP gamutMask();

    void saveSettings();
    void loadSettings();

    KisColor::Type getColorSpace       () const { return m_colorSpace;        }
    qint32         getNumRings         () const { return m_colorRings.size(); }
    qint32         getNumPieces        () const { return m_numPieces;         }
    qint32         getNumLightPieces   () const { return m_numLightPieces;    }
    bool           isSaturationInverted() const { return m_inverseSaturation; }

    quint32        getDefaultHueSteps  () const { return m_defaultHueSteps;        }
    quint32        getDefaultSaturationSteps () const { return m_defaultSaturationSteps;        }
    quint32        getDefaultValueScaleSteps () const { return m_defaultValueScaleSteps;        }
    bool           getShowBgColor () const { return m_showBgColor;        }
    bool           getShowValueScaleNumbers () const { return m_showValueScaleNumbers;        }
    bool           enforceGamutMask () const { return m_enforceGamutMask;        }

Q_SIGNALS:
    void sigFgColorChanged(const KisColor& color);
    void sigBgColorChanged(const KisColor& color);

private:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void leaveEvent(QEvent* e) override;

    bool colorIsClear(const KisColor &color);
    bool colorIsClear(const QPointF &colorPoint);
    void requestUpdateColorAndPreview(const KisColor &color, Acs::ColorRole role);

    void recalculateAreas(quint8 numLightPieces);
    void recalculateRings(quint8 numRings, quint8 numPieces);
    void createRing(ColorRing& wheel, quint8 numPieces, qreal innerRadius, qreal outerRadius);

    void drawRing(QPainter& painter, ColorRing& wheel, const QRect& rect);
    void drawOutline(QPainter& painter, const QRect& rect);
    void drawBlip(QPainter& painter, const QRect& rect);
    void drawLightStrip(QPainter& painter, const QRect& rect);
    void drawGamutMaskShape(QPainter& painter, const QRect& rect);
    void drawColorPreview(QPainter& painter, const QRect& rect);

    qint8 getHueIndex(Radian hue) const;
    qreal getHue(int hueIdx, Radian shift=0.0f) const;
    qint8 getLightIndex(const QPointF& pt) const;
    qint8 getLightIndex(qreal light) const;
    qreal getLight(const QPointF& pt) const;
    qint8 getSaturationIndex(const QPointF& pt) const;
    qint8 getSaturationIndex(qreal saturation) const;
    qreal getSaturation(int saturationIdx) const;

    QPointF mapCoordToView(const QPointF& pt, const QRectF& viewRect) const;
    QPointF mapCoordToUnit(const QPointF& pt, const QRectF& viewRect) const;
    QPointF mapColorToUnit(const KisColor& color, bool invertSaturation = true) const;
    Radian mapCoordToAngle(qreal x, qreal y) const;
    QPointF mapHueToAngle(qreal hue) const;

public:
    // This is a private interface for signal compressor, don't use it.
    // Use requestUpdateColorAndPreview() instead
    void slotUpdateColorAndPreview(QPair<KisColor, Acs::ColorRole> color);

private:
    KisDisplayColorConverter* m_colorConverter;
    KisColor::Type     m_colorSpace;
    quint8             m_numPieces;
    quint8             m_numLightPieces;
    bool               m_inverseSaturation;
    qint8              m_selectedRing;
    qint8              m_selectedPiece;
    qint8              m_selectedLightPiece;
    KisColor           m_selectedColor;
    KisColor           m_fgColor;
    KisColor           m_bgColor;
    QImage             m_renderBuffer;
    QImage             m_maskBuffer;
    QImage             m_lightStripBuffer;
    QImage             m_colorPreviewBuffer;
    QRect              m_widgetArea;
    QRect              m_renderArea;
    QRect              m_lightStripArea;
    bool               m_mouseMoved;
    QPointF            m_clickPos;
    qint8              m_clickedRing;
    QVector<ColorRing> m_colorRings;
    Qt::MouseButtons   m_pressedButtons;

    // docker settings
    quint8 m_defaultHueSteps;
    quint8 m_defaultSaturationSteps;
    quint8 m_defaultValueScaleSteps;
    bool m_showValueScaleNumbers;
    bool m_showBgColor;

    bool m_gamutMaskOn;
    KoGamutMaskSP m_currentGamutMask;
    bool m_enforceGamutMask;
    QSize m_renderAreaSize;
    bool m_maskPreviewActive;
    KisGamutMaskViewConverter* m_viewConverter;

    bool m_widgetUpdatesSelf;
    bool m_isDirtyWheel;
    bool m_isDirtyLightStrip;
    bool m_isDirtyGamutMask;
    bool m_isDirtyColorPreview;

    qreal m_lumaR;
    qreal m_lumaG;
    qreal m_lumaB;
    qreal m_lumaGamma;

    typedef KisSignalCompressorWithParam<QPair<KisColor, Acs::ColorRole>> ColorCompressorType;
    QScopedPointer<ColorCompressorType> m_updateColorCompressor;
};

#endif // H_KIS_COLOR_SELECTOR_H
