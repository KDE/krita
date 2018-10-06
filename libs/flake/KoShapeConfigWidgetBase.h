/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPECONFIGWIDGETBASE_H
#define KOSHAPECONFIGWIDGETBASE_H

#include <QWidget>

#include "kritaflake_export.h"

class KoShape;
class KUndo2Command;
class KoUnit;
class KoCanvasResourceProvider;

/**
 * Base widget for shape-configuration panels.
 * This is an interface type class used by classes that intend to provide
 * a GUI for configuring newly created shapes as created by a KoShapeFactoryBase.
 *
 * Every time after a shape is created the KoShapeFactoryBase for that shape-type
 * will be queried for all the config widgets; both factory specific as well as
 * those set by the hosting application.
 * A dialog will be shown with all those panels, each extending this class.
 * The framework will then call open() to populate the widget with data from
 * the param shape.  After the user ok-ed the dialog the save() will be called
 * to allow the widget to apply all settings from the widget to the shape.
 */
class KRITAFLAKE_EXPORT KoShapeConfigWidgetBase : public QWidget
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    KoShapeConfigWidgetBase();
    ~KoShapeConfigWidgetBase() override;

    /**
     * Open the argument shape by interpreting the data and setting that data on this
     * widget.
     * @param shape the shape that is to be queried for the data this widget can edit.
     */
    virtual void open(KoShape *shape) = 0;
    /**
     * Save the data  of this widget to the shape passed to open earlier to
     * apply any user changed options.
     * Called by the tool that created the shape.
     */
    virtual void save() = 0;

    /**
     * Overwrite this method to set the application unit type and update all unit-widgets
     * in this panel.
     * Called by the tool that created the shape using KoCavasBase::unit()
     * @param unit the new unit to show data in.
     */
    virtual void setUnit(const KoUnit &unit);

    /// called to set the canvas resource manager of the canvas the user used to insert the new shape.
    void setResourceManager(KoCanvasResourceProvider *rm);

    /// Return true if the shape config panel should be shown after the shape is created
    virtual bool showOnShapeCreate();

    /// Return true if the shape config panel should be shown when the shape is selected
    virtual bool showOnShapeSelect();

    /// Creates a command which applies all changes to the opened shape
    virtual KUndo2Command * createCommand();

Q_SIGNALS:
    /// is emitted after one of the config options has changed
    void propertyChanged();

    /// is emitted when the dialog should be accepted ie a file double clicked in a filebrowser
    void accept();

protected:
    KoCanvasResourceProvider *m_resourceManager; ///< the resource provider with data for this canvas
};

#endif
