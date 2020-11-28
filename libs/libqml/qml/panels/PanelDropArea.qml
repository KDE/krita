/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.draganddrop 1.0 as DnD

DnD.DropArea {
    id: base;

    property string state: "collapsed";

    onYChanged: {
        if (children.length > 0) {
            if (Constants.IsLandscape && y > 0) {
                children[0].roundTop = true;
            }
            else {
                children[0].roundTop = false;
            }
        }
    }

    onDragEnter: {
        event.accept(Qt.MoveAction);
    }

    onDrop: {
        var item = event.mimeData.source;
        if (children.length > 0) {
            children[0].state = "collapsed";
            children[0].parent = item.lastArea;
        }
        item.parent = base;
    }

    onChildrenChanged: {
        if (children.length > 0) {
            var item = children[0];
            item.state = base.state;
            item.lastArea = base;
            item.x = 0;
            item.y = 0;
            item.width = width;
            item.height = height;
            item.roundTop = (Constants.IsLandscape && y > 0);
        }
    }
}
