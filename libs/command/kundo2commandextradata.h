/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KUNDO2COMMANDEXTRADATA_H
#define __KUNDO2COMMANDEXTRADATA_H

#include "kritacommand_export.h"


class KRITACOMMAND_EXPORT KUndo2CommandExtraData
{
public:
    virtual ~KUndo2CommandExtraData();
    virtual KUndo2CommandExtraData* clone() const = 0;
};

#endif /* __KUNDO2COMMANDEXTRADATA_H */
