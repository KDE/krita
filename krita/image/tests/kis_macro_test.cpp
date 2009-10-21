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

#include "kis_macro_test.h"

#include <qtest_kde.h>
#include "recorder/kis_macro.h"
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_image.h"

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


void KisMacroTest::testCreation()
{
    QList<KisRecordedAction*> actions;
    TestAction tc("bla", "bla", KisNodeQueryPath::fromString("/"));
    actions << &tc;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, cs, "test");

    KisMacro a();
    KisMacro b(actions);
}


QTEST_KDEMAIN(KisMacroTest, GUI)
#include "kis_macro_test.moc"
