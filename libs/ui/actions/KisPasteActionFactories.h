/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

struct KRITAUI_EXPORT KisPasteNewActionFactory : public KisNoParameterActionFactory {
    KisPasteNewActionFactory() : KisNoParameterActionFactory("paste-new-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisPasteReferenceActionFactory : public KisNoParameterActionFactory {
    KisPasteReferenceActionFactory() : KisNoParameterActionFactory("paste-reference-ui-action") {}
    void run(KisViewManager *view) override;
};

#endif // KISPASTEACTIONFACTORY_H
