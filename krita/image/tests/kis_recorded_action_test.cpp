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

#include "kis_recorded_action_test.h"

#include <qtest_kde.h>
#include <kis_node.h>
#include "recorder/kis_recorded_action.h"
#include <recorder/kis_node_query_path.h>

class TestAction : public KisRecordedAction
{
public:

    TestAction(const QString & id, const QString & name, const KisNodeQueryPath& path)
            : KisRecordedAction(id, name, path) {
    }

    void play(KisNodeSP node, const KisPlayInfo&) const {
    }

    KisRecordedAction* clone() const {
        return new TestAction(id(), name(), nodeQueryPath());
    }

};


void KisRecordedActionTest::testCreation()
{
    TestAction tc("bla", "bla", KisNodeQueryPath::fromString("/"));
}


QTEST_KDEMAIN(KisRecordedActionTest, GUI)
#include "kis_recorded_action_test.moc"
