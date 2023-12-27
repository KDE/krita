/****************************************************************************
**
** SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
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

// The PageStack item defines a container for pages and a stack-based
// navigation model. Pages can be defined as QML items or components.

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 1.1
        .
        PageStack.js

Description:
        The PageStack component provides a stack-based navigation model that you can use in your application. A stack-based navigation model means that a page of content for your application is pushed onto a stack when the user navigates deeper into the application page hierarchy. The user can then go back to the previous page (or several pages back) by popping a page (or several pages) off the top of the stack.

Properties:
      * int depth:
        The number of pages on the page stack.

      * Item currentPage:
        The page in the stack that is currently visible.

      * ToolBar toolBar:
        The toolbar container for the tools associated with each page. If a toolbar is specified, the tools set for the current page is shown to the user.
        If toolbar is null, then no tools are shown even if a page does have tools.

      * variant initialPage:
        The page to be automatically loaded when this PageStack component gets instantiated.

      * bool busy:
        Indicates whether there is an ongoing page transition.

Methods:
      * clear():
        Clears the page stack of all pages.

      * find(function):
        This iterates, top to bottom, through all the pages in the page stack and passes each page to the given function. If the specified function returns true, the iterating stops and this function returns the page that produced the true result. If no matching page is found in the page stack, null is returned.

      * pop(page, immediate):
        When you use pop() with no arguments, it pops the top page off the stack and returns that page to the caller. The normal pop transition animation is performed. If the page popped off the stack is based on the Item element, the page is re-parented back to its original parent.
        If the page didn't have an original parent (ie, was created with push(Qt.createComponent("foo.qml")) the page instance will be deleted.
        If you give a page argument, the stack is unwound to the given page. Any Item-based pages popped off the stack are re-parented to their original parent.
        If the given page is not found in the stack, the stack is unwound to the first page in the stack. However, if you specifically want to unwind the page stack to the first page in the stack, it is best to be explicit about what you are doing and use pop(null) rather than guessing a page that is not on the stack.
        The immediate argument defaults to false which means the normal transition animation is performed when a page is popped. If you do not want the transition animation to be performed pass a value of true for immediate.
        Note: A pop() on a stack with that contains 0 or 1 pages is a no-operation.
        Returns: The page that was top-most on the stack before the pop operation.

      * push(page, properties, immediate):
        Pushes the given page onto the page stack. You can use a component, item or string for the page. If the page is based on the Item element, the page is re-parented. If a string is used then it is interpreted as a URL that is used to load a page component. The push operation results in the appropriate transition animation being run. If you are pushing an array of pages, the transition animation is only shown for the last page.
        Returns: The new top page on the stack.
        The page argument can also be an array of pages. In this case, all the pages in the array are pushed onto the stack. The items in the array can be components, items or strings just like for pushing a single page.
        The page argument can also be an object that specifies a page with properties or even an array of pages with properties.
        The properties argument is optional and allows you to specify values for properties in the page being pushed.
        The immediate argument defaults to false which means the normal transition animation is performed when a page is pushed. If you do not want the transition animation to be performed pass a value of true for immediate.
        Note: When the stack is empty, a push() or replace() does not perform a transition animation because there is no page to transition from. The only time this normally happens is when an application is starting up so it is not appropriate to have a transition animation anyway.
        See also Page.

      * replace(page, properties, immediate):
        Replaces the top-most page on the stack with page. As in the push() operation, you can use a component, item or string for the page, or even an array of pages. If the page is based on the Item element, the page is re- parented. As in the pop() operation, the replaced page on the stack is re- parented back to its original parent.
        Returns: The new top page on the stack.
        See also push().

**/

import QtQuick 2.3
import org.krita.sketch 1.0
import "."
import "PageStack.js" as Engine

Item {
    id: root

    width: parent ? parent.width : 0
    height: parent ? parent.height : 0

    property int depth: Engine.getDepth()
    property Item currentPage: null
    property Item toolBar
    property variant initialPage

    property int transitionDuration: Constants.AnimationDuration

    // Indicates whether there is an ongoing page transition.
    property bool busy: internal.ongoingTransitionCount > 0

    // Pushes a page on the stack.
    // The page can be defined as a component, item or string.
    // If an item is used then the page will get re-parented.
    // If a string is used then it is interpreted as a url that is used to load a page component.
    //
    // The page can also be given as an array of pages. In this case all those pages will be pushed
    // onto the stack. The items in the stack can be components, items or strings just like for single
    // pages. Additionally an object can be used, which specifies a page and an optional properties
    // property. This can be used to push multiple pages while still giving each of them properties.
    // When an array is used the transition animation will only be to the last page.
    //
    // The properties argument is optional and allows defining a map of properties to set on the page.
    // If the immediate argument is true then no transition animation is performed.
    // Returns the page instance.
    function push(page, properties, immediate)
    {
        return Engine.push(page, properties, false, immediate);
    }

    // Pops a page off the stack.
    // If page is specified then the stack is unwound to that page, to unwind to the first page specify
    // page as null. If the immediate argument is true then no transition animation is performed.
    // Returns the page instance that was popped off the stack.
    function pop(page, immediate)
    {
        return Engine.pop(page, immediate);
    }

    // Replaces a page on the stack.
    // See push() for details.
    function replace(page, properties, immediate)
    {
        return Engine.push(page, properties, true, immediate);
    }

    // Clears the page stack.
    function clear()
    {
        return Engine.clear();
    }

    // Iterates through all pages (top to bottom) and invokes the specified function.
    // If the specified function returns true the search stops and the find function
    // returns the page that the iteration stopped at. If the search doesn't result
    // in any page being found then null is returned.
    function find(func)
    {
        return Engine.find(func);
    }

    // Called when the page stack visibility changes.
    onVisibleChanged: {
        if (currentPage) {
            internal.setPageStatus(currentPage, visible ? pageStatus.active : pageStatus.inactive);
            if (visible)
                currentPage.visible = currentPage.parent.visible = true;
        }
    }

    onInitialPageChanged: {
        if (!internal.completed) {
            return
        }

        if (initialPage) {
            if (depth == 0) {
                push(initialPage, null, true)
            } else if (depth == 1) {
                replace(initialPage, null, true)
            } else {
                console.log("Cannot update PageStack.initialPage")
            }
        }
    }

    Component.onCompleted: {
        internal.completed = true
        if (initialPage && depth == 0)
            push(initialPage, null, true)
    }

    QtObject {
        id: internal

        // The number of ongoing transitions.
        property int ongoingTransitionCount: 0

        //FIXME: there should be a way to access to them without storing it in an ugly way
        property bool completed: false

        // Sets the page status.
        function setPageStatus(page, status)
        {
            if (page != null) {
                if (page.status !== undefined) {
                    if (status == pageStatus.active && page.status == pageStatus.inactive)
                        page.status = pageStatus.activating;
                    else if (status == pageStatus.inactive && page.status == pageStatus.active)
                        page.status = pageStatus.deactivating;

                    page.status = status;
                }
            }
        }
    }

    QtObject {
        id: pageStatus;

        property int active: 0;
        property int inactive: 1;
        property int activating: 2;
        property int deactivating: 3;
    }

    // Component for page containers.
    Component {
        id: containerComponent

        Item {
            id: container

            width: parent ? parent.width : 0
            height: parent ? parent.height : 0

            // The states correspond to the different possible positions of the container.
            state: "Hidden"

            // The page held by this container.
            property Item page: null

            // The owner of the page.
            property Item owner: null

            // The width of the longer stack dimension
            property int stackWidth: Math.max(root.width, root.height)

            // Duration of transition animation (in ms)


            // Flag that indicates the container should be cleaned up after the transition has ended.
            property bool cleanupAfterTransition: false

            // Flag that indicates if page transition animation is running
            property bool transitionAnimationRunning: false

            // State to be set after previous state change animation has finished
            property string pendingState: "none"

            // Ensures that transition finish actions are executed
            // in case the object is destroyed before reaching the
            // end state of an ongoing transition
            Component.onDestruction: {
                if (transitionAnimationRunning)
                    transitionEnded();
            }

            // Sets pending state as current if state change is delayed
            onTransitionAnimationRunningChanged: {
                if (!transitionAnimationRunning && pendingState != "none") {
                    state = pendingState;
                    pendingState = "none";
                }
            }

            // Handles state change depending on transition animation status
            function setState(newState)
            {
                if (transitionAnimationRunning)
                    pendingState = newState;
                else
                    state = newState;
            }

            // Performs a push enter transition.
            function pushEnter(immediate, orientationChanges)
            {
                if (!immediate) {
                    if (orientationChanges)
                        setState("LandscapeRight");
                    else
                        setState("Right");
                }
                setState("");
                page.visible = true;
                if (root.visible && immediate)
                    internal.setPageStatus(page, pageStatus.active);
            }

            // Performs a push exit transition.
            function pushExit(replace, immediate, orientationChanges)
            {
                if (orientationChanges)
                    setState(immediate ? "Hidden" : "LandscapeLeft");
                else
                    setState(immediate ? "Hidden" : "Left");
                if (root.visible && immediate)
                    internal.setPageStatus(page, pageStatus.inactive);
                if (replace) {
                    if (immediate)
                        cleanup();
                    else
                        cleanupAfterTransition = true;
                }
            }

            // Performs a pop enter transition.
            function popEnter(immediate, orientationChanges)
            {
                if (!immediate)
                    state = orientationChanges ? "LandscapeLeft" : "Left";
                setState("");
                page.visible = true;
                if (root.visible && immediate)
                    internal.setPageStatus(page, pageStatus.active);
            }

            // Performs a pop exit transition.
            function popExit(immediate, orientationChanges)
            {
                if (orientationChanges)
                    setState(immediate ? "Hidden" : "LandscapeRight");
                else
                    setState(immediate ? "Hidden" : "Right");

                if (root.visible && immediate)
                    internal.setPageStatus(page, pageStatus.inactive);
                if (immediate)
                    cleanup();
                else
                    cleanupAfterTransition = true;
            }

            // Called when a transition has started.
            function transitionStarted()
            {
                transitionAnimationRunning = true;
                internal.ongoingTransitionCount++;
                if (root.visible) {
                    internal.setPageStatus(page, (state == "") ? pageStatus.activating : pageStatus.deactivating);
                }
            }

            // Called when a transition has ended.
            function transitionEnded()
            {
                if (state != "")
                    state = "Hidden";
                if (root.visible)
                    internal.setPageStatus(page, (state == "") ? pageStatus.active : pageStatus.inactive);

                internal.ongoingTransitionCount--;
                transitionAnimationRunning = false;
                if (cleanupAfterTransition)
                    cleanup();
            }

            states: [
                // Explicit properties for default state.
                State {
                    name: ""
                    PropertyChanges { target: container; visible: true; opacity: 1 }
                },
                // Start state for pop entry, end state for push exit.
                State {
                    name: "Left"
                    PropertyChanges { target: container; x: -width / 2; opacity: 0 }
                },
                // Start state for pop entry, end state for push exit
                // when exiting portrait and entering landscape.
                State {
                    name: "LandscapeLeft"
                    PropertyChanges { target: container; x: -stackWidth / 2; opacity: 0 }
                },
                // Start state for push entry, end state for pop exit.
                State {
                    name: "Right"
                    PropertyChanges { target: container; x: width / 2; opacity: 0 }
                },
                // Start state for push entry, end state for pop exit
                // when exiting portrait and entering landscape.
                State {
                    name: "LandscapeRight"
                    PropertyChanges { target: container; x: stackWidth / 2; opacity: 0 }
                },
                // Inactive state.
                State {
                    name: "Hidden"
                    PropertyChanges { target: container; visible: false }
                }
            ]

            transitions: [
                // Push exit transition
                Transition {
                    from: ""; to: "Left"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.InQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop entry transition
                Transition {
                    from: "Left"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.OutQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Push exit transition landscape
                Transition {
                    from: ""; to: "LandscapeLeft"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.InQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop entry transition landscape
                Transition {
                    from: "LandscapeLeft"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.OutQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop exit transition
                Transition {
                    from: ""; to: "Right"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.InQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        // Workaround for transition animation bug causing ghost view with page pop transition animation
                        // TODO: Root cause still unknown
                        PropertyAnimation {}
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Push entry transition
                Transition {
                    from: "Right"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.OutQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Pop exit transition landscape
                Transition {
                    from: ""; to: "LandscapeRight"
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.InQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        // Workaround for transition animation bug causing ghost view with page pop transition animation
                        // TODO: Root cause still unknown
                        PropertyAnimation {}
                        ScriptAction { script: transitionEnded() }
                    }
                },
                // Push entry transition landscape
                Transition {
                    from: "LandscapeRight"; to: ""
                    SequentialAnimation {
                        ScriptAction { script: transitionStarted() }
                        ParallelAnimation {
                            PropertyAnimation { properties: "x"; easing.type: Easing.OutQuad; duration: transitionDuration }
                            PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: transitionDuration }
                        }
                        ScriptAction { script: transitionEnded() }
                    }
                }
            ]

            // Cleans up the container and then destroys it.
            function cleanup()
            {
                if (page != null) {
                    if (page.status == pageStatus.active) {
                        internal.setPageStatus(page, pageStatus.inactive)
                    }
                    if (owner != container) {
                        // container is not the owner of the page - re-parent back to original owner
                        page.visible = false;
                        page.anchors.fill = undefined
                        page.parent = owner;
                    }
                }

                container.destroy();
            }
        }
    }
}

