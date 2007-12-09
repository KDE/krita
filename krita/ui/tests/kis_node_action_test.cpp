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
#include <qtest_kde.h>

#include "kis_node_action_test.h"

#include <KoProgressUpdater.h>
#include <kis_paint_layer.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_node_action.h>
#include <QCoreApplication>
//#include "kis_simple_node_action.h"

struct TestProgressBar : public KoProgressProxy
{
    int maximum() const { return 0; }
    void setValue( int value )
        {
            qDebug() << "Progress (" << this << "): " << value ;
        }
    void setRange( int, int) {}
    void setFormat( const QString & ) {}
};


SimpleNodeAction::SimpleNodeAction( QObject * parent, KisNodeSP node, KoProgressProxy * progressBar)
        : KisNodeAction( parent, node, progressBar )
{
}

void SimpleNodeAction::slotTriggered()
{
     // Do stuff on the layer
}


void KisNodeActionTest::updateGUI()
{
    jobFinished = true;
}

void KisNodeActionTest::testExecution()
{
    jobFinished = false;
    TestProgressBar bar;

    KisImageSP img = new KisImage(0, 1000, 1000, 0, "test");
    KisPaintLayerSP layer = new KisPaintLayer( img, "test", OPACITY_OPAQUE, img->colorSpace() );
    
    SimpleNodeAction action( this, layer.data(), &bar );
    QCOMPARE(layer->locked(), true);
    // Only for test purposes
    connect(&action, SIGNAL(updateUi(const QVariant&)), this, SLOT(updateGUI()), Qt::DirectConnection);
    action.execute();
    while (!jobFinished) {
        QTest::qSleep(250); // allow the action to do its job.
        QCoreApplication::processEvents(); // allow the actions 'gui' stuff to run.
    }
    QCOMPARE(layer->locked(), false);
}


QTEST_KDEMAIN(KisNodeActionTest, GUI);
#include "kis_node_action_test.moc"
