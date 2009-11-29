/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_KRA_SAVE_VISITOR_H_
#define KIS_KRA_SAVE_VISITOR_H_

#include <QRect>
#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_image.h"

class KoStore;

class KisKraSaveVisitor : public KisNodeVisitor
{
public:
    KisKraSaveVisitor(KisImageWSP image, KoStore *store, quint32 &count, const QString & name, QMap<const KisNode*, QString> nodeFileNames);

    using KisNodeVisitor::visit;

public:
    void setExternalUri(const QString &uri);

    bool visit(KisNode*) {
        return true;
    }

    bool visit(KisExternalLayer *);

    bool visit(KisPaintLayer *layer);

    bool visit(KisGroupLayer *layer);

    bool visit(KisAdjustmentLayer* layer);

    bool visit(KisGeneratorLayer * layer);

    bool visit(KisCloneLayer *layer);

    bool visit(KisFilterMask *mask);

    bool visit(KisTransparencyMask *mask);

    bool visit(KisTransformationMask *mask);

    bool visit(KisSelectionMask *mask);

private:

    bool savePaintDevice(KisNode * node);
    bool saveAnnotations(KisLayer* layer);
    bool saveSelection(KisNode* node);
    bool saveFilterConfiguration(KisNode* node);
    bool saveMetaData(KisNode* node);
    QString getLocation(KisNode* node, const QString& suffix = "");

private:
    KisImageWSP m_image;
    KoStore *m_store;
    bool m_external;
    QString m_uri;
    quint32 &m_count;
    QString m_name;
    QMap<const KisNode*, QString> m_nodeFileNames;
};

#endif // KIS_KRA_SAVE_VISITOR_H_

