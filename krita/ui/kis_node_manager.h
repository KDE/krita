/*
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_NODE_MANAGER
#define KIS_NODE_MANAGER

#include <QObject>
#include <QList>

#include "kis_types.h"
#include <krita_export.h>

class KAction;
class QAction;
class KToggleAction;
class KActionCollection;

class KoCompositeOp;
class KoColorSpace;

class KisDoc2;
class KisFilterStrategy;
class KisView2;
class KisFilterConfiguration;


/**
 * XXX
 */
class KRITAUI_EXPORT KisNodeManager : public QObject {

    Q_OBJECT

public:

    KisNodeManager(KisView2 * view,  KisDoc2 * doc);
    ~KisNodeManager();

    void setup(KActionCollection * collection);
    void updateGUI();

    /// Convenience function to get the active layer
    KisNodeSP activeNode();

signals:

    void sigNodeActivated( KisNodeSP layer );

public slots:

    void activateNode( KisNodeSP layer );
    void nodesUpdated();

private:

    struct Private;
    Private * const m_d;

};

#endif
