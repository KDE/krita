/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHMODEL_H
#define KISBRUSHMODEL_H

#include <QtGlobal>
#include <QSize>
#include <KoResourceSignature.h>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <boost/operators.hpp>

#include "kis_paintop_settings.h"

#include "kritabrush_export.h"

// TODO: move enumBrushApplication into a separate file
#include <kis_brush.h>

namespace KisBrushModel {
struct BRUSH_EXPORT CommonData : public boost::equality_comparable<CommonData>
{
    inline friend bool operator==(const CommonData &lhs, const CommonData &rhs) {
        return qFuzzyCompare(lhs.angle, rhs.angle) &&
                qFuzzyCompare(lhs.spacing, rhs.spacing) &&
                lhs.useAutoSpacing == rhs.useAutoSpacing &&
                qFuzzyCompare(lhs.autoSpacingCoeff, rhs.autoSpacingCoeff);
    }

    qreal angle = 0.0;
    qreal spacing = 0.05;
    bool useAutoSpacing = false;
    qreal autoSpacingCoeff = 1.0;

    // TODO: preview image
};

enum AutoBrushGeneratorShape {
    Circle = 0,
    Rectangle
};

enum AutoBrushGeneratorType {
    Default = 0,
    Soft,
    Gaussian
};

struct BRUSH_EXPORT AutoBrushGeneratorData : public boost::equality_comparable<AutoBrushGeneratorData>
{
    inline friend bool operator==(const AutoBrushGeneratorData &lhs, const AutoBrushGeneratorData &rhs) {
        return qFuzzyCompare(lhs.diameter, rhs.diameter) &&
                qFuzzyCompare(lhs.ratio, rhs.ratio) &&
                qFuzzyCompare(lhs.horizontalFade, rhs.horizontalFade) &&
                qFuzzyCompare(lhs.verticalFade, rhs.verticalFade) &&
                lhs.spikes == rhs.spikes &&
                lhs.antialiasEdges == rhs.antialiasEdges &&
                lhs.shape == rhs.shape &&
                lhs.type == rhs.type &&
                lhs.curveString == rhs.curveString;
    }

    qreal diameter = 42.0;
    qreal ratio = 1.0;
    qreal horizontalFade = 1.0;
    qreal verticalFade = 1.0;
    int spikes = 2;
    bool antialiasEdges = true;
    AutoBrushGeneratorShape shape = Circle;
    AutoBrushGeneratorType type = Default;
    QString curveString;
};

struct BRUSH_EXPORT AutoBrushData : public boost::equality_comparable<AutoBrushData>
{
    inline friend bool operator==(const AutoBrushData &lhs, const AutoBrushData &rhs) {
        return qFuzzyCompare(lhs.randomness, rhs.randomness) &&
                qFuzzyCompare(lhs.density, rhs.density) &&
                lhs.generator == rhs.generator;
    }

    qreal randomness = 0.0;
    qreal density = 0.0;
    AutoBrushGeneratorData generator;
};

struct BRUSH_EXPORT PredefinedBrushData : public boost::equality_comparable<PredefinedBrushData>
{
    inline friend bool operator==(const PredefinedBrushData &lhs, const PredefinedBrushData &rhs) {
        return lhs.resourceSignature == rhs.resourceSignature &&
                lhs.baseSize == rhs.baseSize &&
                qFuzzyCompare(lhs.scale, rhs.scale) &&
                lhs.application == rhs.application &&
                lhs.hasColorAndTransparency == rhs.hasColorAndTransparency &&
                lhs.autoAdjustMidPoint == rhs.autoAdjustMidPoint &&
                lhs.adjustmentMidPoint == rhs.adjustmentMidPoint &&
                qFuzzyCompare(lhs.brightnessAdjustment, rhs.brightnessAdjustment) &&
                qFuzzyCompare(lhs.contrastAdjustment, rhs.contrastAdjustment);
    }

    KoResourceSignature resourceSignature;

    QSize baseSize = QSize(42, 42);
    qreal scale = 1.0;
    enumBrushApplication application = ALPHAMASK;
    bool hasColorAndTransparency = false;
    bool autoAdjustMidPoint = false;
    quint8 adjustmentMidPoint = 127;
    qreal brightnessAdjustment = 0.0;
    qreal contrastAdjustment = 0.0;
};

struct BRUSH_EXPORT TextBrushData : boost::equality_comparable<TextBrushData>
{
    inline friend bool operator==(const TextBrushData &lhs, const TextBrushData &rhs) {
        return lhs.baseSize == rhs.baseSize &&
                qFuzzyCompare(lhs.scale, rhs.scale) &&
                lhs.text == rhs.text &&
                lhs.font == rhs.font &&
                lhs.usePipeMode == rhs.usePipeMode;
    }

    QSize baseSize = QSize(42, 42);
    qreal scale = 1.0;
    QString text = "The quick brown fox ate your text";
    QString font;
    bool usePipeMode = false;
};

enum BrushType {
    Auto = 0,
    Predefined,
    Text
};

struct BRUSH_EXPORT BrushData : public boost::equality_comparable<BrushData> {
    inline friend bool operator==(const BrushData &lhs, const BrushData &rhs) {
        return lhs.common == rhs.common &&
                lhs.type == rhs.type &&
                lhs.subtype == rhs.subtype &&
                lhs.autoBrush == rhs.autoBrush &&
                lhs.predefinedBrush == rhs.predefinedBrush &&
                lhs.textBrush == rhs.textBrush;
    }

    CommonData common;
    BrushType type = Auto;
    QString subtype; // must be consistent with the type of the predefined brush
    AutoBrushData autoBrush;
    PredefinedBrushData predefinedBrush;
    TextBrushData textBrush;

    void write(KisPropertiesConfiguration *settings) const;
    static std::optional<BrushData> read(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface);
};

class BRUSH_EXPORT BrushModel : public QObject
{
    Q_OBJECT
public:
    BrushModel(lager::cursor<BrushData> source);

    // the state must be declared **before** any cursors or readers
    lager::cursor<BrushData> m_source;

    LAGER_QT_CURSOR(CommonData, commonData);
    LAGER_QT_CURSOR(BrushType, brushType);
    LAGER_QT_CURSOR(AutoBrushData, autoBrush);
    LAGER_QT_CURSOR(PredefinedBrushData, predefinedBrush);
    LAGER_QT_CURSOR(TextBrushData, textBrush);
    LAGER_QT_READER(qreal, userEffectiveSize);
};


}

#endif // KISBRUSHMODEL_H
