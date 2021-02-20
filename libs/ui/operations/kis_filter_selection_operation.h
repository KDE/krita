/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTER_SELECTION_OPERATION_H
#define KIS_FILTER_SELECTION_OPERATION_H

#include <kritaui_export.h>
#include "kis_operation.h"

class KisSelectionFilter;
class KisViewManager;

struct KRITAUI_EXPORT KisFilterSelectionOperation : public KisOperation {
    KisFilterSelectionOperation(const QString& id) : KisOperation(id) {}
    void runFilter(KisSelectionFilter* filter, KisViewManager *view, const KisOperationConfiguration &config);
};

#endif // KIS_FILTER_SELECTION_OPERATION_H
