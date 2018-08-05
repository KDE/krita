/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
