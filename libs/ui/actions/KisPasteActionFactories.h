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
    enum Flag {
        None = 0x0,
        PasteAtCursor = 0x1,
        ForceNewLayer = 0x2
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    KisPasteActionFactory() : KisOperation("paste-ui-action") {}

    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override {
        Flags flags;
        flags.setFlag(PasteAtCursor, config.getBool("paste-at-cursor-position", false));
        flags.setFlag(ForceNewLayer, config.getBool("force-new-layer", false));
        run(flags, view);
    }

    void run(Flags flags, KisViewManager *view);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisPasteActionFactory::Flags)

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
