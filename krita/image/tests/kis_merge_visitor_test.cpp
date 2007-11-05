/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_merge_visitor_test.h"
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_types.h"
#include "kis_image.h"
#include "kis_paint_device.h"

#include "kis_effect_mask.h"
#include "kis_transparency_mask.h"
#include "kis_transformation_mask.h"

#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"


void KisMergeVisitorTest::testMerge()
{
    QFAIL( "Implement!" );
}


QTEST_KDEMAIN(KisMergeVisitorTest, NoGUI)
#include "kis_merge_visitor_test.moc"

