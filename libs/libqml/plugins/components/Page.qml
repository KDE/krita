/****************************************************************************
**
** SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Components project.
**
** $QT_BEGIN_LICENSE:BSD$
** SPDX-License-Identifier: BSD-3-Clause
**
****************************************************************************/

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 1.1
        everything in the same dir which are version 0.1

Description:
        Defines the content of a piece of the user interface, it's meant to be loaded by a PageStack or TabGroup element.
        The typical use can be in small plasmoids or  mobile devices where the whole screen is a series of interchangeable and flickable pages, of which the user navigates across.


Properties:
        * int status:
        The current status of the page. It can be one of the following values:
        PageStatus.Inactive (default) - the page is not visible.
        PageStatus.Activating - the page is becoming to the active page.
        PageStatus.Active - the page is the currently active page.
        PageStatus.Deactivating - the page is becoming to inactive page.

        * PageStack pageStack:
        The page stack that this page is owned by.

        * int orientationLock:
        Sets the orientation for the Page

        * Item tools:
        Defines the toolbar contents for the page. If the page stack is set up using a toolbar instance, it automatically shows the currently active page's toolbar contents in the toolbar. The default value is null resulting in the page's toolbar to be invisible when the page is active.
**/

import QtQuick 2.3

import "."

Item {
    id: root

    // The status of the page. One of the following:
    //      PageStatus.Inactive - the page is not visible
    //      PageStatus.Activating - the page is transitioning into becoming the active page
    //      PageStatus.Active - the page is the current active page
    //      PageStatus.Deactivating - the page is transitioning into becoming inactive
    property int status: 0;

    property Item pageStack

    property Item tools: null

    visible: false

    width: visible && parent ? parent.width : internal.previousWidth
    height: visible && parent ? parent.height : internal.previousHeight

    onWidthChanged: internal.previousWidth = (visible ? width : internal.previousWidth)
    onHeightChanged: internal.previousHeight = (visible ? height : internal.previousHeight)

    // This is needed to make a parentless Page component visible in the Designer of QtCreator.
    // It's done here instead of binding the visible property because binding it to a script
    // block returning false would cause an element on the Page not to end up focused despite
    // specifying focus=true inside the active focus scope. The focus would be gained and lost
    // in createObject.
    Component.onCompleted: if (!parent) visible = true

    QtObject {
        id: internal
        property int previousWidth: 0
        property int previousHeight: 0
    }
}
