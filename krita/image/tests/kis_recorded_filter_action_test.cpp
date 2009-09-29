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

#include "kis_recorded_filter_action_test.h"

#include <qtest_kde.h>
#include "recorder/kis_recorded_filter_action.h"
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_paint_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include <recorder/kis_node_query_path.h>

void KisRecordedFilterActionTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    KisImageWSP image = new KisImage(0, 10, 10, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE);

    KisRecordedFilterAction test("invert", KisNodeQueryPath::absolutePath(layer), f, kfc);
}


QTEST_KDEMAIN(KisRecordedFilterActionTest, GUI)
#include "kis_recorded_filter_action_test.moc"
