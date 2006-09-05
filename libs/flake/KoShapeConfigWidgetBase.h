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

#ifndef KOSHAPECONFIGWIDGETBASE_H
#define KOSHAPECONFIGWIDGETBASE_H

#include <QWidget>

#include <koffice_export.h>
#include <kaction.h>

#include <KoUnit.h>

class KoShape;

/**
 * Base widget for shape-configuration panels.
 * This is an interface type class used by classes that intend to provide
 * a GUI for configuring newly created shapes as created by a KoShapeFactory.
 *
 * Every time after a shape is created the KoShapeFactory for that shape-type
 * will be queried for all the config widgets; both factory specific as well as
 * those set by the hosting application.
 * A dialog will be shown with all those panels, each extending this class.
 * The framework will then call open() to populate the widget with data from
 * the param shape.  After the user ok-ed the dialog the save() will be called
 * to allow the widget to apply all settings from the widget to the shape.
 * Next, the createAction will be called which expects an action to be created
 * with an execute() and unexecute() to redo or undo the changes the user made
 * in this specific dialog.
 */
class FLAKE_EXPORT KoShapeConfigWidgetBase : public QWidget {
public:
    /**
     * Default constructor
     */
    KoShapeConfigWidgetBase() {};
    virtual ~KoShapeConfigWidgetBase() {}

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
    virtual KAction *createAction() = 0;

    /**
     * Overwrite this method to set the application unit type and update all unit-widgets
     * in this panel.
     * Called by the tool that created the shape using KoCavasBase::unit()
     * @param unit the new unit to show data in.
     */
    virtual void setUnit(KoUnit::Unit unit) { Q_UNUSED(unit); }
};


#endif
