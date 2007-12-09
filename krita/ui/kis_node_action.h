/*
 * Copyright (C) (C) 2007, Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_NODE_ACTION
#define KIS_NODE_ACTION

#include "KoAction.h"
#include "kis_types.h"
#include "krita_export.h"

class KoProgressProxy;

/**
 * KisnodeAction is a wrapper around KoAction that locks a certain node,
 * executes the specified method on it in a thread (possibly chunked
 * using KisThreadedApplicator) and unlocks the node when the action
 * is done.
 */
class KRITAUI_EXPORT KisNodeAction : public KoAction
{

Q_OBJECT

public:

    KisNodeAction( QObject * parent, KisNodeSP node, KoProgressProxy * progressProxy );
    virtual ~KisNodeAction();
    
protected slots:

    virtual void slotTriggered() {}

private slots:
    
    void slotUpdateGUI();

private:
    
    struct Private;
    Private * const m_d;

};
#endif
