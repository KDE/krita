/* This file is part of the KDE project
 *  Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KO_TOOL_MANAGER
#define KO_TOOL_MANAGER

#include <koffice_export.h>
#include <QMap>
#include <QObject>
#include <QCursor>
#include <QStack>

class ToolHelper;
class KoCanvasController;
class KoCanvasBase;
class KoTool;
class KoShapeControllerBase;
class KoToolBox;
class KoCreateShapesTool;
class KActionCollection;
class KoShape;

/**
 * This class manages the activation and deactivation of tools for
 * each input device.
 * Managing the active tool and switching tool based on various variables.
 *
 * The state of the toolbox will be the same for all views in the process so practically
 * you can say we have one toolbox per application instance (process).  Implementation
 * does not allow one widget to be in more then one view, so we just make sure the toolbox
 * is hidden in not-in-focus views.
 *
 * There is one tool instance per pointer
 * There is one set of tools per pointer per process
 * All views share this set of tools
 * The tool manager knows all canvasController intances
 * The tool manager set the active tool in all canvasses
 * (Tools can set another tool as active)
 */
class KOFFICEUI_EXPORT KoToolManager : public QObject {
    Q_OBJECT

public:
    static KoToolManager* instance();
    ~KoToolManager();

    KoToolBox *toolBox(const QString &applicationName = QString());
    void registerTools(KActionCollection *ac);
    void addControllers(KoCanvasController *controller, KoShapeControllerBase *sc);
    void removeCanvasController(KoCanvasController *controller);

    KoCreateShapesTool *shapeCreatorTool(KoCanvasBase *canvas) const;

signals:
    void changedTool(int uniqueToolId);
    void toolCodesSelected(QList<QString> types);

private:
    KoToolManager();
    KoToolManager(const KoToolManager&);
    KoToolManager operator=(const KoToolManager&);
    void setup();
    void switchTool(KoTool *tool, bool temporary);
    void switchTool(const QString &id, bool temporary);

private slots:
    void toolActivated(ToolHelper *tool);
    void detachCanvas(KoCanvasController* controller);
    void attachCanvas(KoCanvasController* controller);
    void updateCursor(QCursor cursor);
    void switchToolRequested(const QString &id);
    void switchToolTemporaryRequested(const QString &id);
    void switchBackRequested();
    void selectionChanged(QList<KoShape*> shapes);

private:
    static KoToolManager* s_instance;

    QList<ToolHelper*> m_tools;
    QMap<KoCanvasController*, KoShapeControllerBase*> m_shapeControllers;
    QList<KoCanvasController*> m_canvases;
    KoCanvasController *m_activeCanvas;
    KoTool *m_activeTool, *m_dummyTool;
    ToolHelper *m_defaultTool;

    QMap<KoTool*, int> m_uniqueToolIds;
    QMap<KoCanvasController*, QMap<QString, KoTool*> > m_allTools;
    QStack<KoTool*> m_stack;
};

#endif
