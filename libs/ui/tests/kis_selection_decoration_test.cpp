/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_decoration_test.h"

#include <simpletest.h>
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
                                           KisImageSignalVector(),
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

SIMPLE_TEST_MAIN(KisSelectionDecorationTest)
