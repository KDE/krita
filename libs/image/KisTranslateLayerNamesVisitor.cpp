/*
 *  SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisTranslateLayerNamesVisitor.h"

#include "kis_node.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_clone_layer.h"
#include "kis_filter_mask.h"
#include "kis_transform_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "lazybrush/kis_colorize_mask.h"
#include "generator/kis_generator_layer.h"

#include <klocalizedstring.h>

KisTranslateLayerNamesVisitor::KisTranslateLayerNamesVisitor(QMap<QString, QString> dictionary)
    : m_dictionary(dictionary)
{
    QMap<QString, QString> d = defaultDictionary();
    QMap<QString, QString>::const_iterator i = d.constBegin();
    while (i != d.constEnd()) {
        if (!dictionary.contains(i.key())) {
            dictionary[i.key()] = i.value();
        }
        ++i;
    }
    m_dictionary = dictionary;
}

bool KisTranslateLayerNamesVisitor::translate(KisNode *node)
{
    if (m_dictionary.contains(node->name())) {
        node->setName(m_dictionary[node->name()]);
    }
    node->setName(node->name().replace("Layer", i18n("Layer")));
    node->setName(node->name().replace("layer", i18n("layer")));
    return true;
}

bool KisTranslateLayerNamesVisitor::visit(KisNode* node) {
    return translate(node);
}

QMap<QString, QString> KisTranslateLayerNamesVisitor::defaultDictionary()
{
    QMap<QString, QString> dictionary;

    dictionary["Background"] = i18nc("Layer name for translation of template", "Background");
    dictionary["Group"] = i18nc("Layer name for translation of template", "Group");
    dictionary["Margins"] = i18nc("Layer name for translation of template", "Margins");
    dictionary["Bleed"] = i18nc("Layer name for translation of template", "Bleed");
    dictionary["Lines"] = i18nc("Layer name for translation of template", "Lines");
    dictionary["Colors"] = i18nc("Layer name for translation of template", "Colors");
    dictionary["Sketch"] = i18nc("Layer name for translation of template", "Sketch");
    dictionary["Shade"] = i18nc("Layer name for translation of template", "Shade");
    dictionary["Filter"] = i18nc("Layer name for translation of template", "Filter");
    dictionary["Mask"] = i18nc("Layer name for translation of template", "Mask");
    dictionary["Layer"] = i18nc("Layer name for translation of template", "Layer");
    dictionary["Indirect light"] = i18nc("Layer name for translation of template", "Indirect light");
    dictionary["Highlight"] = i18nc("Layer name for translation of template", "Highlight");
    dictionary["Flat"] = i18nc("Layer name for translation of template", "Flat");
    dictionary["Panel"] = i18nc("Layer name for translation of template", "Panel");
    dictionary["Text"] = i18nc("Layer name for translation of template", "Text");
    dictionary["Effect"] = i18nc("Layer name for translation of template", "Effect");
    dictionary["Tones"] = i18nc("Layer name for translation of template", "Tones");
    dictionary["Textures"] = i18nc("Layer name for translation of template", "Textures");
    dictionary["Guides"] = i18nc("Layer name for translation of template", "Guides");
    dictionary["Balloons"] = i18nc("Layer name for translation of template", "Balloons");
    dictionary["Clone"] = i18nc("Layer name for translation of template", "Clone");
    dictionary["In Betweening"] = i18nc("Layer name for translation of template", "In Betweening");
    dictionary["Layout"] = i18nc("Layer name for translation of template", "Layout");

    return dictionary;
}

bool KisTranslateLayerNamesVisitor::visit(KisPaintLayer *layer) {
    return translate(layer);
}

bool KisTranslateLayerNamesVisitor::visit(KisGroupLayer *layer) {
    return translate(layer);
}


bool KisTranslateLayerNamesVisitor::visit(KisAdjustmentLayer *layer) {
    return translate(layer);
}


bool KisTranslateLayerNamesVisitor::visit(KisExternalLayer *layer) {
    return translate(layer);
}


bool KisTranslateLayerNamesVisitor::visit(KisCloneLayer *layer) {
    return translate(layer);
}


bool KisTranslateLayerNamesVisitor::visit(KisFilterMask *mask) {
    return translate(mask);
}

bool KisTranslateLayerNamesVisitor::visit(KisTransformMask *mask) {
    return translate(mask);
}

bool KisTranslateLayerNamesVisitor::visit(KisTransparencyMask *mask) {
    return translate(mask);
}


bool KisTranslateLayerNamesVisitor::visit(KisGeneratorLayer * layer) {
    return translate(layer);
}

bool KisTranslateLayerNamesVisitor::visit(KisSelectionMask* mask) {
    return translate(mask);
}

bool KisTranslateLayerNamesVisitor::visit(KisColorizeMask* mask) {
    return translate(mask);
}

