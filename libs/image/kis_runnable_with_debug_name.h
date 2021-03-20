/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_RUNNABLE_WITH_DEBUG_NAME_H
#define KIS_RUNNABLE_WITH_DEBUG_NAME_H

#include "kis_runnable.h"
#include <QString>

class KRITAIMAGE_EXPORT KisRunnableWithDebugName : public KisRunnable
{
public:
    virtual QString debugName() const = 0;
};

#endif // KIS_RUNNABLE_WITH_DEBUG_NAME_H
