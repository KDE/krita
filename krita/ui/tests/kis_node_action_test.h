/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#ifndef KIS_NODE_ACTION_TEST_H
#define KIS_NODE_ACTION_TEST_H

#include <QtTest/QtTest>

#include "kis_types.h"
#include "kis_node_action.h"

class KoProgressProxy;

class SimpleNodeAction : public KisNodeAction
{
    Q_OBJECT
    
public:

    SimpleNodeAction( QObject * parent, KisNodeSP node, KoProgressProxy * progressBar);
public slots:
    void slotTriggered();
};

class KisNodeActionTest : public QObject
{
    Q_OBJECT

public slots:

    void updateGUI();
    
private slots:

    void testExecution();

private:

    bool jobFinished;

};

#endif
