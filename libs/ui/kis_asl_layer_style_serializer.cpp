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

#include <QDomDocument>

#include <KoResourceServerProvider.h>
#include <resources/KoAbstractGradient.h>
#include <resources/KoSegmentGradient.h>
#include <resources/KoStopGradient.h>
#include <resources/KoPattern.h>

#include "kis_dom_utils.h"


#include "psd.h"
#include "kis_global.h"

#include "asl/kis_asl_reader.h"
#include "asl/kis_asl_xml_parser.h"
#include "asl/kis_asl_writer_utils.h"

#include "asl/kis_asl_xml_writer.h"
#include "asl/kis_asl_writer.h"

#include <functional>
using namespace std::placeholders;

KisAslLayerStyleSerializer::KisAslLayerStyleSerializer()
{
}

KisAslLayerStyleSerializer::~KisAslLayerStyleSerializer()
{
}

QVector<KisPSDLayerStyleSP> KisAslLayerStyleSerializer::styles() const
{
    return m_stylesVector;
}

void KisAslLayerStyleSerializer::setStyles(const QVector<KisPSDLayerStyleSP> &styles)
{
    m_stylesVector = styles;
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
    } else if (compositeOp == COMPOSITE_HARD_MIX_PHOTOSHOP) {
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
        dbgKrita << "Unknown composite op:" << mode << "Returning \"Nrml\"!";
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
        warnKrita << "WARNING: techniqueToString: invalid technique type!" << ppVar(technique) << ppVar(typeId);
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

QVector<KoPatternSP> KisAslLayerStyleSerializer::fetchAllPatterns(KisPSDLayerStyle *style) const
{
    QVector <KoPatternSP> allPatterns;

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

QString fetchPatternUuidSafe(KoPatternSP pattern, QHash<KoPatternSP, QString> patternToUuid)
{
    if (patternToUuid.contains(pattern)) {
        return patternToUuid[pattern];
    } else {
        warnKrita << "WARNING: the pattern is not present in the Uuid map!";
        return "invalid-uuid";
    }
}

QDomDocument KisAslLayerStyleSerializer::formXmlDocument() const
{
    KIS_ASSERT_RECOVER(!m_stylesVector.isEmpty()) { return QDomDocument(); }

    QVector<KoPatternSP> allPatterns;

    Q_FOREACH (KisPSDLayerStyleSP style, m_stylesVector) {
        allPatterns += fetchAllPatterns(style.data());
    }

    QHash<KoPatternSP, QString> patternToUuidMap;

    KisAslXmlWriter w;

    if (!allPatterns.isEmpty()) {
        w.enterList(ResourceType::Patterns);

        Q_FOREACH (KoPatternSP pattern, allPatterns) {
            if (pattern) {
                if (!patternToUuidMap.contains(pattern)) {
                    QString uuid = w.writePattern("", pattern);
                    patternToUuidMap.insert(pattern, uuid);
                }
            } else {
                warnKrita << "WARNING: KisAslLayerStyleSerializer::saveToDevice: saved pattern is null!";
            }
        }

        w.leaveList();
    }

    Q_FOREACH (KisPSDLayerStyleSP style, m_stylesVector) {

        w.enterDescriptor("", "", "null");
        w.writeText("Nm  ", style->name());
        w.writeText("Idnt", style->psdUuid());
        w.leaveDescriptor();

        w.enterDescriptor("", "", "Styl");

        w.enterDescriptor("documentMode", "", "documentMode");
        w.leaveDescriptor();

        w.enterDescriptor("Lefx", "", "Lefx");

        w.writeUnitFloat("Scl ", "#Prc", 100);
        w.writeBoolean("masterFXSwitch", style->isEnabled());


        // Drop Shadow
        const psd_layer_effects_drop_shadow *dropShadow = style->dropShadow();
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
        const psd_layer_effects_inner_shadow *innerShadow = style->innerShadow();
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
        const psd_layer_effects_outer_glow *outerGlow = style->outerGlow();
        if (outerGlow->effectEnabled()) {
            w.enterDescriptor("OrGl", "", "OrGl");

            w.writeBoolean("enab", outerGlow->effectEnabled());
            w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(outerGlow->blendMode()));


            if (outerGlow->fillType() == psd_fill_gradient && outerGlow->gradient()) {
                KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(outerGlow->gradient().data());
                KoStopGradient *stopGradient = dynamic_cast<KoStopGradient*>(outerGlow->gradient().data());

                if (segmentGradient) {
                    w.writeSegmentGradient("Grad", segmentGradient);
                } else if (stopGradient) {
                    w.writeStopGradient("Grad", stopGradient);
                } else {
                    warnKrita << "WARNING: OG: Unknown gradient type!";
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
        const psd_layer_effects_inner_glow *innerGlow = style->innerGlow();
        if (innerGlow->effectEnabled()) {
            w.enterDescriptor("IrGl", "", "IrGl");

            w.writeBoolean("enab", innerGlow->effectEnabled());
            w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(innerGlow->blendMode()));


            if (innerGlow->fillType() == psd_fill_gradient && innerGlow->gradient()) {
                KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(innerGlow->gradient().data());
                KoStopGradient *stopGradient = dynamic_cast<KoStopGradient*>(innerGlow->gradient().data());

                if (segmentGradient) {
                    w.writeSegmentGradient("Grad", segmentGradient);
                } else if (stopGradient) {
                    w.writeStopGradient("Grad", stopGradient);
                } else {
                    warnKrita << "WARNING: IG: Unknown gradient type!";
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
        const psd_layer_effects_bevel_emboss *bevelAndEmboss = style->bevelAndEmboss();
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
        const psd_layer_effects_satin *satin = style->satin();
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

        const psd_layer_effects_color_overlay *colorOverlay = style->colorOverlay();
        if (colorOverlay->effectEnabled()) {
            w.enterDescriptor("SoFi", "", "SoFi");

            w.writeBoolean("enab", colorOverlay->effectEnabled());
            w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(colorOverlay->blendMode()));
            w.writeUnitFloat("Opct", "#Prc", colorOverlay->opacity());
            w.writeColor("Clr ", colorOverlay->color());

            w.leaveDescriptor();
        }

        // Gradient Overlay
        const psd_layer_effects_gradient_overlay *gradientOverlay = style->gradientOverlay();
        KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(gradientOverlay->gradient().data());
        KoStopGradient *stopGradient = dynamic_cast<KoStopGradient*>(gradientOverlay->gradient().data());

        if (gradientOverlay->effectEnabled() && (segmentGradient || stopGradient)) {
            w.enterDescriptor("GrFl", "", "GrFl");

            w.writeBoolean("enab", gradientOverlay->effectEnabled());
            w.writeEnum("Md  ", "BlnM", compositeOpToBlendMode(gradientOverlay->blendMode()));
            w.writeUnitFloat("Opct", "#Prc", gradientOverlay->opacity());

            if (segmentGradient) {
                w.writeSegmentGradient("Grad", segmentGradient);
            } else if (stopGradient) {
                w.writeStopGradient("Grad", stopGradient);
            }

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
        const psd_layer_effects_pattern_overlay *patternOverlay = style->patternOverlay();
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

        const psd_layer_effects_stroke *stroke = style->stroke();
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
                KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(stroke->gradient().data());
                KoStopGradient *stopGradient = dynamic_cast<KoStopGradient*>(stroke->gradient().data());

                if (segmentGradient) {
                    w.writeSegmentGradient("Grad", segmentGradient);
                } else if (stopGradient) {
                    w.writeStopGradient("Grad", stopGradient);
                } else {
                    warnKrita << "WARNING: Stroke: Unknown gradient type!";
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
    }

    return w.document();
}

inline QDomNode findNodeByClassId(const QString &classId, QDomNode parent) {
    return KisDomUtils::findElementByAttibute(parent, "node", "classId", classId);
}

void replaceAllChildren(QDomNode src, QDomNode dst)
{
    QDomNode node;

    do {
        node = dst.lastChild();
        dst.removeChild(node);

    } while(!node.isNull());


    node = src.firstChild();
    while(!node.isNull()) {
        dst.appendChild(node);
        node = src.firstChild();
    }

    src.parentNode().removeChild(src);
}

QDomDocument KisAslLayerStyleSerializer::formPsdXmlDocument() const
{
    QDomDocument doc = formXmlDocument();

    QDomNode nullNode = findNodeByClassId("null", doc.documentElement());
    QDomNode stylNode = findNodeByClassId("Styl", doc.documentElement());
    QDomNode lefxNode = findNodeByClassId("Lefx", stylNode);

    replaceAllChildren(lefxNode, nullNode);

    return doc;
}

void KisAslLayerStyleSerializer::saveToDevice(QIODevice *device)
{
    QDomDocument doc = formXmlDocument();
    if (doc.isNull()) return ;

    KisAslWriter writer;
    writer.writeFile(device, doc);
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
        compositeOp = COMPOSITE_HARD_MIX_PHOTOSHOP;
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
        dbgKrita << "Unknown blending mode:" << mode << "Returning COMPOSITE_OVER!";
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

    warnKrita << "convertAndSetBlendMode:" << "Curve conversion is not implemented yet";
}

template <typename T>
void convertAndSetEnum(const QString &value,
                       const QMap<QString, T> map,
                       boost::function<void (T)> setMappedValue)
{
    setMappedValue(map[value]);
}

inline QString _prepaddr(const QString &pref, const QString &addr) {
    return pref + addr;
}

#define CONN_TEXT_RADDR(addr, method, object, type) m_catcher.subscribeText(addr, std::bind(&type::method, object, _1))
#define CONN_COLOR(addr, method, object, type, prefix) m_catcher.subscribeColor(_prepaddr(prefix, addr), std::bind(&type::method, object, _1))
#define CONN_UNITF(addr, unit, method, object, type, prefix) m_catcher.subscribeUnitFloat(_prepaddr(prefix, addr), unit, std::bind(&type::method, object, _1))
#define CONN_BOOL(addr, method, object, type, prefix) m_catcher.subscribeBoolean(_prepaddr(prefix, addr), std::bind(&type::method, object, _1))

#define CONN_POINT(addr, method, object, type, prefix) m_catcher.subscribePoint(_prepaddr(prefix, addr), std::bind(&type::method, object, _1))

#define CONN_COMPOSITE_OP(addr, method, object, type, prefix)                   \
    {                                                                   \
        boost::function<void (const QString&)> setter =                 \
            std::bind(&type::method, object, _1);                     \
        m_catcher.subscribeEnum(_prepaddr(prefix, addr), "BlnM", std::bind(convertAndSetBlendMode, _1, setter)); \
    }

#define CONN_CURVE(addr, method, object, type, prefix)                          \
    {                                                                   \
        boost::function<void (const quint8*)> setter =                  \
            std::bind(&type::method, object, _1);                     \
        m_catcher.subscribeCurve(_prepaddr(prefix, addr), std::bind(convertAndSetCurve, _1, _2, setter)); \
    }

#define CONN_ENUM(addr, tag, method, map, mapped_type, object, type, prefix)                       \
    {                                                                   \
        boost::function<void (mapped_type)> setter =                  \
            std::bind(&type::method, object, _1);                     \
        m_catcher.subscribeEnum(_prepaddr(prefix, addr), tag, std::bind(convertAndSetEnum<mapped_type>, _1, map, setter)); \
    }

#define CONN_GRADIENT(addr, method, object, type, prefix)                      \
    {                                                                  \
        m_catcher.subscribeGradient(_prepaddr(prefix, addr), std::bind(&type::method, object, _1)); \
    }

#define CONN_PATTERN(addr, method, object, type, prefix)                       \
    {                                                                  \
        boost::function<void (KoPatternSP)> setter =    \
            std::bind(&type::method, object, _1);                     \
        m_catcher.subscribePatternRef(_prepaddr(prefix, addr), std::bind(&KisAslLayerStyleSerializer::assignPatternObject, this, _1, _2, setter)); \
    }

void KisAslLayerStyleSerializer::registerPatternObject(const KoPatternSP pattern) {
    QString uuid = KisAslWriterUtils::getPatternUuidLazy(pattern);

    if (m_patternsStore.contains(uuid)) {
        warnKrita << "WARNING: ASL style contains a duplicated pattern!" << ppVar(pattern->name()) << ppVar(m_patternsStore[uuid]->name());
    } else {
        KoResourceServer<KoPattern> *server = KoResourceServerProvider::instance()->patternServer();
        KoPatternSP patternToAdd = server->resourceByMD5(pattern->md5());

        if (!patternToAdd) {
            patternToAdd = pattern->clone().dynamicCast<KoPattern>();
            server->addResource(patternToAdd, false);
        }

        m_patternsStore.insert(uuid, patternToAdd);
    }
}

void KisAslLayerStyleSerializer::assignPatternObject(const QString &patternUuid,
                                                     const QString &patternName,
                                                     boost::function<void (KoPatternSP )> setPattern)
{
    Q_UNUSED(patternName);

    KoPatternSP pattern = m_patternsStore[patternUuid];

    if (!pattern) {
        warnKrita << "WARNING: ASL style contains non-existent pattern reference!";

        QImage dumbImage(32, 32, QImage::Format_ARGB32);
        dumbImage.fill(Qt::red);
        KoPatternSP dumbPattern(new KoPattern(dumbImage, "invalid", ""));
        registerPatternObject(dumbPattern);
        pattern = dumbPattern;
    }

    setPattern(pattern);
}

class FillStylesCorrector {
public:

    static void correct(KisPSDLayerStyle *style) {
        correctWithoutPattern(style->outerGlow());
        correctWithoutPattern(style->innerGlow());
        correctWithPattern(style->stroke());
    }

private:

    template <class T>
    static void correctWithPattern(T *config) {
        if (config->pattern()) {
            config->setFillType(psd_fill_pattern);
        } else if (config->gradient()) {
            config->setFillType(psd_fill_gradient);
        } else {
            config->setFillType(psd_fill_solid_color);
        }
    }

    template <class T>
    static void correctWithoutPattern(T *config) {
        if (config->gradient()) {
            config->setFillType(psd_fill_gradient);
        } else {
            config->setFillType(psd_fill_solid_color);
        }
    }
};

void KisAslLayerStyleSerializer::connectCatcherToStyle(KisPSDLayerStyle *style, const QString &prefix)
{
    CONN_TEXT_RADDR("/null/Nm  ", setName, style, KisPSDLayerStyle);
    CONN_TEXT_RADDR("/null/Idnt", setPsdUuid, style, KisPSDLayerStyle);

    CONN_BOOL("/masterFXSwitch", setEnabled, style, KisPSDLayerStyle, prefix);

    psd_layer_effects_drop_shadow *dropShadow = style->dropShadow();

    CONN_COMPOSITE_OP("/DrSh/Md  ", setBlendMode, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_COLOR("/DrSh/Clr ", setColor, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_UNITF("/DrSh/Opct", "#Prc", setOpacity, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_UNITF("/DrSh/lagl", "#Ang", setAngle, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_UNITF("/DrSh/Dstn", "#Pxl", setDistance, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_UNITF("/DrSh/Ckmt", "#Pxl", setSpread, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_UNITF("/DrSh/blur", "#Pxl", setSize, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_UNITF("/DrSh/Nose", "#Prc", setNoise, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_BOOL("/DrSh/enab", setEffectEnabled, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_BOOL("/DrSh/uglg", setUseGlobalLight, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_BOOL("/DrSh/AntA", setAntiAliased, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_BOOL("/DrSh/layerConceals", setKnocksOut, dropShadow, psd_layer_effects_drop_shadow, prefix);
    CONN_CURVE("/DrSh/TrnS", setContourLookupTable, dropShadow, psd_layer_effects_drop_shadow, prefix);

    psd_layer_effects_inner_shadow *innerShadow = style->innerShadow();

    CONN_COMPOSITE_OP("/IrSh/Md  ", setBlendMode, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_COLOR("/IrSh/Clr ", setColor, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_UNITF("/IrSh/Opct", "#Prc", setOpacity, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_UNITF("/IrSh/lagl", "#Ang", setAngle, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_UNITF("/IrSh/Dstn", "#Pxl", setDistance, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_UNITF("/IrSh/Ckmt", "#Pxl", setSpread, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_UNITF("/IrSh/blur", "#Pxl", setSize, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_UNITF("/IrSh/Nose", "#Prc", setNoise, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_BOOL("/IrSh/enab", setEffectEnabled, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_BOOL("/IrSh/uglg", setUseGlobalLight, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_BOOL("/IrSh/AntA", setAntiAliased, innerShadow, psd_layer_effects_inner_shadow, prefix);
    CONN_CURVE("/IrSh/TrnS", setContourLookupTable, innerShadow, psd_layer_effects_inner_shadow, prefix);

    psd_layer_effects_outer_glow *outerGlow = style->outerGlow();

    CONN_COMPOSITE_OP("/OrGl/Md  ", setBlendMode, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_COLOR("/OrGl/Clr ", setColor, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_UNITF("/OrGl/Opct", "#Prc", setOpacity, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_UNITF("/OrGl/Ckmt", "#Pxl", setSpread, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_UNITF("/OrGl/blur", "#Pxl", setSize, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_UNITF("/OrGl/Nose", "#Prc", setNoise, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_BOOL("/OrGl/enab", setEffectEnabled, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_BOOL("/OrGl/AntA", setAntiAliased, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_CURVE("/OrGl/TrnS", setContourLookupTable, outerGlow, psd_layer_effects_outer_glow, prefix);

    QMap<QString, psd_technique_type> fillTechniqueMap;
    fillTechniqueMap.insert("PrBL", psd_technique_precise);
    fillTechniqueMap.insert("SfBL", psd_technique_softer);
    CONN_ENUM("/OrGl/GlwT", "BETE", setTechnique, fillTechniqueMap, psd_technique_type, outerGlow, psd_layer_effects_outer_glow, prefix);

    CONN_GRADIENT("/OrGl/Grad", setGradient, outerGlow, psd_layer_effects_outer_glow, prefix);

    CONN_UNITF("/OrGl/Inpr", "#Prc", setRange, outerGlow, psd_layer_effects_outer_glow, prefix);
    CONN_UNITF("/OrGl/ShdN", "#Prc", setJitter, outerGlow, psd_layer_effects_outer_glow, prefix);


    psd_layer_effects_inner_glow *innerGlow = style->innerGlow();

    CONN_COMPOSITE_OP("/IrGl/Md  ", setBlendMode, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_COLOR("/IrGl/Clr ", setColor, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_UNITF("/IrGl/Opct", "#Prc", setOpacity, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_UNITF("/IrGl/Ckmt", "#Pxl", setSpread, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_UNITF("/IrGl/blur", "#Pxl", setSize, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_UNITF("/IrGl/Nose", "#Prc", setNoise, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_BOOL("/IrGl/enab", setEffectEnabled, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_BOOL("/IrGl/AntA", setAntiAliased, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_CURVE("/IrGl/TrnS", setContourLookupTable, innerGlow, psd_layer_effects_inner_glow, prefix);

    CONN_ENUM("/IrGl/GlwT", "BETE", setTechnique, fillTechniqueMap, psd_technique_type, innerGlow, psd_layer_effects_inner_glow, prefix);

    CONN_GRADIENT("/IrGl/Grad", setGradient, innerGlow, psd_layer_effects_inner_glow, prefix);

    CONN_UNITF("/IrGl/Inpr", "#Prc", setRange, innerGlow, psd_layer_effects_inner_glow, prefix);
    CONN_UNITF("/IrGl/ShdN", "#Prc", setJitter, innerGlow, psd_layer_effects_inner_glow, prefix);

    QMap<QString, psd_glow_source> glowSourceMap;
    glowSourceMap.insert("SrcC", psd_glow_center);
    glowSourceMap.insert("SrcE", psd_glow_edge);
    CONN_ENUM("/IrGl/glwS", "IGSr", setSource, glowSourceMap, psd_glow_source, innerGlow, psd_layer_effects_inner_glow, prefix);


    psd_layer_effects_satin *satin = style->satin();

    CONN_COMPOSITE_OP("/ChFX/Md  ", setBlendMode, satin, psd_layer_effects_satin, prefix);
    CONN_COLOR("/ChFX/Clr ", setColor, satin, psd_layer_effects_satin, prefix);

    CONN_UNITF("/ChFX/Opct", "#Prc", setOpacity, satin, psd_layer_effects_satin, prefix);
    CONN_UNITF("/ChFX/lagl", "#Ang", setAngle, satin, psd_layer_effects_satin, prefix);
    CONN_UNITF("/ChFX/Dstn", "#Pxl", setDistance, satin, psd_layer_effects_satin, prefix);
    CONN_UNITF("/ChFX/blur", "#Pxl", setSize, satin, psd_layer_effects_satin, prefix);

    CONN_BOOL("/ChFX/enab", setEffectEnabled, satin, psd_layer_effects_satin, prefix);
    CONN_BOOL("/ChFX/AntA", setAntiAliased, satin, psd_layer_effects_satin, prefix);
    CONN_BOOL("/ChFX/Invr", setInvert, satin, psd_layer_effects_satin, prefix);
    CONN_CURVE("/ChFX/MpgS", setContourLookupTable, satin, psd_layer_effects_satin, prefix);

    psd_layer_effects_color_overlay *colorOverlay = style->colorOverlay();

    CONN_COMPOSITE_OP("/SoFi/Md  ", setBlendMode, colorOverlay, psd_layer_effects_color_overlay, prefix);
    CONN_COLOR("/SoFi/Clr ", setColor, colorOverlay, psd_layer_effects_color_overlay, prefix);
    CONN_UNITF("/SoFi/Opct", "#Prc", setOpacity, colorOverlay, psd_layer_effects_color_overlay, prefix);
    CONN_BOOL("/SoFi/enab", setEffectEnabled, colorOverlay, psd_layer_effects_color_overlay, prefix);

    psd_layer_effects_gradient_overlay *gradientOverlay = style->gradientOverlay();

    CONN_COMPOSITE_OP("/GrFl/Md  ", setBlendMode, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_UNITF("/GrFl/Opct", "#Prc", setOpacity, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_UNITF("/GrFl/Scl ", "#Prc", setScale, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_UNITF("/GrFl/Angl", "#Ang", setAngle, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_BOOL("/GrFl/enab", setEffectEnabled, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    // CONN_BOOL("/GrFl/Dthr", setDitherNotImplemented, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_BOOL("/GrFl/Rvrs", setReverse, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_BOOL("/GrFl/Algn", setAlignWithLayer, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_POINT("/GrFl/Ofst", setGradientOffset, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);
    CONN_GRADIENT("/GrFl/Grad", setGradient, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);


    QMap<QString, psd_gradient_style> gradientStyleMap;
    gradientStyleMap.insert("Lnr ", psd_gradient_style_linear);
    gradientStyleMap.insert("Rdl ", psd_gradient_style_radial);
    gradientStyleMap.insert("Angl", psd_gradient_style_angle);
    gradientStyleMap.insert("Rflc", psd_gradient_style_reflected);
    gradientStyleMap.insert("Dmnd", psd_gradient_style_diamond);
    CONN_ENUM("/GrFl/Type", "GrdT", setStyle, gradientStyleMap, psd_gradient_style, gradientOverlay, psd_layer_effects_gradient_overlay, prefix);

    psd_layer_effects_pattern_overlay *patternOverlay = style->patternOverlay();

    CONN_BOOL("/patternFill/enab", setEffectEnabled, patternOverlay, psd_layer_effects_pattern_overlay, prefix);
    CONN_COMPOSITE_OP("/patternFill/Md  ", setBlendMode, patternOverlay, psd_layer_effects_pattern_overlay, prefix);
    CONN_UNITF("/patternFill/Opct", "#Prc", setOpacity, patternOverlay, psd_layer_effects_pattern_overlay, prefix);
    CONN_PATTERN("/patternFill/Ptrn", setPattern, patternOverlay, psd_layer_effects_pattern_overlay, prefix);
    CONN_UNITF("/patternFill/Scl ", "#Prc", setScale, patternOverlay, psd_layer_effects_pattern_overlay, prefix);
    CONN_BOOL("/patternFill/Algn", setAlignWithLayer, patternOverlay, psd_layer_effects_pattern_overlay, prefix);
    CONN_POINT("/patternFill/phase", setPatternPhase, patternOverlay, psd_layer_effects_pattern_overlay, prefix);

    psd_layer_effects_stroke *stroke = style->stroke();

    CONN_COMPOSITE_OP("/FrFX/Md  ", setBlendMode, stroke, psd_layer_effects_stroke, prefix);
    CONN_BOOL("/FrFX/enab", setEffectEnabled, stroke, psd_layer_effects_stroke, prefix);
    CONN_UNITF("/FrFX/Opct", "#Prc", setOpacity, stroke, psd_layer_effects_stroke, prefix);
    CONN_UNITF("/FrFX/Sz  ", "#Pxl", setSize, stroke, psd_layer_effects_stroke, prefix);

    QMap<QString, psd_stroke_position> strokeStyleMap;
    strokeStyleMap.insert("OutF", psd_stroke_outside);
    strokeStyleMap.insert("InsF", psd_stroke_inside);
    strokeStyleMap.insert("CtrF", psd_stroke_center);
    CONN_ENUM("/FrFX/Styl", "FStl", setPosition, strokeStyleMap, psd_stroke_position, stroke, psd_layer_effects_stroke, prefix);

    QMap<QString, psd_fill_type> strokeFillType;
    strokeFillType.insert("SClr", psd_fill_solid_color);
    strokeFillType.insert("GrFl", psd_fill_gradient);
    strokeFillType.insert("Ptrn", psd_fill_pattern);
    CONN_ENUM("/FrFX/PntT", "FrFl", setFillType, strokeFillType, psd_fill_type, stroke, psd_layer_effects_stroke, prefix);

    // Color type
    CONN_COLOR("/FrFX/Clr ", setColor, stroke, psd_layer_effects_stroke, prefix);

    // Gradient Type
    CONN_GRADIENT("/FrFX/Grad", setGradient, stroke, psd_layer_effects_stroke, prefix);
    CONN_UNITF("/FrFX/Angl", "#Ang", setAngle, stroke, psd_layer_effects_stroke, prefix);
    CONN_UNITF("/FrFX/Scl ", "#Prc", setScale, stroke, psd_layer_effects_stroke, prefix);
    CONN_ENUM("/FrFX/Type", "GrdT", setStyle, gradientStyleMap, psd_gradient_style, stroke, psd_layer_effects_stroke, prefix);
    CONN_BOOL("/FrFX/Rvrs", setReverse, stroke, psd_layer_effects_stroke, prefix);
    CONN_BOOL("/FrFX/Algn", setAlignWithLayer, stroke, psd_layer_effects_stroke, prefix);
    CONN_POINT("/FrFX/Ofst", setGradientOffset, stroke, psd_layer_effects_stroke, prefix);
    // CONN_BOOL("/FrFX/Dthr", setDitherNotImplemented, stroke, psd_layer_effects_stroke, prefix);

    // Pattern type

    CONN_PATTERN("/FrFX/Ptrn", setPattern, stroke, psd_layer_effects_stroke, prefix);
    CONN_BOOL("/FrFX/Lnkd", setAlignWithLayer, stroke, psd_layer_effects_stroke, prefix); // yes, we share the params...
    CONN_POINT("/FrFX/phase", setPatternPhase, stroke, psd_layer_effects_stroke, prefix);


    psd_layer_effects_bevel_emboss *bevelAndEmboss = style->bevelAndEmboss();

    CONN_BOOL("/ebbl/enab", setEffectEnabled, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_COMPOSITE_OP("/ebbl/hglM", setHighlightBlendMode, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_COLOR("/ebbl/hglC", setHighlightColor, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_UNITF("/ebbl/hglO", "#Prc", setHighlightOpacity, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_COMPOSITE_OP("/ebbl/sdwM", setShadowBlendMode, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_COLOR("/ebbl/sdwC", setShadowColor, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_UNITF("/ebbl/sdwO", "#Prc", setShadowOpacity, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    QMap<QString, psd_technique_type> bevelTechniqueMap;
    bevelTechniqueMap.insert("PrBL", psd_technique_precise);
    bevelTechniqueMap.insert("SfBL", psd_technique_softer);
    bevelTechniqueMap.insert("Slmt", psd_technique_slope_limit);
    CONN_ENUM("/ebbl/bvlT", "bvlT", setTechnique, bevelTechniqueMap, psd_technique_type, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    QMap<QString, psd_bevel_style> bevelStyleMap;
    bevelStyleMap.insert("OtrB", psd_bevel_outer_bevel);
    bevelStyleMap.insert("InrB", psd_bevel_inner_bevel);
    bevelStyleMap.insert("Embs", psd_bevel_emboss);
    bevelStyleMap.insert("PlEb", psd_bevel_pillow_emboss);
    bevelStyleMap.insert("strokeEmboss", psd_bevel_stroke_emboss);
    CONN_ENUM("/ebbl/bvlS", "BESl", setStyle, bevelStyleMap, psd_bevel_style, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_BOOL("/ebbl/uglg", setUseGlobalLight, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_UNITF("/ebbl/lagl", "#Ang", setAngle, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_UNITF("/ebbl/Lald", "#Ang", setAltitude, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_UNITF("/ebbl/srgR", "#Prc", setDepth, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_UNITF("/ebbl/blur", "#Pxl", setSize, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);


    QMap<QString, psd_direction> bevelDirectionMap;
    bevelDirectionMap.insert("In  ", psd_direction_up);
    bevelDirectionMap.insert("Out ", psd_direction_down);
    CONN_ENUM("/ebbl/bvlD", "BESs", setDirection, bevelDirectionMap, psd_direction, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_CURVE("/ebbl/TrnS", setContourLookupTable, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_BOOL("/ebbl/antialiasGloss", setGlossAntiAliased, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    CONN_UNITF("/ebbl/Sftn", "#Pxl", setSoften, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    // Use shape mode

    CONN_BOOL("/ebbl/useShape", setContourEnabled, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_CURVE("/ebbl/MpgS", setGlossContourLookupTable, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_BOOL("/ebbl/AntA", setAntiAliased, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_UNITF("/ebbl/Inpr", "#Prc", setContourRange, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);

    // Use texture mode

    CONN_BOOL("/ebbl/useTexture", setTextureEnabled, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_BOOL("/ebbl/InvT", setTextureInvert, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_BOOL("/ebbl/Algn", setTextureAlignWithLayer, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_UNITF("/ebbl/Scl ", "#Prc", setTextureScale, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_UNITF("/ebbl/textureDepth", "#Prc", setTextureDepth, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_PATTERN("/ebbl/Ptrn", setTexturePattern, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
    CONN_POINT("/ebbl/phase", setTexturePhase, bevelAndEmboss, psd_layer_effects_bevel_emboss, prefix);
}

void KisAslLayerStyleSerializer::newStyleStarted(bool isPsdStructure)
{
    m_stylesVector.append(toQShared(new KisPSDLayerStyle()));
    KisPSDLayerStyle *currentStyle = m_stylesVector.last().data();

    psd_layer_effects_context *context = currentStyle->context();
    context->keep_original = 0;

    QString prefix = isPsdStructure ? "/null" : "/Styl/Lefx";
    connectCatcherToStyle(currentStyle, prefix);
}

void KisAslLayerStyleSerializer::readFromDevice(QIODevice *device)
{
    m_stylesVector.clear();

    m_catcher.subscribePattern("/Patterns/KisPattern", std::bind(&KisAslLayerStyleSerializer::registerPatternObject, this, _1));
    m_catcher.subscribeNewStyleStarted(std::bind(&KisAslLayerStyleSerializer::newStyleStarted, this, false));

    KisAslReader reader;
    QDomDocument doc = reader.readFile(device);

    //dbgKrita << ppVar(doc.toString());

    //KisAslObjectCatcher c2;
    KisAslXmlParser parser;
    parser.parseXML(doc, m_catcher);

    // correct all the layer styles
    Q_FOREACH (KisPSDLayerStyleSP style, m_stylesVector) {
        FillStylesCorrector::correct(style.data());
    }
}

void KisAslLayerStyleSerializer::registerPSDPattern(const QDomDocument &doc)
{
    KisAslCallbackObjectCatcher catcher;
    catcher.subscribePattern("/Patterns/KisPattern", std::bind(&KisAslLayerStyleSerializer::registerPatternObject, this, _1));

    //KisAslObjectCatcher c2;
    KisAslXmlParser parser;
    parser.parseXML(doc, catcher);
}

void KisAslLayerStyleSerializer::readFromPSDXML(const QDomDocument &doc)
{
    // The caller prepares the document using th efollowing code
    //
    // KisAslReader reader;
    // QDomDocument doc = reader.readLfx2PsdSection(device);

    m_stylesVector.clear();

    //m_catcher.subscribePattern("/Patterns/KisPattern", std::bind(&KisAslLayerStyleSerializer::registerPatternObject, this, _1));
    m_catcher.subscribeNewStyleStarted(std::bind(&KisAslLayerStyleSerializer::newStyleStarted, this, true));

    //KisAslObjectCatcher c2;
    KisAslXmlParser parser;
    parser.parseXML(doc, m_catcher);

    // correct all the layer styles
    Q_FOREACH (KisPSDLayerStyleSP style, m_stylesVector) {
        FillStylesCorrector::correct(style.data());
    }
}
