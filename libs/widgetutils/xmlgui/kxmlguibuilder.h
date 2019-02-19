/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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
   Boston, MA 02110-1301, USA.
*/
#ifndef kxmlguibuilder_h
#define kxmlguibuilder_h

#include <kritawidgetutils_export.h>

class KXMLGUIBuilderPrivate;
class KXMLGUIClient;

class QAction;
class QDomElement;
class QStringList;
class QWidget;

/**
 * Implements the creation of the GUI (menubar, menus and toolbars)
 * as requested by the GUI factory.
 *
 * The virtual methods are mostly for historical reasons, there isn't really
 * a need to derive from KXMLGUIBuilder anymore.
 */
class KRITAWIDGETUTILS_EXPORT KXMLGUIBuilder
{
public:

    explicit KXMLGUIBuilder(QWidget *widget);
    virtual ~KXMLGUIBuilder();

    /* @internal */
    KXMLGUIClient *builderClient() const;
    /* @internal */
    void setBuilderClient(KXMLGUIClient *client);
    /* @internal */
    QWidget *widget();

    virtual QStringList containerTags() const;

    /**
     * Creates a container (menubar/menu/toolbar/statusbar/separator/...)
     * from an element in the XML file
     *
     * @param parent The parent for the container
     * @param index The index where the container should be inserted
     *              into the parent container/widget
     * @param element The element from the DOM tree describing the
     *                container (use it to access container specified
     *                attributes or child elements)
     * @param containerAction The action created for this container; used for e.g. passing to removeContainer.
     */
    virtual QWidget *createContainer(QWidget *parent, int index,
                                     const QDomElement &element, QAction *&containerAction);

    /**
     * Removes the given (and previously via createContainer )
     * created container.
     *
     */
    virtual void removeContainer(QWidget *container, QWidget *parent,
                                 QDomElement &element, QAction *containerAction);

    virtual QStringList customTags() const;

    virtual QAction *createCustomElement(QWidget *parent, int index, const QDomElement &element);

    virtual void removeCustomElement(QWidget *parent, QAction *action);

    virtual void finalizeGUI(KXMLGUIClient *client);

protected:
    virtual void virtual_hook(int id, void *data);
private:
    KXMLGUIBuilderPrivate *const d;
};

#endif

