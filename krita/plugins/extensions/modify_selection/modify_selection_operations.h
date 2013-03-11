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


#ifndef MODIFY_SELECTION_OPERATIONS_H
#define MODIFY_SELECTION_OPERATIONS_H

#include <operations/kis_operation.h>
#include <operations/kis_filter_selection_operation.h>

class KisView2;
class KisSelectionFilter;

struct GrowSelectionOperation : public KisFilterSelectionOperation {
    GrowSelectionOperation() : KisFilterSelectionOperation("growselection") {}
    void runFromXML(KisView2 *view, const KisOperationConfiguration &config);
};

struct ShrinkSelectionOperation : public KisFilterSelectionOperation {
    ShrinkSelectionOperation() : KisFilterSelectionOperation("shrinkselection") {}
    void runFromXML(KisView2 *view, const KisOperationConfiguration &config);
};

struct BorderSelectionOperation : public KisFilterSelectionOperation {
    BorderSelectionOperation() : KisFilterSelectionOperation("borderselection") {}
    void runFromXML(KisView2 *view, const KisOperationConfiguration &config);
};

struct FeatherSelectionOperation : public KisFilterSelectionOperation {
    FeatherSelectionOperation() : KisFilterSelectionOperation("featherselection") {}
    void runFromXML(KisView2 *view, const KisOperationConfiguration &config);
};

struct SmoothSelectionOperation : public KisFilterSelectionOperation {
    SmoothSelectionOperation() : KisFilterSelectionOperation("smoothselection") {}
    void runFromXML(KisView2 *view, const KisOperationConfiguration &config);
};

#endif // MODIFY_SELECTION_OPERATIONS_H
