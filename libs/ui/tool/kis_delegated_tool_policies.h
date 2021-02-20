/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DELEGATED_TOOL_POLICIES_H
#define __KIS_DELEGATED_TOOL_POLICIES_H

#include <QtGlobal>

#include "kritaui_export.h"


class KoCanvasBase;

struct KRITAUI_EXPORT NoopActivationPolicy {
    static inline void onActivate(KoCanvasBase *canvas) {
        Q_UNUSED(canvas);
    }
};

struct KRITAUI_EXPORT DeselectShapesActivationPolicy {
    static void onActivate(KoCanvasBase *canvas);
};


#endif /* __KIS_DELEGATED_TOOL_POLICIES_H */
