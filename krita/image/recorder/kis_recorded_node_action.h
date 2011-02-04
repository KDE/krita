/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_RECORDED_NODE_ACTION_H_
#define _KIS_RECORDED_NODE_ACTION_H_

#include "kis_recorded_action.h"

/**
 * Used for action that applys on nodes that are 
 */
class KRITAIMAGE_EXPORT KisRecordedNodeAction : public KisRecordedAction
{
public:
    KisRecordedNodeAction(const QString& id, const QString& name, const KisNodeQueryPath& path);
    KisRecordedNodeAction(const KisRecordedNodeAction& _rhs);
    virtual ~KisRecordedNodeAction();
    /**
     * Play the action on one node
     */
    virtual void play(KisNodeSP node, const KisPlayInfo&, KoUpdater* _updater) const = 0;
    /**
     * Play the action on all the nodes returned by the nodeQueryPath
     */
    virtual void play(const KisPlayInfo& _info, KoUpdater* _updater) const;
    virtual void toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* ) const;
public:
    const KisNodeQueryPath& nodeQueryPath() const;
    void setNodeQueryPath(const KisNodeQueryPath&);
private:
    struct Private;
    Private* const d;
};

#endif
