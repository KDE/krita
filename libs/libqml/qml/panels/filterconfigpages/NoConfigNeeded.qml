/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Column {
    Item {
        width: parent.width;
        height: Constants.GridHeight;
    }
    Label {
        width: parent.width;
        wrapMode: Text.WordWrap;
        horizontalAlignment: Text.AlignHCenter;
        elide: Text.ElideNone;
        text: "This filter requires no configuration. Click below to apply it.";
    }
    Item {
        width: parent.width;
        height: Constants.GridHeight / 2;
    }
    Button {
        width: height;
        height: Constants.GridHeight
        anchors.horizontalCenter: parent.horizontalCenter;
        image: Settings.theme.icon("apply");
        onClicked: fullFilters.model.activateFilter(fullFilters.currentIndex);
    }
}
