/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_RUNNABLE_H
#define __KIS_RUNNABLE_H

#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisRunnable
{
public:
    virtual ~KisRunnable() {};
    virtual void run() = 0;
};

#endif /* __KIS_RUNNABLE_H */
