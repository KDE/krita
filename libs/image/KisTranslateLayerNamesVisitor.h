/*
 *  SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTRANSLATELAYERNAMESVISITOR_H
#define KISTRANSLATELAYERNAMESVISITOR_H

#include <QMap>
#include "kis_node_visitor.h"

#include <kritaimage_export.h>


/**
 * @brief KisTranslateLayerNamesVisitor::KisTranslateLayerNamesVisitor translates layer names
 * from templates.
 */
class KRITAIMAGE_EXPORT KisTranslateLayerNamesVisitor : public KisNodeVisitor
{
public:
    KisTranslateLayerNamesVisitor(QMap<QString, QString> dictionary);

    using KisNodeVisitor::visit;

    bool visit(KisNode* node) override;

    bool visit(KisPaintLayer *layer) override;

    bool visit(KisGroupLayer *layer) override;

    bool visit(KisAdjustmentLayer *layer) override;

    bool visit(KisExternalLayer *layer) override;

    bool visit(KisCloneLayer *layer) override;

    bool visit(KisFilterMask *mask) override;

    bool visit(KisTransformMask *mask) override;

    bool visit(KisTransparencyMask *mask) override;

    bool visit(KisGeneratorLayer * layer) override;

    bool visit(KisSelectionMask* mask) override;

    bool visit(KisColorizeMask* mask) override;

    QMap<QString, QString> defaultDictionary();

private:

    bool translate(KisNode *node);

    QMap<QString, QString> m_dictionary;
};

#endif // KISTRANSLATELAYERNAMESVISITOR_H
