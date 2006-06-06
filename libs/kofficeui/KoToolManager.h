/*
 *  Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#ifndef KO_TOOL_MANAGER
#define KO_TOOL_MANAGER

#include "KoToolBox.h"

#include <KoID.h>
#include <KoCanvasView.h>
#include <koffice_export.h>

#include <QMap>
#include <QObject>

class ToolHelper;
class KActionCollection;
class QAbstractButton;
class KoToolFactory;

/**
 * This class manages the activation and deactivation of tools for
 * each input device.
 *
 * We assume ONE toolbox per application instance (process), not one
 * per view. This one toolbox is owned and managed by this
 * toolmanager.  We will create that without using actions, just signal slots.
 *
 * There is one tool instance per pointer
 * There is one set of tools per pointer per process
 * All views share this set of tools
 * The tool manager knows all canvasView intances
 * The tool manager set the active tool in all canvasses
 * (Tools can set another tool as active)
 */
class KOFFICEUI_EXPORT KoToolManager : public QObject {
    Q_OBJECT

public:
    static KoToolManager* instance();
    ~KoToolManager();

    QWidget *createToolBox(); // TODO alter from QWidget to KoToolBox
    void registerTools(KActionCollection *ac);
    void addCanvasView(const KoCanvasView *view);

private:
    KoToolManager();
    KoToolManager(const KoToolManager&);
    KoToolManager operator=(const KoToolManager&);
    void setup();

private slots:
    void toolActivated(ToolHelper *tool);

private:
    static KoToolManager* s_instance;

    KoToolBox *m_toolBox;

    QList<ToolHelper*> m_tools;
};

class ToolHelper : public QObject {
    Q_OBJECT
public:
    ToolHelper(KoToolFactory *tool) { m_toolFactory = tool; }
    QAbstractButton *createButton(QWidget *parent);

    KoID id() const;

signals:
    void toolActivated(ToolHelper *tool);

private slots:
    void buttonPressed();

private:
    KoToolFactory *m_toolFactory;
};

#endif
