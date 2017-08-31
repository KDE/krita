/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Components project.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
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
