/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_KRA_SAVEXML_VISITOR_H_
#define KIS_KRA_SAVEXML_VISITOR_H_

#include <QDomDocument>
#include <QDomElement>
#include <QStringList>

#include "kis_node_visitor.h"
#include "kis_types.h"
#include "kritalibkra_export.h"

class KRITALIBKRA_EXPORT KisSaveXmlVisitor : public KisNodeVisitor
{
public:
    KisSaveXmlVisitor(QDomDocument doc, const QDomElement & element, quint32 &count, const QString &url, bool root);

    void setSelectedNodes(vKisNodeSP selectedNodes);

    using KisNodeVisitor::visit;

    QStringList errorMessages() const;

public:

    bool visit(KisNode*) override {
        return true;
    }
    bool visit(KisExternalLayer *) override;
    bool visit(KisPaintLayer *layer) override;
    bool visit(KisGroupLayer *layer) override;
    bool visit(KisAdjustmentLayer* layer) override;
    bool visit(KisGeneratorLayer *layer) override;
    bool visit(KisCloneLayer *layer) override;
    bool visit(KisFilterMask *mask) override;
    bool visit(KisTransformMask *mask) override;
    bool visit(KisTransparencyMask *mask) override;
    bool visit(KisSelectionMask *mask) override;
    bool visit(KisColorizeMask *mask) override;

    QMap<const KisNode*, QString> nodeFileNames() {
        return m_nodeFileNames;
    }

    QMap<const KisNode*, QString> keyframeFileNames() {
        return m_keyframeFileNames;
    }

public:
    QDomElement savePaintLayerAttributes(KisPaintLayer *layer, QDomDocument &doc);

    // used by EXR to save properties of Krita layers inside .exr
    static void loadPaintLayerAttributes(const QDomElement &el, KisPaintLayer *layer);

private:
    static void loadLayerAttributes(const QDomElement &el, KisLayer *layer);

private:

    void saveLayer(QDomElement & el, const QString & layerType, const KisLayer * layer);
    void saveMask(QDomElement & el, const QString & maskType, const KisMaskSP mask);
    bool saveMasks(KisNode * node, QDomElement & layerElement);
    void saveNodeKeyframes(const KisNode *node, QString filename, QDomElement& el);

    friend class KisKraSaveXmlVisitorTest;

    vKisNodeSP m_selectedNodes;
    QMap<const KisNode*,  QString> m_nodeFileNames;
    QMap<const KisNode*,  QString> m_keyframeFileNames;
    QDomDocument m_doc;
    QDomElement m_elem;
    quint32 &m_count;
    QString m_url;
    bool m_root;
    QStringList m_errorMessages;

    bool saveReferenceImagesLayer(KisExternalLayer *layer);
};

#endif

