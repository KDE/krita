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
#ifndef KIS_SAVEXML_VISITOR_H_
#define KIS_SAVEXML_VISITOR_H_

#include <QDomDocument>
#include <QDomElement>

#include "kis_node_visitor.h"
#include "kis_types.h"


class KisSaveXmlVisitor : public KisNodeVisitor {
public:
    KisSaveXmlVisitor(QDomDocument doc, QDomElement element, quint32 &count, bool root=false);

public:
    bool visit( KisExternalLayer * );

    bool visit(KisPaintLayer *layer);

    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer* layer);

private:
    QDomDocument m_doc;
    QDomElement m_elem;
    quint32 &m_count;
    bool m_root;
};

#endif // KIS_SAVEXML_VISITOR_H_

