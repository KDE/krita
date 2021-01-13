/* This file is part of the KDE project
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
#ifndef KO_TOOL_MANAGER_P
#define KO_TOOL_MANAGER_P

#include <QList>
#include <QObject>
#include <QString>
#include <QHash>

#include <QKeySequence>
#include <QAction>

#include "KoInputDevice.h"
#include "KoToolManager.h"

class KoToolFactoryBase;
class KoShapeManager;
class KoCanvasBase;
class KoToolBase;
class KoShape;
class KoToolManager;
class KoCanvasController;
class KoShapeLayer;
class ToolHelper;
class CanvasData;
class KoToolProxy;

class Q_DECL_HIDDEN KoToolManager::Private
{
public:
    Private(KoToolManager *qq);
    ~Private();

    void setup();

    void connectActiveTool();
    void disconnectActiveTool();
    void switchTool(KoToolBase *tool, bool temporary);
    void switchTool(const QString &id, bool temporary);
    void postSwitchTool(bool temporary);
    void switchCanvasData(CanvasData *cd);

    bool eventFilter(QObject *object, QEvent *event);
    void toolActivated(ToolHelper *tool);

    void detachCanvas(KoCanvasController *controller);
    void attachCanvas(KoCanvasController *controller);
    void movedFocus(QWidget *from, QWidget *to);
    void updateCursor(const QCursor &cursor);
    void switchBackRequested();
    void selectionChanged(const QList<KoShape*> &shapes);
    void currentLayerChanged(const KoShapeLayer *layer);
    void updateToolForProxy();
    void switchToolTemporaryRequested(const QString &id);
    CanvasData *createCanvasData(KoCanvasController *controller, const KoInputDevice &device);

    /**
     * Request a switch from to the param input device.
     * This will cause the tool for that device to be selected.
     */
    void switchInputDevice(const KoInputDevice &device);

    /**
     * Whenever a new tool proxy class is instantiated, it will use this method to register itself
     * so the toolManager can update it to the latest active tool.
     * @param proxy the proxy to register.
     * @param canvas which canvas the proxy is associated with; whenever a new tool is selected for that canvas,
     *        the proxy gets an update.
     */
    void registerToolProxy(KoToolProxy *proxy, KoCanvasBase *canvas);

    KoToolManager *q;

    QList<ToolHelper*> tools; // list of all available tools via their factories.

    QHash<KoToolBase*, int> uniqueToolIds; // for the changedTool signal
    QHash<KoCanvasController*, QList<CanvasData*> > canvasses;
    QHash<KoCanvasBase*, KoToolProxy*> proxies;

    CanvasData *canvasData; // data about the active canvas.

    KoInputDevice inputDevice;

    bool layerExplicitlyDisabled;
};

/// \internal
class ToolHelper : public QObject
{
    Q_OBJECT
public:
    explicit ToolHelper(KoToolFactoryBase *tool);
    KoToolAction *toolAction();
    /// wrapper around KoToolFactoryBase::id();
    QString id() const;
    /// wrapper around KoToolFactoryBase::iconName();
    QString iconName() const;
    /// descriptive text, as ;
    QString text() const;
    /// descriptive icon text, e.g. use on a button next to an icon or without one;
    QString iconText() const;
    /// tooltip of the tool, e.g. for tooltip of a button;
    QString toolTip() const;
    /// wrapper around KoToolFactoryBase::toolType();
    QString section() const;
    /// wrapper around KoToolFactoryBase::activationShapeId();
    QString activationShapeId() const;
    /// wrapper around KoToolFactoryBase::priority();
    int priority() const;
    KoToolBase *createTool(KoCanvasBase *canvas) const;
    /// unique id, >= 0
    int uniqueId() const {
        return m_uniqueId;
    }
    /// QAction->shortcut() if it exists, otherwise KoToolFactoryBase::shortcut()
    QKeySequence shortcut() const;

public Q_SLOTS:
    void activate();

Q_SIGNALS:
    /// Emitted when the tool should be activated, e.g. by pressing the tool's assigned button in the toolbox
    void toolActivated(ToolHelper *tool);

private:
    KoToolFactoryBase * const m_toolFactory;
    const int m_uniqueId;
    QKeySequence m_customShortcut;
    bool m_hasCustomShortcut;
    KoToolAction *m_toolAction;
};

/// \internal
/// Helper class to transform a simple signal selection changed into a signal with a parameter
class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(KoShapeManager *parent);

public Q_SLOTS:
    void selectionChanged();

Q_SIGNALS:
    void selectionChanged(const QList<KoShape*> &shape);

private:
    KoShapeManager *m_shapeManager;
};

#endif
