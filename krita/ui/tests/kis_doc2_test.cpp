/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_doc2_test.h"

#include <kundo2stack.h>
#include <qtest_kde.h>
#include <kstandarddirs.h>
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_undo_store.h"
#include "kis_factory2.h"
#include <KoDocumentEntry.h>
#include "kis_part2.h"
#include <KoMainWindow.h>
#include <kis_view2.h>
#include "util.h"


void KisDoc2Test::testOpenImageTwiceInSameDoc()
{
    QString fname2 = QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra";
    QString fname = QString(FILES_DATA_DIR) + QDir::separator() + "load_test2.kra";


    Q_ASSERT(!fname.isEmpty());
    Q_ASSERT(!fname2.isEmpty());

    KisPart2 part;
    KisDoc2 doc(&part);
    part.setDocument(&doc);

    doc.loadNativeFormat(fname);
    doc.loadNativeFormat(fname2);
}

void KisDoc2Test::testActiveNodes()
{
    KisDoc2* doc = createEmptyDocument();
    KoMainWindow* shell = new KoMainWindow(doc->documentPart()->componentData());
    KisView2* view = new KisView2(static_cast<KisPart2*>(doc->documentPart()), static_cast<KisDoc2*>(doc), shell);
    doc->documentPart()->addView(view);
    vKisNodeSP nodes = doc->activeNodes();
    QVERIFY(nodes.isEmpty());

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(doc->image(), "paintlayer1", OPACITY_OPAQUE_U8);
    doc->image()->addNode(paintLayer1);
    nodes = doc->activeNodes();
    QCOMPARE(nodes.count(), 1);
}


QTEST_KDEMAIN(KisDoc2Test, GUI)
#include "kis_doc2_test.moc"

