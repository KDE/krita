/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_selection_decoration_test.h"

#include <QTest>
#include <stroke_testing_utils.h>
#include "kis_processing_applicator.h"
#include "commands/kis_selection_commands.h"
#include "kis_selection.h"


void KisSelectionDecorationTest::testConcurrentSelectionFetches()
{
    KisImageSP image = utils::createImage(0, QSize(3000, 3000));

    for (int i = 0; i < 10000; i++) {
        KisProcessingApplicator applicator(image,
                                           0 /* we need no automatic updates */,
                                           KisProcessingApplicator::SUPPORTS_WRAPAROUND_MODE,
                                           KisImageSignalVector() << ModifiedSignal,
                                           kundo2_noi18n("test stroke"));


        applicator.applyCommand(new KisSetEmptyGlobalSelectionCommand(image), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
        applicator.applyCommand(new KisDeselectGlobalSelectionCommand(image), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

        applicator.end();

        for (int j = 0; j < 100; j++) {
            KisSelectionSP selection = image->globalSelection();
        }
    }

    image->waitForDone();
}

QTEST_MAIN(KisSelectionDecorationTest)
