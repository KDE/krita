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

// Page stack. Items are page containers.
var pageStack = [];

// Page component cache map. Key is page url, value is page component.
var componentCache = {};

// Returns the page stack depth.
function getDepth() {
    return pageStack.length;
}

// Pushes a page on the stack.
function push(page, properties, replace, immediate) {
    // page order sanity check
    if ((!replace && page == currentPage)
        || (replace && pageStack.length > 1
        && page == pageStack[pageStack.length - 2].page)) {
        throw new Error("Cannot navigate so that the resulting page stack has two consecutive entries of the same page instance.");
    }

    // figure out if more than one page is being pushed
    var pages;
    if (page instanceof Array) {
        pages = page;
        page = pages.pop();
        if (page.createObject === undefined && page.parent === undefined && typeof page != "string") {
            properties = properties || page.properties;
            page = page.page;
        }
    }

    // get the current container
    var oldContainer = pageStack[pageStack.length - 1];

    // pop the old container off the stack if this is a replace
    if (oldContainer && replace) {
        pageStack.pop();
    }

    // push any extra defined pages onto the stack
    if (pages) {
        var i;
        for (i = 0; i < pages.length; i++) {
            var tPage = pages[i];
            var tProps;
            if (tPage.createObject === undefined && tPage.parent === undefined && typeof tPage != "string") {
                tProps = tPage.properties;
                tPage = tPage.page;
            }
            pageStack.push(initPage(tPage, tProps));
        }
    }

    // initialize the page
    var container = initPage(page, properties);

    // push the page container onto the stack
    pageStack.push(container);

    depth = pageStack.length;
    currentPage = container.page;

    // perform page transition
    immediate = immediate || !oldContainer;
    var orientationChange = false;
    if (oldContainer) {
        orientationChange = orientationChanges(oldContainer.page, container.page);
        oldContainer.pushExit(replace, immediate, orientationChange);
    }

    // sync tool bar
    var tools = container.page.tools || null;
    if (toolBar) {
        toolBar.setTools(tools, immediate ? "set" : replace ? "replace" : "push");
    }

    container.pushEnter(immediate, orientationChange);
    return container.page;
}

// Initializes a page and its container.
function initPage(page, properties) {
    var container = containerComponent.createObject(root);

    var pageComp;
    if (page.createObject) {
        // page defined as component
        pageComp = page;
    } else if (typeof page == "string") {
        // page defined as string (a url)
        pageComp = componentCache[page];
        if (!pageComp) {
            pageComp = componentCache[page] = Qt.createComponent(page);
        }
    }
    if (pageComp) {
        if (pageComp.status == Component.Error) {
            throw new Error("Error while loading page: " + pageComp.errorString());
        } else {
            // instantiate page from component
            page = pageComp.createObject(container, properties || {});
        }
    } else {
        // copy properties to the page
        for (var prop in properties) {
            if (properties.hasOwnProperty(prop)) {
                page[prop] = properties[prop];
            }
        }
    }

    container.page = page;
    if (page.parent == null) {
        container.owner = container;
    } else {
        container.owner = page.parent;
    }

    // the page has to be reparented if
    if (page.parent != container) {
        page.parent = container;
    }

    if (page.pageStack !== undefined) {
        page.pageStack = root;
    }

    page.anchors.fill = container

    return container;
}

// Pops a page off the stack.
function pop(page, immediate) {
    // make sure there are enough pages in the stack to pop
    if (pageStack.length > 1) {
        //unwind to itself means no pop
        if (page !== undefined && page == pageStack[pageStack.length - 1].page) {
            return
        }
        // pop the current container off the stack and get the next container
        var oldContainer = pageStack.pop();
        var container = pageStack[pageStack.length - 1];
        if (page !== undefined) {
            // an unwind target has been specified - pop until we find it
            while (page != container.page && pageStack.length > 1) {
                container.cleanup();
                pageStack.pop();
                container = pageStack[pageStack.length - 1];
            }
        }

        depth = pageStack.length;
        currentPage = container.page;

        // perform page transition
        var orientationChange = orientationChanges(oldContainer.page, container.page);
        oldContainer.popExit(immediate, orientationChange);
        container.popEnter(immediate, orientationChange);

        // sync tool bar
        var tools = container.page.tools || null;
        if (toolBar) {
            toolBar.setTools(tools, immediate ? "set" : "pop");
        }
        return oldContainer.page;
    } else {
        return null;
    }
}

// Checks if the orientation changes between oldPage and newPage
function orientationChanges(oldPage, newPage) {
    return false;
}

// Clears the page stack.
function clear() {
    var container;
    while (container = pageStack.pop()) {
        container.cleanup();
    }
    depth = 0;
    currentPage = null;
}

// Iterates through all pages in the stack (top to bottom) to find a page.
function find(func) {
    for (var i = pageStack.length - 1; i >= 0; i--) {
        var page = pageStack[i].page;
        if (func(page)) {
            return page;
        }
    }
    return null;
}

