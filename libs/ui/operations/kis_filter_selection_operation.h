/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
