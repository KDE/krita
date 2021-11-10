/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRESETGROUPLAYERCACHECOMMAND_H
#define KISRESETGROUPLAYERCACHECOMMAND_H

#include "kis_types.h"
#include "kis_command_utils.h"

class KoColorSpace;

class KisResetGroupLayerCacheCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    KisResetGroupLayerCacheCommand(KisGroupLayerSP layer, const KoColorSpace *dstColorSpace, State state);

    void partB();

private:
    KisGroupLayerSP m_layer;
    const KoColorSpace *m_dstColorSpace;
};

#endif // KISRESETGROUPLAYERCACHECOMMAND_H
