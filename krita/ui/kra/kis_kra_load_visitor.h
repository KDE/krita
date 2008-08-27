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
#ifndef KIS_KRA_LOAD_VISITOR_H_
#define KIS_KRA_LOAD_VISITOR_H_

#include <QRect>

// kritaimage
#include "kis_types.h"
#include "kis_node_visitor.h"

class KoStore;

class KisKraLoadVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisKraLoadVisitor(KisImageSP img, KoStore *store, QMap<KisLayer *, QString> &layerFilenames, const QString & name);

public:
    void setExternalUri(const QString &uri);

    bool visit(KisExternalLayer *);
    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer* layer);
    bool visit(KisGeneratorLayer* layer);
private:
    KisImageSP m_img;
    KoStore *m_store;
    bool m_external;
    QString m_uri;
    QMap<KisLayer *, QString> m_layerFilenames;
    QString m_name;
};

#endif // KIS_KRA_LOAD_VISITOR_H_

