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
#include <KoSegmentGradient.h>
#include <KoPattern.h>


#include "psd.h"
#include "kis_global.h"
#include "kis_psd_layer_style.h"
#include "kis_embedded_pattern_manager.h"

#include "kis_asl_reader.h"
#include "kis_asl_xml_parser.h"
#include "kis_asl_callback_object_catcher.h"
#include "kis_asl_writer_utils.h"

#include "kis_asl_xml_writer.h"
#include "kis_asl_writer.h"


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

QString compositeOpToBlendMode(const QString &compositeOp)
{
    QString mode = "Nrml";

    if (compositeOp == COMPOSITE_OVER) {
        mode = "Nrml";
    } else if (compositeOp == COMPOSITE_DISSOLVE) {
        mode = "Dslv";
    } else if (compositeOp == COMPOSITE_DARKEN) {
        mode = "Drkn";
    } else if (compositeOp == COMPOSITE_MULT) {
        mode = "Mltp";
    } else if (compositeOp == COMPOSITE_BURN) {
        mode = "CBrn";
    } else if (compositeOp == COMPOSITE_LINEAR_BURN) {
        mode = "linearBurn";
    } else if (compositeOp == COMPOSITE_DARKER_COLOR) {
        mode = "darkerColor";
    } else if (compositeOp == COMPOSITE_LIGHTEN) {
        mode = "Lghn";
    } else if (compositeOp == COMPOSITE_SCREEN) {
        mode = "Scrn";
    } else if (compositeOp == COMPOSITE_DODGE) {
        mode = "CDdg";
    } else if (compositeOp == COMPOSITE_LINEAR_DODGE) {
        mode = "linearDodge";
    } else if (compositeOp == COMPOSITE_LIGHTER_COLOR) {
        mode = "lighterColor";
    } else if (compositeOp == COMPOSITE_OVERLAY) {
        mode = "Ovrl";
    } else if (compositeOp == COMPOSITE_SOFT_LIGHT_PHOTOSHOP) {
        mode = "SftL";
    } else if (compositeOp == COMPOSITE_HARD_LIGHT) {
        mode = "HrdL";
    } else if (compositeOp == COMPOSITE_VIVID_LIGHT) {
        mode = "vividLight";
    } else if (compositeOp == COMPOSITE_LINEAR_LIGHT) {
        mode = "linearLight";
    } else if (compositeOp == COMPOSITE_PIN_LIGHT) {
        mode = "pinLight";
    } else if (compositeOp == COMPOSITE_HARD_MIX) {
        mode = "hardMix";
    } else if (compositeOp == COMPOSITE_DIFF) {
        mode = "Dfrn";
    } else if (compositeOp == COMPOSITE_EXCLUSION) {
        mode = "Xclu";
    } else if (compositeOp == COMPOSITE_SUBTRACT) {
        mode = "Sbtr";
    } else if (compositeOp == COMPOSITE_DIVIDE) {
        mode = "divide";
    } else if (compositeOp == COMPOSITE_HUE) {
        mode = "H   ";
    } else if (compositeOp == COMPOSITE_SATURATION) {
        mode = "Strt";
    } else if (compositeOp == COMPOSITE_COLOR) {
        mode = "Clr ";
    } else if (compositeOp == COMPOSITE_LUMINIZE) {
        mode = "Lmns";
    } else {
        qDebug() << "Unknown composite op:" << mode << "Returning \"Nrml\"!";
    }

    return mode;
}

QString techniqueToString(psd_technique_type technique, const QString &typeId)
{
    QString result = "SfBL";

    switch (technique) {
    case psd_technique_softer:
        result = "SfBL";
        break;
    case psd_technique_precise:
        result = "PrBL";
        break;
    case psd_technique_slope_limit:
        result = "Slmt";
        break;
    }

    if (typeId == "BETE" && technique == psd_technique_slope_limit) {
        qWarning() << "WARNING: techniqueToString: invalid technique type!" << ppVar(technique) << ppVar(typeId);
    }

    return result;
}

QString bevelStyleToString(psd_bevel_style style)
{
    QString result = "OtrB";

    switch (style) {
    case psd_bevel_outer_bevel:
        result = "OtrB";
        break;
    case psd_bevel_inner_bevel:
        result = "InrB";
        break;
    case psd_bevel_emboss:
        result = "Embs";
        break;
    case psd_bevel_pillow_emboss:
        result = "PlEb";
        break;
    case psd_bevel_stroke_emboss:
        result = "strokeEmboss";
        break;
    }

    return result;
}

QString gradientTypeToString(psd_gradient_style style)
{
    QString result = "Lnr ";

    switch (style) {
    case psd_gradient_style_linear:
        result = "Lnr ";
        break;
    case psd_gradient_style_radial:
        result = "Rdl ";
        break;
    case psd_gradient_style_angle:
        result = "Angl";
        break;
    case psd_gradient_style_reflected:
        result = "Rflc";
        break;
    case psd_gradient_style_diamond:
        result = "Dmnd";
        break;
    }

    return result;
}

QString strokePositionToString(psd_stroke_position position)
{
    QString result = "OutF";

    switch (position) {
    case psd_stroke_outside:
        result = "OutF";
        break;
    case psd_stroke_inside:
        result = "InsF";
        break;
    case psd_stroke_center:
        result = "CtrF";
        break;
    }

    return result;
}

QString strokeFillTypeToString(psd_fill_type position)
{
    QString result = "SClr";

    switch (position) {
    case psd_fill_solid_color:
        result = "SClr";
        break;
    case psd_fill_gradient:
        result = "GrFl";
        break;
    case psd_fill_pattern:
        result = "Ptrn";
        break;
    }

    return result;
}

QVector<KoPattern*> KisAslLayerStyleSerializer::fetchAllPatterns(KisPSDLayerStyle *style)
{
    QVector <KoPattern*> allPatterns;

    if (style->patternOverlay()->effectEnabled()) {
        allPatterns << style->patternOverlay()->pattern();
    }

    if (style->stroke()->effectEnabled() &&
        style->stroke()->fillType() == psd_fill_pattern) {

        allPatterns << style->stroke()->pattern();
    }

    if(style->bevelAndEmboss()->effectEnabled() &&
       style->bevelAndEmboss()->textureEnabled()) {

        allPatterns << style->bevelAndEmboss()->texturePattern();
    }

    return allPatterns;
}

QString fetchPatternUuidSafe(KoPattern *pattern, QHash<KoPattern*, QString> patternToUuid)
{
    if (patternToUuid.contains(pattern)) {
        return patternToUuid[pattern];
    } else {
        qWarning() << "WARNING: the pattern is not present in the Uuid map!";
        return "invalid-uuid";
    }
}


void KisAslLayerStyleSerializer::saveToDevice(QIODevice *device)
{
    QVector<KoPattern*> allPatterns = fetchAllPatterns(m_style);

    QHash<KoPattern*, QString> patternToUuidMap;

    KisAslXmlWriter w;

    if (!allPatterns.isEmpty()) {
        w.enterList("Patterns");

        foreach (KoPattern *pattern, allPatterns) {
            if (!patternToUuidMap.contains(pattern)) {
                QString uuid = w.writePattern("", pattern);
                patternToUuidMap.insert(pattern, uuid);
            }
        }

        w.leaveList();
    }

    w.enterDescriptor("", "", "null");
    w.writeText("Nm  ", m_style->name());
    w.writeText("Idnt", m_style->uuid());
    w.leaveDescriptor();

    w.enterDescriptor("", "", "Styl");

    w.enterDescriptor("documentMode", "", "documentMode");
    w.leaveDescriptor();

    w.enterDescriptor("Lefx", "", "Lefx");

    w.writeUnitFloat("Scl ", "#Prc", 100);
    w.writeBoolean("masterFXSwitch", true);


    // Drop Shadow
    const psd_layer_effects_drop_shadow *dropShadow = m_style->dropShadow();
    if (dropShadow->effectEnabled()) {
        w.enterDescriptor("DrSh", "", "DrSh");

        w.writeBoolean("enab", dropShadow->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(dropShadow->blendMode()));
        w.writeColor("Clr ", dropShadow->color());

        w.writeUnitFloat("Opct", "#Prc", dropShadow->opacity());
        w.writeBoolean("uglg", dropShadow->useGlobalLight());
        w.writeUnitFloat("lagl", "#Ang", dropShadow->angle());
        w.writeUnitFloat("Dstn", "#Pxl", dropShadow->distance());
        w.writeUnitFloat("Ckmt", "#Pxl", dropShadow->spread());
        w.writeUnitFloat("blur", "#Pxl", dropShadow->size());
        w.writeUnitFloat("Nose", "#Prc", dropShadow->noise());

        w.writeBoolean("AntA", dropShadow->antiAliased());

        // FIXME: save curves
        w.writeCurve("TrnS",
                     "Linear",
                     QVector<QPointF>() << QPointF() << QPointF(255, 255));

        w.writeBoolean("layerConceals", dropShadow->knocksOut());
        w.leaveDescriptor();
    }

    // Inner Shadow
    const psd_layer_effects_inner_shadow *innerShadow = m_style->innerShadow();
    if (innerShadow->effectEnabled()) {
        w.enterDescriptor("IrSh", "", "IrSh");

        w.writeBoolean("enab", innerShadow->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(innerShadow->blendMode()));
        w.writeColor("Clr ", innerShadow->color());

        w.writeUnitFloat("Opct", "#Prc", innerShadow->opacity());
        w.writeBoolean("uglg", innerShadow->useGlobalLight());
        w.writeUnitFloat("lagl", "#Ang", innerShadow->angle());
        w.writeUnitFloat("Dstn", "#Pxl", innerShadow->distance());
        w.writeUnitFloat("Ckmt", "#Pxl", innerShadow->spread());
        w.writeUnitFloat("blur", "#Pxl", innerShadow->size());
        w.writeUnitFloat("Nose", "#Prc", innerShadow->noise());

        w.writeBoolean("AntA", innerShadow->antiAliased());

        // FIXME: save curves
        w.writeCurve("TrnS",
                     "Linear",
                     QVector<QPointF>() << QPointF() << QPointF(255, 255));

        w.leaveDescriptor();
    }

    // Outer Glow
    const psd_layer_effects_outer_glow *outerGlow = m_style->outerGlow();
    if (outerGlow->effectEnabled()) {
        w.enterDescriptor("OrGl", "", "OrGl");

        w.writeBoolean("enab", outerGlow->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(outerGlow->blendMode()));


        if (outerGlow->fillType() == psd_fill_gradient && outerGlow->gradient()) {
            KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(outerGlow->gradient());

            if (segmentGradient) {
                w.writeGradient("Grad", segmentGradient);
            } else {
                qWarning() << "WARNING: OG: FIXME: saving stop-gradients is not supported yet, please convert them into segment gradients first!";
                w.writeColor("Clr ", outerGlow->color());
            }

        } else {
            w.writeColor("Clr ", outerGlow->color());
        }

        w.writeUnitFloat("Opct", "#Prc", outerGlow->opacity());

        w.writeEnum("GlwT", "BETE", techniqueToString(outerGlow->technique(), "BETE"));

        w.writeUnitFloat("Ckmt", "#Pxl", outerGlow->spread());
        w.writeUnitFloat("blur", "#Pxl", outerGlow->size());
        w.writeUnitFloat("Nose", "#Prc", outerGlow->noise());

        w.writeUnitFloat("ShdN", "#Prc", outerGlow->jitter());

        w.writeBoolean("AntA", outerGlow->antiAliased());

        // FIXME: save curves
        w.writeCurve("TrnS",
                     "Linear",
                     QVector<QPointF>() << QPointF() << QPointF(255, 255));

        w.writeUnitFloat("Inpr", "#Prc", outerGlow->range());

        w.leaveDescriptor();
    }

    // Inner Glow
    const psd_layer_effects_inner_glow *innerGlow = m_style->innerGlow();
    if (innerGlow->effectEnabled()) {
        w.enterDescriptor("IrGl", "", "IrGl");

        w.writeBoolean("enab", innerGlow->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(innerGlow->blendMode()));


        if (innerGlow->fillType() == psd_fill_gradient && innerGlow->gradient()) {
            KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(innerGlow->gradient());

            if (segmentGradient) {
                w.writeGradient("Grad", segmentGradient);
            } else {
                qWarning() << "WARNING: IG: FIXME: saving stop-gradients is not supported yet, please convert them into segment gradients first!";
                w.writeColor("Clr ", innerGlow->color());
            }

        } else {
            w.writeColor("Clr ", innerGlow->color());
        }

        w.writeUnitFloat("Opct", "#Prc", innerGlow->opacity());

        w.writeEnum("GlwT", "BETE", techniqueToString(innerGlow->technique(), "BETE"));

        w.writeUnitFloat("Ckmt", "#Pxl", innerGlow->spread());
        w.writeUnitFloat("blur", "#Pxl", innerGlow->size());

        // NOTE: order is swapped in ASL!
        w.writeUnitFloat("ShdN", "#Prc", innerGlow->jitter());
        w.writeUnitFloat("Nose", "#Prc", innerGlow->noise());

        w.writeBoolean("AntA", innerGlow->antiAliased());

        w.writeEnum("glwS", "IGSr", innerGlow->source() == psd_glow_center ? "SrcC" : "SrcE");

        // FIXME: save curves
        w.writeCurve("TrnS",
                     "Linear",
                     QVector<QPointF>() << QPointF() << QPointF(255, 255));

        w.writeUnitFloat("Inpr", "#Prc", innerGlow->range());

        w.leaveDescriptor();
    }

    // Bevel and Emboss
    const psd_layer_effects_bevel_emboss *bevelAndEmboss = m_style->bevelAndEmboss();
    if (bevelAndEmboss->effectEnabled()) {
        w.enterDescriptor("ebbl", "", "ebbl");

        w.writeBoolean("enab", bevelAndEmboss->effectEnabled());

        w.writeEnum("hglM", "BlnM", compositeOpToBlendMode(bevelAndEmboss->highlightBlendMode()));
        w.writeColor("hglC", bevelAndEmboss->highlightColor());
        w.writeUnitFloat("hglO", "#Prc", bevelAndEmboss->highlightOpacity());

        w.writeEnum("sdwM", "BlnM", compositeOpToBlendMode(bevelAndEmboss->shadowBlendMode()));
        w.writeColor("sdwC", bevelAndEmboss->shadowColor());
        w.writeUnitFloat("sdwO", "#Prc", bevelAndEmboss->shadowOpacity());

        w.writeEnum("bvlT", "bvlT", techniqueToString(bevelAndEmboss->technique(), "bvlT"));
        w.writeEnum("bvlS", "BESl", bevelStyleToString(bevelAndEmboss->style()));

        w.writeBoolean("uglg", bevelAndEmboss->useGlobalLight());
        w.writeUnitFloat("lagl", "#Ang", bevelAndEmboss->angle());
        w.writeUnitFloat("Lald", "#Ang", bevelAndEmboss->altitude());

        w.writeUnitFloat("srgR", "#Prc", bevelAndEmboss->depth());
        w.writeUnitFloat("blur", "#Pxl", bevelAndEmboss->size());
        w.writeEnum("bvlD", "BESs", bevelAndEmboss->direction() == psd_direction_up ? "In  " : "Out ");

        // FIXME: save curves
        w.writeCurve("TrnS",
                     "Linear",
                     QVector<QPointF>() << QPointF() << QPointF(255, 255));

        w.writeBoolean("antialiasGloss", bevelAndEmboss->glossAntiAliased());

        w.writeUnitFloat("Sftn", "#Pxl", bevelAndEmboss->soften());

        if (bevelAndEmboss->contourEnabled()) {
            w.writeBoolean("useShape", bevelAndEmboss->contourEnabled());

            // FIXME: save curves
            w.writeCurve("MpgS",
                         "Linear",
                         QVector<QPointF>() << QPointF() << QPointF(255, 255));

            w.writeBoolean("AntA", bevelAndEmboss->antiAliased());
            w.writeUnitFloat("Inpr", "#Prc", bevelAndEmboss->contourRange());
        }

        w.writeBoolean("useTexture", bevelAndEmboss->textureEnabled());

        if (bevelAndEmboss->textureEnabled()) {
            w.writeBoolean("InvT", bevelAndEmboss->textureInvert());
            w.writeBoolean("Algn", bevelAndEmboss->textureAlignWithLayer());
            w.writeUnitFloat("Scl ", "#Prc", bevelAndEmboss->textureScale());
            w.writeUnitFloat("textureDepth ", "#Prc", bevelAndEmboss->textureDepth());
            w.writePatternRef("Ptrn", bevelAndEmboss->texturePattern(), fetchPatternUuidSafe(bevelAndEmboss->texturePattern(), patternToUuidMap));
            w.writePhasePoint("phase", bevelAndEmboss->texturePhase());
        }

        w.leaveDescriptor();
    }

    // Satin
    const psd_layer_effects_satin *satin = m_style->satin();
    if (satin->effectEnabled()) {
        w.enterDescriptor("ChFX", "", "ChFX");

        w.writeBoolean("enab", satin->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(satin->blendMode()));
        w.writeColor("Clr ", satin->color());

        w.writeBoolean("AntA", satin->antiAliased());
        w.writeBoolean("Invr", satin->invert());
        w.writeUnitFloat("Opct", "#Prc", satin->opacity());
        w.writeUnitFloat("lagl", "#Ang", satin->angle());
        w.writeUnitFloat("Dstn", "#Pxl", satin->distance());
        w.writeUnitFloat("blur", "#Pxl", satin->size());

        // FIXME: save curves
        w.writeCurve("MpgS",
                     "Linear",
                     QVector<QPointF>() << QPointF() << QPointF(255, 255));

        w.leaveDescriptor();
    }

    const psd_layer_effects_color_overlay *colorOverlay = m_style->colorOverlay();
    if (colorOverlay->effectEnabled()) {
        w.enterDescriptor("SoFi", "", "SoFi");

        w.writeBoolean("enab", colorOverlay->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(colorOverlay->blendMode()));
        w.writeUnitFloat("Opct", "#Prc", colorOverlay->opacity());
        w.writeColor("Clr ", colorOverlay->color());

        w.leaveDescriptor();
    }

    // Gradient Overlay
    const psd_layer_effects_gradient_overlay *gradientOverlay = m_style->gradientOverlay();
    KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(gradientOverlay->gradient());
    if (!segmentGradient) {
        qWarning() << "WARNING: FIXME: saving stop-gradients is not supported yet, please convert them into segment gradients first! Gradient Overlay style is skipped!";
    }

    if (gradientOverlay->effectEnabled() && segmentGradient) {
        w.enterDescriptor("GrFl", "", "GrFl");

        w.writeBoolean("enab", gradientOverlay->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(gradientOverlay->blendMode()));
        w.writeUnitFloat("Opct", "#Prc", gradientOverlay->opacity());

        w.writeGradient("Grad", segmentGradient);

        w.writeUnitFloat("Angl", "#Ang", gradientOverlay->angle());

        w.writeEnum("Type", "GrdT", gradientTypeToString(gradientOverlay->style()));

        w.writeBoolean("Rvrs", gradientOverlay->reverse());
        w.writeBoolean("Algn", gradientOverlay->alignWithLayer());
        w.writeUnitFloat("Scl ", "#Prc", gradientOverlay->scale());

        w.writeOffsetPoint("Ofst", gradientOverlay->gradientOffset());

        // FIXME: Krita doesn't support dithering
        w.writeBoolean("Dthr", true/*gradientOverlay->dither()*/);

        w.leaveDescriptor();
    }

    // Pattern Overlay
    const psd_layer_effects_pattern_overlay *patternOverlay = m_style->patternOverlay();
    if (patternOverlay->effectEnabled()) {
        w.enterDescriptor("patternFill", "", "patternFill");

        w.writeBoolean("enab", patternOverlay->effectEnabled());
        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(patternOverlay->blendMode()));
        w.writeUnitFloat("Opct", "#Prc", patternOverlay->opacity());

        w.writePatternRef("Ptrn", patternOverlay->pattern(), fetchPatternUuidSafe(patternOverlay->pattern(), patternToUuidMap));

        w.writeUnitFloat("Scl ", "#Prc", patternOverlay->scale());
        w.writeBoolean("Algn", patternOverlay->alignWithLayer());
        w.writePhasePoint("phase", patternOverlay->patternPhase());

        w.leaveDescriptor();
    }

    const psd_layer_effects_stroke *stroke = m_style->stroke();
    if (stroke->effectEnabled()) {
        w.enterDescriptor("FrFX", "", "FrFX");

        w.writeBoolean("enab", stroke->effectEnabled());

        w.writeEnum("Styl", "FStl", strokePositionToString(stroke->position()));
        w.writeEnum("PntT", "FrFl", strokeFillTypeToString(stroke->fillType()));

        w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(stroke->blendMode()));
        w.writeUnitFloat("Opct", "#Prc", stroke->opacity());

        w.writeUnitFloat("Sz  ", "#Pxl", stroke->size());

        if (stroke->fillType() == psd_fill_solid_color) {
            w.writeColor("Clr ", stroke->color());
        } else if (stroke->fillType() == psd_fill_gradient) {
            KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(stroke->gradient());

            if (segmentGradient) {
                w.writeGradient("Grad", segmentGradient);
            } else {
                qWarning() << "WARNING: FIXME: saving stop-gradients is not supported yet, please convert them into segment gradients first!";
                w.writeColor("Clr ", stroke->color());
            }

            w.writeUnitFloat("Angl", "#Ang", stroke->angle());
            w.writeEnum("Type", "GrdT", gradientTypeToString(stroke->style()));

            w.writeBoolean("Rvrs", stroke->reverse());
            w.writeUnitFloat("Scl ", "#Prc", stroke->scale());
            w.writeBoolean("Algn", stroke->alignWithLayer());
            w.writeOffsetPoint("Ofst", stroke->gradientOffset());

            // FIXME: Krita doesn't support dithering
            w.writeBoolean("Dthr", true/*stroke->dither()*/);

        } else if (stroke->fillType() == psd_fill_pattern) {
            w.writePatternRef("Ptrn", stroke->pattern(), fetchPatternUuidSafe(stroke->pattern(), patternToUuidMap));
            w.writeUnitFloat("Scl ", "#Prc", stroke->scale());
            w.writeBoolean("Lnkd", stroke->alignWithLayer());
            w.writePhasePoint("phase", stroke->patternPhase());
        }

        w.leaveDescriptor();
    }

    w.leaveDescriptor();
    w.leaveDescriptor();

    KisAslWriter writer;
    writer.writeFile(device, w.document());
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

void cloneAndSetGradient(const KoAbstractGradient *resource,
                         boost::function<void (KoAbstractGradient*)> setResource)
{
    QByteArray md5 = resource->md5();
    KoAbstractGradient *newResource = KisEmbeddedPatternManager::tryFetchGradientByMd5(md5);

    if (!newResource) {
        newResource = resource->clone();
        KoResourceServer<KoAbstractGradient> *server = KoResourceServerProvider::instance()->gradientServer();
        server->addResource(newResource, false);
    }

    setResource(newResource);
}

inline QString _prepaddr(const QString &addr) {
    return QString("/Styl/Lefx") + addr;
}

#define CONN_TEXT_RADDR(addr, method, object, type) c.subscribeText(addr, boost::bind(&type::method, object, _1))
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
        c.subscribeGradient(_prepaddr(addr), boost::bind(cloneAndSetGradient, _1, setter)); \
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
    m_style->clear();

    psd_layer_effects_context *context = m_style->context();
    context->global_angle = 0;
    context->keep_original = 0;

    KisAslCallbackObjectCatcher c;
    c.subscribePattern("/Patterns/KisPattern", boost::bind(&KisAslLayerStyleSerializer::registerPatternObject, this, _1));

    CONN_TEXT_RADDR("/null/Nm  ", setName, m_style, KisPSDLayerStyle);
    CONN_TEXT_RADDR("/null/Idnt", setUuid, m_style, KisPSDLayerStyle);

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

    //qDebug() << ppVar(doc.toString());

    //KisAslObjectCatcher c2;
    KisAslXmlParser parser;
    parser.parseXML(doc, c);
}
