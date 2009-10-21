/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_KRA_SAVEXML_VISITOR_H_
#define KIS_KRA_SAVEXML_VISITOR_H_

#include <QDomDocument>
#include <QDomElement>

#include "kis_node_visitor.h"
#include "kis_types.h"
#include "krita_export.h"

class KRITAUI_EXPORT KisSaveXmlVisitor : public KisNodeVisitor
{
public:
    KisSaveXmlVisitor(QDomDocument doc, const QDomElement & element, quint32 &count, bool root = false);

    using KisNodeVisitor::visit;

public:

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisExternalLayer *);
    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer* layer);
    bool visit(KisGeneratorLayer *layer);
    bool visit(KisCloneLayer *layer);
    bool visit(KisFilterMask *mask);
    bool visit(KisTransparencyMask *mask);
    bool visit(KisTransformationMask *mask);
    bool visit(KisSelectionMask *mask);

    QMap<const KisNode*, QString> nodeFileNames() {
        return m_nodeFileNames;
    }

private:

    void saveLayer(QDomElement & el, const QString & layerType, const KisLayer * layer);
    void saveMask(QDomElement & el, const QString & maskType, const KisMask * mask);
    bool saveMasks(KisNode * node, QDomElement & layerElement);

    friend class KisKraSaveXmlVisitorTest;

    QMap<const KisNode*,  QString> m_nodeFileNames;
    QDomDocument m_doc;
    QDomElement m_elem;
    quint32 &m_count;
    bool m_root;
};

#endif

