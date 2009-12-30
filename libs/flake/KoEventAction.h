/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOEVENTACTION_H
#define KOEVENTACTION_H

#include "flake_export.h"
#include "KoXmlReaderForward.h"
class KoShapeLoadingContext;
class KoShapeSavingContext;
class KoTool;

/**
 * This is the base class for actions that are executed on events.
 *
 * See ODF:
 * 9.9 Presentation Events
 * Many objects inside a presentation document support special presentation events. For example, a
 * user can advance the presentation one frame when he clicks on an object with a corresponding
 * event. Presentation events are contained with a graphic object's event listener table. See section
 * 9.2.21 for details.
 *
 * 12.4 Event Listener Tables
 * Many objects such as controls, images, text boxes, or an entire document support events. An
 * event binds the occurrence of a particular condition to an action that is executed if the condition
 * arises. For example, if a user places the cursor over a graphic, this condition triggers an action
 * that is supported by the office application. This event, called "on-mouse-over", can be associated
 * with a macro that is executed whenever the condition occurs, that is, whenever a user places the
 * cursor over a graphic.
 */
class FLAKE_EXPORT KoEventAction
{
public:
    /**
     * Constructor
     */
    KoEventAction();
    virtual ~KoEventAction();

    /**
     * Set The id of the action
     *
     * @param id this is the value that is used for storing the event action in odf.
     */
    void setId(const QString &id);

    /**
     * The id of the action
     *
     * The id is the value that is used for storing the event action in odf.
     */
    QString id() const;

    /**
     * Load action from ODF.
     *
     * @param context the KoShapeLoadingContext used for loading
     * @param element element which represents the shape in odf
     *
     * @return false if loading failed
     */
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) = 0;

    /**
     * Store the action as ODF.
     *
     * @param context The KoShapeSavingContext used for saving
     */
    virtual void saveOdf(KoShapeSavingContext &context) const = 0;

    /**
     * Start the action.
     */
    virtual void start() = 0;
    /**
     * Finish the action
     *
     * If the action takes some time to finish it can bs stoped with
     * this method before its end.
     */
    virtual void finish() {}

private:
    class Private;
    Private * const d;
};

#endif /* KOEVENTACTION_H */
