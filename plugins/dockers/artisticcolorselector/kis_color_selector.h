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
class QPainter;

class KisColorSelector: public QWidget
{
    Q_OBJECT

    typedef KisRadian<float> Radian;

    struct ColorRing
    {
        ColorRing()
            : saturation(0)
            , outerRadius(0)
            , innerRadius(0)
        { }

        float                 saturation;
        float                 outerRadius;
        float                 innerRadius;
        QVector<QPainterPath> pieced;
    };

public:
    KisColorSelector(QWidget* parent, KisColor::Type type=KisColor::HSL);

    void setColorSpace(KisColor::Type type, float valueScaleGamma);
    void setNumPieces(int num);
    void setNumLightPieces(int num) __attribute__((optimize(0)));
    void setNumRings(int num);

    void setLight(float light=0.0f);

    float gamma() const { return m_gamma; }
    void setGamma(float gamma);

    void setInverseSaturation(bool inverse);
    void selectColor(const KisColor& color);
    void setFgColor(const KisColor& fgColor);
    void setBgColor(const KisColor& bgColor);

    void setDefaultHueSteps(int num);
    void setDefaultSaturationSteps(int num);
    void setDefaultValueScaleSteps(int num);
    void setShowColorBlip(bool value);
    void setShowBgColor(bool value);
    void setShowValueScaleNumbers(bool value);
    void setGamutMask(KoGamutMask* gamutMask);
    bool gamutMaskOn();
    void setGamutMaskOn(bool gamutMaskOn);
    void setEnforceGamutMask(bool enforce);
    KoGamutMask* gamutMask();

    bool maskPreviewActive();
    void setMaskPreviewActive(bool value);

    bool saturationIsInvertible();

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
    bool           getShowColorBlip () const { return m_showColorBlip;        }
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
    QPointF mapHueToAngle(float hue) const;

public:
    // This is a private interface for signal compressor, don't use it.
    // Use requestUpdateColorAndPreview() instead
    void slotUpdateColorAndPreview(QPair<KisColor, Acs::ColorRole> color);

private:
    KisColor::Type     m_colorSpace;
    quint8             m_numPieces;
    quint8             m_numLightPieces;
    bool               m_inverseSaturation;
    float              m_gamma;
    qint8              m_selectedRing;
    qint8              m_selectedPiece;
    qint8              m_selectedLightPiece;
    KisColor           m_selectedColor;
    KisColor           m_fgColor;
    KisColor           m_bgColor;
    QImage             m_renderBuffer;
    QImage             m_maskBuffer;
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
    bool m_showColorBlip;
    bool m_showValueScaleNumbers;
    bool m_showBgColor;

    bool m_gamutMaskOn;
    KoGamutMask* m_currentGamutMask;
    bool m_enforceGamutMask;
    QSize m_renderAreaSize;
    bool m_maskPreviewActive;
    KisGamutMaskViewConverter* m_viewConverter;

    bool m_widgetUpdatesSelf;

    typedef KisSignalCompressorWithParam<QPair<KisColor, Acs::ColorRole>> ColorCompressorType;
    QScopedPointer<ColorCompressorType> m_updateColorCompressor;
};

#endif // H_KIS_COLOR_SELECTOR_H
