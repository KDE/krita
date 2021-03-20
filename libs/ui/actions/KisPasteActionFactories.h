/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPASTEACTIONFACTORY_H
#define KISPASTEACTIONFACTORY_H

#include "KisNoParameterActionFactory.h"
#include "operations/kis_operation.h"
#include "operations/kis_operation_configuration.h"

struct KRITAUI_EXPORT KisPasteActionFactory : public KisOperation {
    KisPasteActionFactory() : KisOperation("paste-ui-action") {}

    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override {
        run(config.getBool("paste-at-cursor-position", false), view);
    }

    void run(bool pasteAtCursorPosition, KisViewManager *view);
};

struct KRITAUI_EXPORT KisPasteIntoActionFactory : public KisNoParameterActionFactory {
    KisPasteIntoActionFactory() : KisNoParameterActionFactory("paste-into-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisPasteNewActionFactory : public KisNoParameterActionFactory {
    KisPasteNewActionFactory() : KisNoParameterActionFactory("paste-new-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisPasteReferenceActionFactory : public KisNoParameterActionFactory {
    KisPasteReferenceActionFactory() : KisNoParameterActionFactory("paste-reference-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisPasteShapeStyleActionFactory : public KisNoParameterActionFactory {
    KisPasteShapeStyleActionFactory() : KisNoParameterActionFactory("paste-shape-style-action") {}
    void run(KisViewManager *view) override;
};

#endif // KISPASTEACTIONFACTORY_H
