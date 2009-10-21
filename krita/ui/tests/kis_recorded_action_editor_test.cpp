/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_recorded_action_editor_test.h"

#include <qtest_kde.h>

#include <QTest>
#include <QCoreApplication>
#include <recorder/kis_recorded_action_editor_factory_registry.h>
#include <recorder/kis_recorded_filter_action.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter.h>
#include <kis_paint_device.h>
#include <recorder/kis_node_query_path.h>

void KisRecordedActionEditorTest::testFilterEditorCreation()
{
    KisRecordedActionEditorFactoryRegistry* reg = KisRecordedActionEditorFactoryRegistry::instance();
    const KisFilter* blurFilter = KisFilterRegistry::instance()->get("blur");
    KisRecordedFilterAction* blurFilterAction = new KisRecordedFilterAction(
        "hello", KisNodeQueryPath::fromString(""), blurFilter, blurFilter->defaultConfiguration(0));
    QVERIFY(reg->hasEditor(blurFilterAction));
    QVERIFY(reg->createEditor(0, blurFilterAction));
    const KisFilter* invertFilter = KisFilterRegistry::instance()->get("invert");
    KisRecordedFilterAction* invertFilterAction = new KisRecordedFilterAction(
        "hello", KisNodeQueryPath::fromString(""), invertFilter, invertFilter->defaultConfiguration(0));
    QVERIFY(reg->hasEditor(invertFilterAction));
    QVERIFY(reg->createEditor(0, invertFilterAction));
}


QTEST_KDEMAIN(KisRecordedActionEditorTest, GUI)

#include "kis_recorded_action_editor_test.moc"
