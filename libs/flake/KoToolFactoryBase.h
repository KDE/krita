/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KO_TOOL_FACTORY_H
#define KO_TOOL_FACTORY_H

#include "kritaflake_export.h"

#include <QString>
#include <QList>
#include <QObject>

class KoCanvasBase;
class KoToolBase;
class QKeySequence;
class KActionCollection;
class QAction;

/**
 * Each tool has a "section" which it uses to be grouped in the toolbox.
 */
namespace ToolBoxSection {
    static const QString Main {"main"};                   ///< Tools that only work on vector shapes
    static const QString Shape {"0 Krita/Shape"};         ///< Freehand and shapes like ellipses and lines
    static const QString Transform {"2 Krita/Transform"}; ///< Tools that transform the layer
    static const QString Fill {"3 Krita/Fill"};           ///< Tools that fill parts of the canvas
    static const QString View {"4 Krita/View"};           ///< Assistance tools: guides, reference, etc.
    static const QString Select {"5 Krita/Select"};       ///< Tools that select pixels
    static const QString Navigation {"navigation"};       ///< Tools that affect the canvas: pan, zoom, etc.
}

/**
 * A factory for KoToolBase objects.
 *
 * The baseclass for all tool plugins. Each plugin that ships a KoToolBase should also
 * ship a factory. That factory will extend this class and set variable data like
 * a toolTip and icon in the constructor of that extending class.
 *
 * An example usage would be:<pre>
 * class MyToolFactory : public KoToolFactoryBase {
 * public:
 *   MyToolFactory(const QStringList&)
 *       : KoToolFactoryBase("MyTool") {
 *       setToolTip(i18n("Create object"));
 *       setSection("main");
 *       setPriority(5);
 *   }
 *   ~MyToolFactory() {}
 *   KoToolBase *createTool(KoCanvasBase *canvas);
 * };
 * K_PLUGIN_FACTORY_WITH_JSON((MyToolFactoryFactory, "mytool.json", registerPlugin<MyToolFactory>();)
</pre>

 */
class KRITAFLAKE_EXPORT KoToolFactoryBase : public QObject
{

    Q_OBJECT

public:
    /**
     * Create the new factory
     * @param id a string that will be used internally for referencing the tool
     */
    explicit KoToolFactoryBase(const QString &id);
    virtual ~KoToolFactoryBase();

    /**
     * Create the actions for this tool. Actions are unique per window, not per
     * tool instance; tool instances are unique per view/canvas.
     */
    QList<QAction *> createActions(KActionCollection *actionCollection);

    /**
     * Instantiate a new tool
     * @param canvas the canvas that the new tool will work on. Should be passed
     *    to the constructor of the tool.
     * @return a new KoToolBase instance, or zero if the tool doesn't want to show up.
     */
    virtual KoToolBase *createTool(KoCanvasBase *canvas) = 0;

    /**
     * return the id for the tool this factory creates.
     * @return the id for the tool this factory creates.
     */
    QString id() const;
    /**
     * Returns The priority of this tool in its section in the toolbox
     * @return The priority of this tool.
     */
    int priority() const;
    /**
     * returns the section used to group tools in the toolbox
     * @return the section
     */
    QString section() const;
    /**
     * return a translated tooltip Text
     * @return a translated tooltip Text
     */
    QString toolTip() const;
    /**
     * return the basename of the icon for this tool
     * @return the basename of the icon for this tool
     */
    QString iconName() const;

    /**
     * Return the id of the shape we can process.
     * This is the shape Id the tool we create is associated with.  So a TextTool for a TextShape.
     * @see KoShapeFactoryBase::shapeId()
     * @see setActivationShapeId()
     * @return the id of a shape, or an empty string for all shapes.
     */
    QString activationShapeId() const;

    /**
     * Return the default keyboard shortcut for activation of this tool (if
     * the shape this tool belongs to is active).
     *
     * See KoToolManager for use.
     *
     * @return the shortcut
     */
    QKeySequence shortcut() const;

protected:

    /**
     * Set the default shortcut for activation of this tool.
     */
    void setShortcut(const QKeySequence & shortcut);

    /**
     * Set the tooltip to be used for this tool
     * @param tooltip the tooltip
     */
    void setToolTip(const QString &tooltip);

    /**
     * Set the section used to group tools in the toolbox
     * @param section the section
     */
    void setSection(const QString &section);

    /**
     * Set an icon to be used in the toolBox.
     * @param iconName the basename (without extension) of the icon
     */
    void setIconName(const char *iconName);
    void setIconName(const QString &iconName);

    /**
     * Set the priority of this tool, as it is shown in the toolBox; lower number means
     * it will be show more to the front of the list.
     * @param newPriority the priority
     */
    void setPriority(int newPriority);

    /**
     * Set the id of the shape we can process.
     * This is the Id, as passed to the constructor of a KoShapeFactoryBase, that the tool
     * we create is associated with. This means that if a KoTextShape is selected, then
     * all tools that have its id set here will be added to the dynamic part of the toolbox.
     * @param activationShapeId the Id of the shape
     * @see activationShapeId()
     */
    void setActivationShapeId(const QString &activationShapeId);

    /**
     * @brief createActionsImpl should be reimplemented if the tool needs any actions.
     * The actions should have a valid objectName().
     *
     * @return the list of actions this tool wishes to be available.
     */
    virtual QList<QAction *> createActionsImpl();

private Q_SLOTS:

    void activateTool();

private:
    class Private;
    Private * const d;
};

#endif
