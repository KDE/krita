/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef MODIFY_SELECTION_OPERATIONS_H
#define MODIFY_SELECTION_OPERATIONS_H

#include <operations/kis_operation.h>
#include <operations/kis_filter_selection_operation.h>

class KisViewManager;

struct GrowSelectionOperation : public KisFilterSelectionOperation {
    GrowSelectionOperation() : KisFilterSelectionOperation("growselection") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override;
};

struct ShrinkSelectionOperation : public KisFilterSelectionOperation {
    ShrinkSelectionOperation() : KisFilterSelectionOperation("shrinkselection") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override;
};

struct BorderSelectionOperation : public KisFilterSelectionOperation {
    BorderSelectionOperation() : KisFilterSelectionOperation("borderselection") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override;
};

struct FeatherSelectionOperation : public KisFilterSelectionOperation {
    FeatherSelectionOperation() : KisFilterSelectionOperation("featherselection") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override;
};

struct SmoothSelectionOperation : public KisFilterSelectionOperation {
    SmoothSelectionOperation() : KisFilterSelectionOperation("smoothselection") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override;
};

#endif // MODIFY_SELECTION_OPERATIONS_H
