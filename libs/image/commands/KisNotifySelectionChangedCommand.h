/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISNOTIFYSELECTIONCHANGEDCOMMAND_H
#define KISNOTIFYSELECTIONCHANGEDCOMMAND_H

#include <kritaimage_export.h>
#include <kis_command_utils.h>
#include "kis_types.h"

class KRITAIMAGE_EXPORT KisNotifySelectionChangedCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    KisNotifySelectionChangedCommand(KisImageWSP image, State state);
    void partB();

private:
    KisImageWSP m_image;
};

#endif // KISNOTIFYSELECTIONCHANGEDCOMMAND_H
