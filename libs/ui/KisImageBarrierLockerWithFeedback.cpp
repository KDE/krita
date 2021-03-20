/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisImageBarrierLockerWithFeedback.h"

#include <KisPart.h>
#include <KisViewManager.h>
#include <KisMainWindow.h>

#include "kis_image.h"

namespace KisImageBarrierLockerWithFeedbackImplPrivate {
void blockWithFeedback(KisImageSP image)
{
    if (!image) return;

    // TODO1: find the window corresponding to the awaited image!
    // TODO2: move blocking code from KisViewManager here
    KisMainWindow *window = KisPart::instance()->currentMainwindow();
    if (!window) return;

    KisViewManager *viewManager = window->viewManager();
    viewManager->blockUntilOperationsFinishedForced(image);
}
}
