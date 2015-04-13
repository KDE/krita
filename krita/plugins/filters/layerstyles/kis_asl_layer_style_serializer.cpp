/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_asl_layer_style_serializer.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <QDomDocument>

#include <KoResourceServerProvider.h>
#include <KoAbstractGradient.h>
#include <KoPattern.h>


#include "psd.h"
#include "kis_global.h"
#include "kis_psd_layer_style.h"
#include "kis_embedded_pattern_manager.h"

#include "kis_asl_reader.h"
#include "kis_asl_xml_parser.h"
#include "kis_asl_callback_object_catcher.h"
#include "kis_asl_writer_utils.h"



KisAslLayerStyleSerializer::KisAslLayerStyleSerializer(KisPSDLayerStyle *style)
    : m_style(style)
{
}

KisAslLayerStyleSerializer::~KisAslLayerStyleSerializer()
{
}


KisLayerStyleSerializerSP KisAslLayerStyleSerializer::factoryObject(KisPSDLayerStyle *style)
{
    return toQShared(new KisAslLayerStyleSerializer(style));
}

void KisAslLayerStyleSerializer::saveToDevice(QIODevice *device)
{

}

void convertAndSetBlendMode(const QString &mode,
                            boost::function<void (const QString &)> setBlendMode)
{
    QString compositeOp = COMPOSITE_OVER;

    if (mode == "Nrml") {
        compositeOp = COMPOSITE_OVER;
    } else if (mode == "Dslv") {
        compositeOp = COMPOSITE_DISSOLVE;
    } else if (mode == "Drkn") {
        compositeOp = COMPOSITE_DARKEN;
    } else if (mode == "Mltp") {
        compositeOp = COMPOSITE_MULT;
    } else if (mode == "CBrn") {
        compositeOp = COMPOSITE_BURN;
    } else if (mode == "linearBurn") {
        compositeOp = COMPOSITE_LINEAR_BURN;
    } else if (mode == "darkerColor") {
        compositeOp = COMPOSITE_DARKER_COLOR;
    } else if (mode == "Lghn") {
        compositeOp = COMPOSITE_LIGHTEN;
    } else if (mode == "Scrn") {
        compositeOp = COMPOSITE_SCREEN;
    } else if (mode == "CDdg") {
        compositeOp = COMPOSITE_DODGE;
    } else if (mode == "linearDodge") {
        compositeOp = COMPOSITE_LINEAR_DODGE;
    } else if (mode == "lighterColor") {
        compositeOp = COMPOSITE_LIGHTER_COLOR;
    } else if (mode == "Ovrl") {
        compositeOp = COMPOSITE_OVERLAY;
    } else if (mode == "SftL") {
        compositeOp = COMPOSITE_SOFT_LIGHT_PHOTOSHOP;
    } else if (mode == "HrdL") {
        compositeOp = COMPOSITE_HARD_LIGHT;
    } else if (mode == "vividLight") {
        compositeOp = COMPOSITE_VIVID_LIGHT;
    } else if (mode == "linearLight") {
        compositeOp = COMPOSITE_LINEAR_LIGHT;
    } else if (mode == "pinLight") {
        compositeOp = COMPOSITE_PIN_LIGHT;
    } else if (mode == "hardMix") {
        compositeOp = COMPOSITE_HARD_MIX;
    } else if (mode == "Dfrn") {
        compositeOp = COMPOSITE_DIFF;
    } else if (mode == "Xclu") {
        compositeOp = COMPOSITE_EXCLUSION;
    } else if (mode == "Sbtr") {
        compositeOp = COMPOSITE_SUBTRACT;
    } else if (mode == "divide") {
        compositeOp = COMPOSITE_DIVIDE;
    } else if (mode == "H   ") {
        compositeOp = COMPOSITE_HUE;
    } else if (mode == "Strt") {
        compositeOp = COMPOSITE_SATURATION;
    } else if (mode == "Clr ") {
        compositeOp = COMPOSITE_COLOR;
    } else if (mode == "Lmns") {
        compositeOp = COMPOSITE_LUMINIZE;
    } else {
        qDebug() << "Unknown blending mode:" << mode << "Returning COMPOSITE_OVER!";
    }

    setBlendMode(compositeOp);
}

void convertAndSetCurve(const QString &name,
                        const QVector<QPointF> &points,
                        boost::function<void (const quint8*)> setCurveLookupTable)
{
    Q_UNUSED(name);
    Q_UNUSED(points);
    Q_UNUSED(setCurveLookupTable);

    qWarning() << "convertAndSetBlendMode:" << "Curve conversion is not implemented yet";
}

template <typename T>
void convertAndSetEnum(const QString &value,
                       const QMap<QString, T> map,
                       boost::function<void (T)> setMappedValue)
{
    setMappedValue(map[value]);
}

template <class T>
void cloneAndSetResource(const T *resource,
                         boost::function<void (T*)> setResource)
{
    setResource(const_cast<T *>(resource)->clone());
}

inline QString _prepaddr(const QString &addr) {
    return QString("/Styl/Lefx") + addr;
}

#define CONN_COLOR(addr, method, object, type) c.subscribeColor(_prepaddr(addr), boost::bind(&type::method, object, _1))
#define CONN_UNITF(addr, unit, method, object, type) c.subscribeUnitFloat(_prepaddr(addr), unit, boost::bind(&type::method, object, _1))
#define CONN_BOOL(addr, method, object, type) c.subscribeBoolean(_prepaddr(addr), boost::bind(&type::method, object, _1))

#define CONN_POINT(addr, method, object, type) c.subscribePoint(_prepaddr(addr), boost::bind(&type::method, object, _1))

#define CONN_COMPOSITE_OP(addr, method, object, type)                   \
    {                                                                   \
        boost::function<void (const QString&)> setter =                 \
            boost::bind(&type::method, object, _1);                     \
        c.subscribeEnum(_prepaddr(addr), "BlnM", boost::bind(convertAndSetBlendMode, _1, setter)); \
    }

#define CONN_CURVE(addr, method, object, type)                          \
    {                                                                   \
        boost::function<void (const quint8*)> setter =                  \
            boost::bind(&type::method, object, _1);                     \
        c.subscribeCurve(_prepaddr(addr), boost::bind(convertAndSetCurve, _1, _2, setter)); \
    }

#define CONN_ENUM(addr, tag, method, map, mapped_type, object, type)                       \
    {                                                                   \
        boost::function<void (mapped_type)> setter =                  \
            boost::bind(&type::method, object, _1);                     \
        c.subscribeEnum(_prepaddr(addr), tag, boost::bind(convertAndSetEnum<mapped_type>, _1, map, setter)); \
    }

#define CONN_GRADIENT(addr, method, object, type)                      \
    {                                                                  \
        boost::function<void (KoAbstractGradient*)> setter =    \
            boost::bind(&type::method, object, _1);                    \
        c.subscribeGradient(_prepaddr(addr), boost::bind(cloneAndSetResource<KoAbstractGradient>, _1, setter)); \
    }

#define CONN_PATTERN(addr, method, object, type)                       \
    {                                                                  \
        boost::function<void (KoPattern*)> setter =    \
            boost::bind(&type::method, object, _1);                     \
        c.subscribePatternRef(_prepaddr(addr), boost::bind(&KisAslLayerStyleSerializer::assignPatternObject, this, _1, _2, setter)); \
    }

void KisAslLayerStyleSerializer::registerPatternObject(const KoPattern *pattern) {
    QString uuid = KisAslWriterUtils::getPatternUuidLazy(pattern);

    if (m_patternsStore.contains(uuid)) {
        qWarning() << "WARNING: ASL style contains a duplicated pattern!" << ppVar(pattern->name()) << ppVar(m_patternsStore[uuid]->name());
    } else {
        KoPattern *patternToAdd = KisEmbeddedPatternManager::tryFetchPatternByMd5(pattern->md5());

        if (!patternToAdd) {
            patternToAdd = pattern->clone();

            KoResourceServer<KoPattern> *server = KoResourceServerProvider::instance()->patternServer();
            server->addResource(patternToAdd, false);
        }

        m_patternsStore.insert(uuid, patternToAdd);
    }
}

void KisAslLayerStyleSerializer::assignPatternObject(const QString &patternUuid,
                                                     const QString &patternName,
                                                     boost::function<void (KoPattern *)> setPattern)
{
    Q_UNUSED(patternName);

    KoPattern *pattern = m_patternsStore[patternUuid];

    if (!pattern) {
        qWarning() << "WARNING: ASL style contains inexistent pattern reference!";
    }

    setPattern(pattern);
}

template <class T>
struct CorrectFillTypeOnExitNoPattern
{
    CorrectFillTypeOnExitNoPattern(T *config) : m_config(config) {}

    ~CorrectFillTypeOnExitNoPattern() {
        if (m_config->gradient()) {
            m_config->setFillType(psd_fill_gradient);
        } else {
            m_config->setFillType(psd_fill_solid_color);
        }
    }

private:
    T *m_config;
};

template <class T>
struct CorrectFillTypeOnExitWithPattern
{
    CorrectFillTypeOnExitWithPattern(T *config) : m_config(config) {}

    ~CorrectFillTypeOnExitWithPattern() {
        if (m_config->pattern()) {
            m_config->setFillType(psd_fill_pattern);
        } else if (m_config->gradient()) {
            m_config->setFillType(psd_fill_gradient);
        } else {
            m_config->setFillType(psd_fill_solid_color);
        }
    }

private:
    T *m_config;
};

void KisAslLayerStyleSerializer::readFromDevice(QIODevice *device)
{
    psd_layer_effects_context *context = m_style->context();
    context->global_angle = 0;
    context->keep_original = 0;

    KisAslCallbackObjectCatcher c;

    c.subscribePattern("/Patterns/KisPattern", boost::bind(&KisAslLayerStyleSerializer::registerPatternObject, this, _1));

    psd_layer_effects_drop_shadow *dropShadow = m_style->dropShadow();

    CONN_COMPOSITE_OP("/DrSh/Md  ", setBlendMode, dropShadow, psd_layer_effects_drop_shadow);
    CONN_COLOR("/DrSh/Clr ", setColor, dropShadow, psd_layer_effects_drop_shadow);
    CONN_UNITF("/DrSh/Opct", "#Prc", setOpacity, dropShadow, psd_layer_effects_drop_shadow);
    CONN_UNITF("/DrSh/lagl", "#Ang", setAngle, dropShadow, psd_layer_effects_drop_shadow);
    CONN_UNITF("/DrSh/Dstn", "#Pxl", setDistance, dropShadow, psd_layer_effects_drop_shadow);
    CONN_UNITF("/DrSh/Ckmt", "#Pxl", setSpread, dropShadow, psd_layer_effects_drop_shadow);
    CONN_UNITF("/DrSh/blur", "#Pxl", setSize, dropShadow, psd_layer_effects_drop_shadow);
    CONN_UNITF("/DrSh/Nose", "#Prc", setNoise, dropShadow, psd_layer_effects_drop_shadow);
    CONN_BOOL("/DrSh/enab", setEffectEnabled, dropShadow, psd_layer_effects_drop_shadow);
    CONN_BOOL("/DrSh/uglg", setUseGlobalLight, dropShadow, psd_layer_effects_drop_shadow);
    CONN_BOOL("/DrSh/AntA", setAntiAliased, dropShadow, psd_layer_effects_drop_shadow);
    CONN_BOOL("/DrSh/layerConceals", setKnocksOut, dropShadow, psd_layer_effects_drop_shadow);
    CONN_CURVE("/DrSh/TrnS", setContourLookupTable, dropShadow, psd_layer_effects_drop_shadow);

    psd_layer_effects_inner_shadow *innerShadow = m_style->innerShadow();

    CONN_COMPOSITE_OP("/IrSh/Md  ", setBlendMode, innerShadow, psd_layer_effects_inner_shadow);
    CONN_COLOR("/IrSh/Clr ", setColor, innerShadow, psd_layer_effects_inner_shadow);
    CONN_UNITF("/IrSh/Opct", "#Prc", setOpacity, innerShadow, psd_layer_effects_inner_shadow);
    CONN_UNITF("/IrSh/lagl", "#Ang", setAngle, innerShadow, psd_layer_effects_inner_shadow);
    CONN_UNITF("/IrSh/Dstn", "#Pxl", setDistance, innerShadow, psd_layer_effects_inner_shadow);
    CONN_UNITF("/IrSh/Ckmt", "#Pxl", setSpread, innerShadow, psd_layer_effects_inner_shadow);
    CONN_UNITF("/IrSh/blur", "#Pxl", setSize, innerShadow, psd_layer_effects_inner_shadow);
    CONN_UNITF("/IrSh/Nose", "#Prc", setNoise, innerShadow, psd_layer_effects_inner_shadow);
    CONN_BOOL("/IrSh/enab", setEffectEnabled, innerShadow, psd_layer_effects_inner_shadow);
    CONN_BOOL("/IrSh/uglg", setUseGlobalLight, innerShadow, psd_layer_effects_inner_shadow);
    CONN_BOOL("/IrSh/AntA", setAntiAliased, innerShadow, psd_layer_effects_inner_shadow);
    CONN_CURVE("/IrSh/TrnS", setContourLookupTable, innerShadow, psd_layer_effects_inner_shadow);

    psd_layer_effects_outer_glow *outerGlow = m_style->outerGlow();

    CONN_COMPOSITE_OP("/OrGl/Md  ", setBlendMode, outerGlow, psd_layer_effects_outer_glow);
    CONN_COLOR("/OrGl/Clr ", setColor, outerGlow, psd_layer_effects_outer_glow);
    CONN_UNITF("/OrGl/Opct", "#Prc", setOpacity, outerGlow, psd_layer_effects_outer_glow);
    CONN_UNITF("/OrGl/Ckmt", "#Pxl", setSpread, outerGlow, psd_layer_effects_outer_glow);
    CONN_UNITF("/OrGl/blur", "#Pxl", setSize, outerGlow, psd_layer_effects_outer_glow);
    CONN_UNITF("/OrGl/Nose", "#Prc", setNoise, outerGlow, psd_layer_effects_outer_glow);
    CONN_BOOL("/OrGl/enab", setEffectEnabled, outerGlow, psd_layer_effects_outer_glow);
    CONN_BOOL("/OrGl/AntA", setAntiAliased, outerGlow, psd_layer_effects_outer_glow);
    CONN_CURVE("/OrGl/TrnS", setContourLookupTable, outerGlow, psd_layer_effects_outer_glow);

    QMap<QString, psd_technique_type> fillTechniqueMap;
    fillTechniqueMap.insert("PrBL", psd_technique_precise);
    fillTechniqueMap.insert("SfBL", psd_technique_softer);
    CONN_ENUM("/OrGl/GlwT", "BETE", setTechnique, fillTechniqueMap, psd_technique_type, outerGlow, psd_layer_effects_outer_glow);

    CorrectFillTypeOnExitNoPattern<psd_layer_effects_outer_glow> outerGlowFillTypeDelayedFiller(outerGlow);

    CONN_GRADIENT("/OrGl/Grad", setGradient, outerGlow, psd_layer_effects_outer_glow);

    CONN_UNITF("/OrGl/Inpr", "#Prc", setRange, outerGlow, psd_layer_effects_outer_glow);
    CONN_UNITF("/OrGl/ShdN", "#Prc", setJitter, outerGlow, psd_layer_effects_outer_glow);


    psd_layer_effects_inner_glow *innerGlow = m_style->innerGlow();

    CONN_COMPOSITE_OP("/IrGl/Md  ", setBlendMode, innerGlow, psd_layer_effects_inner_glow);
    CONN_COLOR("/IrGl/Clr ", setColor, innerGlow, psd_layer_effects_inner_glow);
    CONN_UNITF("/IrGl/Opct", "#Prc", setOpacity, innerGlow, psd_layer_effects_inner_glow);
    CONN_UNITF("/IrGl/Ckmt", "#Pxl", setSpread, innerGlow, psd_layer_effects_inner_glow);
    CONN_UNITF("/IrGl/blur", "#Pxl", setSize, innerGlow, psd_layer_effects_inner_glow);
    CONN_UNITF("/IrGl/Nose", "#Prc", setNoise, innerGlow, psd_layer_effects_inner_glow);
    CONN_BOOL("/IrGl/enab", setEffectEnabled, innerGlow, psd_layer_effects_inner_glow);
    CONN_BOOL("/IrGl/AntA", setAntiAliased, innerGlow, psd_layer_effects_inner_glow);
    CONN_CURVE("/IrGl/TrnS", setContourLookupTable, innerGlow, psd_layer_effects_inner_glow);

    CONN_ENUM("/IrGl/GlwT", "BETE", setTechnique, fillTechniqueMap, psd_technique_type, innerGlow, psd_layer_effects_inner_glow);

    CorrectFillTypeOnExitNoPattern<psd_layer_effects_inner_glow> innerGlowFillTypeDelayedFiller(innerGlow);

    CONN_GRADIENT("/IrGl/Grad", setGradient, innerGlow, psd_layer_effects_inner_glow);

    CONN_UNITF("/IrGl/Inpr", "#Prc", setRange, innerGlow, psd_layer_effects_inner_glow);
    CONN_UNITF("/IrGl/ShdN", "#Prc", setJitter, innerGlow, psd_layer_effects_inner_glow);

    QMap<QString, psd_glow_source> glowSourceMap;
    glowSourceMap.insert("SrcC", psd_glow_center);
    glowSourceMap.insert("SrcE", psd_glow_edge);
    CONN_ENUM("/IrGl/glwS", "IGSr", setSource, glowSourceMap, psd_glow_source, innerGlow, psd_layer_effects_inner_glow);


    psd_layer_effects_satin *satin = m_style->satin();

    CONN_COMPOSITE_OP("/ChFX/Md  ", setBlendMode, satin, psd_layer_effects_satin);
    CONN_COLOR("/ChFX/Clr ", setColor, satin, psd_layer_effects_satin);

    CONN_UNITF("/ChFX/Opct", "#Prc", setOpacity, satin, psd_layer_effects_satin);
    CONN_UNITF("/ChFX/lagl", "#Ang", setAngle, satin, psd_layer_effects_satin);
    CONN_UNITF("/ChFX/Dstn", "#Pxl", setDistance, satin, psd_layer_effects_satin);
    CONN_UNITF("/ChFX/blur", "#Pxl", setSize, satin, psd_layer_effects_satin);

    CONN_BOOL("/ChFX/enab", setEffectEnabled, satin, psd_layer_effects_satin);
    CONN_BOOL("/ChFX/AntA", setAntiAliased, satin, psd_layer_effects_satin);
    CONN_BOOL("/ChFX/Invr", setInvert, satin, psd_layer_effects_satin);
    CONN_CURVE("/ChFX/MpgS", setContourLookupTable, satin, psd_layer_effects_satin);

    psd_layer_effects_color_overlay *colorOverlay = m_style->colorOverlay();

    CONN_COMPOSITE_OP("/SoFi/Md  ", setBlendMode, colorOverlay, psd_layer_effects_color_overlay);
    CONN_COLOR("/SoFi/Clr ", setColor, colorOverlay, psd_layer_effects_color_overlay);
    CONN_UNITF("/SoFi/Opct", "#Prc", setOpacity, colorOverlay, psd_layer_effects_color_overlay);
    CONN_BOOL("/SoFi/enab", setEffectEnabled, colorOverlay, psd_layer_effects_color_overlay);

    psd_layer_effects_gradient_overlay *gradientOverlay = m_style->gradientOverlay();

    CONN_COMPOSITE_OP("/GrFl/Md  ", setBlendMode, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_UNITF("/GrFl/Opct", "#Prc", setOpacity, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_UNITF("/GrFl/Scl ", "#Prc", setScale, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_UNITF("/GrFl/Angl", "#Ang", setAngle, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_BOOL("/GrFl/enab", setEffectEnabled, gradientOverlay, psd_layer_effects_gradient_overlay);
    // CONN_BOOL("/GrFl/Dthr", setDitherNotImplemented, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_BOOL("/GrFl/Rvrs", setReverse, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_BOOL("/GrFl/Algn", setAlignWithLayer, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_POINT("/GrFl/Ofst", setGradientOffset, gradientOverlay, psd_layer_effects_gradient_overlay);
    CONN_GRADIENT("/GrFl/Grad", setGradient, gradientOverlay, psd_layer_effects_gradient_overlay);


    QMap<QString, psd_gradient_style> gradientStyleMap;
    gradientStyleMap.insert("Lnr ", psd_gradient_style_linear);
    gradientStyleMap.insert("Rdl ", psd_gradient_style_radial);
    gradientStyleMap.insert("Angl", psd_gradient_style_angle);
    gradientStyleMap.insert("Rflc", psd_gradient_style_reflected);
    gradientStyleMap.insert("Dmnd", psd_gradient_style_diamond);
    CONN_ENUM("/GrFl/Type", "GrdT", setStyle, gradientStyleMap, psd_gradient_style, gradientOverlay, psd_layer_effects_gradient_overlay);

    psd_layer_effects_pattern_overlay *patternOverlay = m_style->patternOverlay();

    CONN_BOOL("/patternFill/enab", setEffectEnabled, patternOverlay, psd_layer_effects_pattern_overlay);
    CONN_COMPOSITE_OP("/patternFill/Md  ", setBlendMode, patternOverlay, psd_layer_effects_pattern_overlay);
    CONN_UNITF("/patternFill/Opct", "#Prc", setOpacity, patternOverlay, psd_layer_effects_pattern_overlay);
    CONN_PATTERN("/patternFill/Ptrn", setPattern, patternOverlay, psd_layer_effects_pattern_overlay);
    CONN_UNITF("/patternFill/Scl ", "#Prc", setScale, patternOverlay, psd_layer_effects_pattern_overlay);
    CONN_BOOL("/patternFill/Algn", setAlignWithLayer, patternOverlay, psd_layer_effects_pattern_overlay);
    CONN_POINT("/patternFill/phase", setPatternPhase, patternOverlay, psd_layer_effects_pattern_overlay);

    psd_layer_effects_stroke *stroke = m_style->stroke();

    CONN_COMPOSITE_OP("/FrFX/Md  ", setBlendMode, stroke, psd_layer_effects_stroke);
    CONN_BOOL("/FrFX/enab", setEffectEnabled, stroke, psd_layer_effects_stroke);
    CONN_UNITF("/FrFX/Opct", "#Prc", setOpacity, stroke, psd_layer_effects_stroke);
    CONN_UNITF("/FrFX/Sz  ", "#Pxl", setSize, stroke, psd_layer_effects_stroke);

    QMap<QString, psd_stroke_position> strokeStyleMap;
    strokeStyleMap.insert("OutF", psd_stroke_outside);
    strokeStyleMap.insert("InsF", psd_stroke_inside);
    strokeStyleMap.insert("CtrF", psd_stroke_center);
    CONN_ENUM("/FrFX/Styl", "FStl", setPosition, strokeStyleMap, psd_stroke_position, stroke, psd_layer_effects_stroke);

    QMap<QString, psd_fill_type> strokeFillType;
    strokeFillType.insert("SClr", psd_fill_solid_color);
    strokeFillType.insert("GrFl", psd_fill_gradient);
    strokeFillType.insert("Ptrn", psd_fill_pattern);
    CONN_ENUM("/FrFX/PntT", "FrFl", setFillType, strokeFillType, psd_fill_type, stroke, psd_layer_effects_stroke);

    CorrectFillTypeOnExitWithPattern<psd_layer_effects_stroke> strokeFillTypeDelayedFiller(stroke);

    // Color type
    CONN_COLOR("/FrFX/Clr ", setColor, stroke, psd_layer_effects_stroke);

    // Gradient Type
    CONN_GRADIENT("/FrFX/Grad", setGradient, stroke, psd_layer_effects_stroke);
    CONN_UNITF("/FrFX/Angl", "#Ang", setAngle, stroke, psd_layer_effects_stroke);
    CONN_UNITF("/FrFX/Scl ", "#Prc", setScale, stroke, psd_layer_effects_stroke);
    CONN_ENUM("/FrFX/Type", "GrdT", setStyle, gradientStyleMap, psd_gradient_style, stroke, psd_layer_effects_stroke);
    CONN_BOOL("/FrFX/Rvrs", setReverse, stroke, psd_layer_effects_stroke);
    CONN_BOOL("/FrFX/Algn", setAlignWithLayer, stroke, psd_layer_effects_stroke);
    CONN_POINT("/FrFX/Ofst", setGradientOffset, stroke, psd_layer_effects_stroke);
    // CONN_BOOL("/FrFX/Dthr", setDitherNotImplemented, stroke, psd_layer_effects_stroke);

    // Pattern type

    CONN_PATTERN("/FrFX/Ptrn", setPattern, stroke, psd_layer_effects_stroke);
    CONN_BOOL("/FrFX/Lnkd", setAlignWithLayer, stroke, psd_layer_effects_stroke); // yes, we share the params...
    CONN_POINT("/FrFX/phase", setPatternPhase, stroke, psd_layer_effects_stroke);


    psd_layer_effects_bevel_emboss *bevelAndEmboss = m_style->bevelAndEmboss();

    CONN_BOOL("/ebbl/enab", setEffectEnabled, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_COMPOSITE_OP("/ebbl/hglM", setHighlightBlendMode, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_COLOR("/ebbl/hglC", setHighlightColor, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_UNITF("/ebbl/hglO", "#Prc", setHighlightOpacity, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_COMPOSITE_OP("/ebbl/sdwM", setShadowBlendMode, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_COLOR("/ebbl/sdwC", setShadowColor, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_UNITF("/ebbl/sdwO", "#Prc", setShadowOpacity, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    QMap<QString, psd_technique_type> bevelTechniqueMap;
    bevelTechniqueMap.insert("PrBL", psd_technique_precise);
    bevelTechniqueMap.insert("SfBL", psd_technique_softer);
    bevelTechniqueMap.insert("Slmt", psd_technique_slope_limit);
    CONN_ENUM("/ebbl/bvlT", "bvlT", setTechnique, bevelTechniqueMap, psd_technique_type, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    QMap<QString, psd_bevel_style> bevelStyleMap;
    bevelStyleMap.insert("OtrB", psd_bevel_outer_bevel);
    bevelStyleMap.insert("InrB", psd_bevel_inner_bevel);
    bevelStyleMap.insert("Embs", psd_bevel_emboss);
    bevelStyleMap.insert("PlEb", psd_bevel_pillow_emboss);
    bevelStyleMap.insert("strokeEmboss", psd_bevel_stroke_emboss);
    CONN_ENUM("/ebbl/bvlS", "BESl", setStyle, bevelStyleMap, psd_bevel_style, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_BOOL("/ebbl/uglg", setUseGlobalLight, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_UNITF("/ebbl/lagl", "#Ang", setAngle, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_UNITF("/ebbl/Lald", "#Ang", setAltitude, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_UNITF("/ebbl/srgR", "#Prc", setDepth, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_UNITF("/ebbl/blur", "#Pxl", setSize, bevelAndEmboss, psd_layer_effects_bevel_emboss);


    QMap<QString, psd_direction> bevelDirectionMap;
    bevelDirectionMap.insert("In  ", psd_direction_up);
    bevelDirectionMap.insert("Out ", psd_direction_down);
    CONN_ENUM("/ebbl/bvlD", "BESs", setDirection, bevelDirectionMap, psd_direction, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_CURVE("/ebbl/TrnS", setContourLookupTable, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_BOOL("/ebbl/antialiasGloss", setGlossAntiAliased, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    CONN_UNITF("/ebbl/Sftn", "#Pxl", setSoften, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    // Use shape mode

    CONN_BOOL("/ebbl/useShape", setContourEnabled, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_CURVE("/ebbl/MpgS", setGlossContourLookupTable, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_BOOL("/ebbl/AntA", setAntiAliased, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_UNITF("/ebbl/Inpr", "#Prc", setContourRange, bevelAndEmboss, psd_layer_effects_bevel_emboss);

    // Use texture mode

    CONN_BOOL("/ebbl/useTexture", setTextureEnabled, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_BOOL("/ebbl/InvT", setTextureInvert, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_BOOL("/ebbl/Algn", setTextureAlignWithLayer, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_UNITF("/ebbl/Scl ", "#Prc", setTextureScale, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_UNITF("/ebbl/textureDepth", "#Prc", setTextureDepth, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_PATTERN("/ebbl/Ptrn", setTexturePattern, bevelAndEmboss, psd_layer_effects_bevel_emboss);
    CONN_POINT("/ebbl/phase", setTexturePhase, bevelAndEmboss, psd_layer_effects_bevel_emboss);


    KisAslReader reader;
    QDomDocument doc = reader.readFile(device);

    qDebug() << ppVar(doc.toString());

    //KisAslObjectCatcher c2;
    KisAslXmlParser parser;
    parser.parseXML(doc, c);
}
