/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisNotifySelectionChangedCommand.h"

#include "kis_image.h"

KisNotifySelectionChangedCommand::KisNotifySelectionChangedCommand(KisImageWSP image, State state)
    : KisCommandUtils::FlipFlopCommand(state)
    , m_image(image)
{
}
void KisNotifySelectionChangedCommand::partB()
{
    KisImageSP image = m_image.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);
    image->notifySelectionChanged();
}
