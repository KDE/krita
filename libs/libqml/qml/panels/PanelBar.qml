/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.draganddrop 1.0 as DnD
import org.krita.sketch 1.0

Item {
    id: base;

    property real panelHeight;

    function collapse() {
        if (d.peeking) {
            d.peeking.state = "collapsed";
            d.peeking.z = 0;
            d.peeking = null;
        }
    }

    PresetsPanel {
        id: presetsPanel;
        objectName: "presets";
        width: (parent === leftArea || parent === rightArea) ? parent.width : Constants.GridWidth * 2;
        height: base.panelHeight;
        page: base.parent;
        onPeek: beginPeek( presetsPanel );
        onCollapsed: endPeek( presetsPanel );
        onFull: endPeek( presetsPanel );
        onDragStarted: beginDrag( presetsPanel );
        onDrop: endDrag( presetsPanel, action );
    }
    LayersPanel {
        id: layersPanel;
        objectName: "layers";
        width: (parent === leftArea || parent === rightArea) ? parent.width : Constants.GridWidth * 2;
        height: base.panelHeight;
        page: base.parent;
        onPeek: beginPeek( layersPanel );
        onCollapsed: endPeek( layersPanel );
        onFull: endPeek( layersPanel );
        onDragStarted: beginDrag( layersPanel );
        onDrop: endDrag( layersPanel, action );
    }
    FilterPanel {
        id: filterPanel;
        objectName: "filter";
        width: (parent === leftArea || parent === rightArea) ? parent.width : Constants.GridWidth * 2;
        height: base.panelHeight;
        page: base.parent;
        onPeek: beginPeek( filterPanel );
        onCollapsed: endPeek( filterPanel );
        onFull: endPeek( filterPanel );
        onDragStarted: beginDrag( filterPanel );
        onDrop: endDrag( filterPanel, action );
    }
    SelectPanel {
        id: selectPanel;
        objectName: "select";
        width: (parent === leftArea || parent === rightArea) ? parent.width : Constants.GridWidth * 2;
        height: base.panelHeight;
        page: base.parent;
        onPeek: beginPeek( selectPanel );
        onCollapsed: endPeek( selectPanel );
        onFull: endPeek( selectPanel );
        onDragStarted: beginDrag( selectPanel );
        onDrop: endDrag( selectPanel, action );
    }
    ToolPanel {
        id: toolPanel;
        objectName: "tool";
        width: (parent === leftArea || parent === rightArea) ? parent.width : Constants.GridWidth * 2;
        height: base.panelHeight;
        page: base.parent;
        onPeek: beginPeek( toolPanel );
        onCollapsed: endPeek( toolPanel );
        onFull: endPeek( toolPanel );
        onDragStarted: beginDrag( toolPanel );
        onDrop: endDrag( toolPanel, action );
    }
    ColorPanel {
        id: colorPanel;
        objectName: "color";
        width: (parent === leftArea || parent === rightArea) ? parent.width : Constants.GridWidth * 2;
        height: base.panelHeight;
        page: base.parent;
        onPeek: beginPeek( colorPanel );
        onCollapsed: endPeek( colorPanel );
        onFull: endPeek( colorPanel );
        onDragStarted: beginDrag( colorPanel );
        onDrop: endDrag( colorPanel, action );
    }

    Component.onCompleted: {
        panelConfiguration.restore();
    }

    PanelDropArea {
        id: leftArea;

        objectName: "leftFull";

        anchors.top: parent.top;
//         anchors.bottom: parent.bottom;
        anchors.left: parent.left;
        height: base.panelHeight;

        onHeightChanged: if (children.length > 0) children[0].height = height;
        width: Constants.GridWidth * 2;
        state: "full";
    }

    PanelDropArea {
        id: rightArea;

        objectName: "rightFull";

        anchors.top: parent.top;
//         anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        height: base.panelHeight;

        onHeightChanged: if (children.length > 0) children[0].height = height;
        width: Constants.GridWidth * 2;
        state: "full";
    }

//     Row {
//         id: panelTopRow;
//         spacing: Constants.IsLandscape ? -Constants.GridWidth * 2 : 0;

        PanelDropArea {
            id: centerTopArea1;
            objectName: "centerTop1"

            x: Constants.GridWidth * 2 + 4;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea1.x; y: centerTopArea1.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }

        PanelDropArea {
            id: centerTopArea2;
            objectName: "centerTop2"

            anchors.left: centerTopArea1.right;
            y: Constants.IsLandscape ? Constants.GridHeight / 2 : 0;
            anchors.leftMargin: Constants.IsLandscape ? -Constants.GridWidth / 2 : 0;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea2.x; y: centerTopArea2.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }

        PanelDropArea {
            id: centerTopArea3;
            objectName: "centerTop3"

            anchors.left: centerTopArea2.right;
            anchors.leftMargin: Constants.IsLandscape ? -Constants.GridWidth / 2 : 0;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea3.x; y: centerTopArea3.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }

        PanelDropArea {
            id: centerTopArea4;
            objectName: "centerTop4"

            anchors.left: centerTopArea3.right;
            y: Constants.IsLandscape ? Constants.GridHeight / 2 : 0;
            anchors.leftMargin: Constants.IsLandscape ? -Constants.GridWidth / 2 : 0;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea4.x; y: centerTopArea4.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }

        PanelDropArea {
            id: centerTopArea5;
            objectName: "centerTop5"

            anchors.left: centerTopArea4.right;
            anchors.leftMargin: Constants.IsLandscape ? -Constants.GridWidth / 2 : 0;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea5.x; y: centerTopArea5.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }

        PanelDropArea {
            id: centerTopArea6;
            objectName: "centerTop6"

            anchors.left: centerTopArea5.right;
            y: Constants.IsLandscape ? Constants.GridHeight / 2 : 0;
            anchors.leftMargin: Constants.IsLandscape ? -Constants.GridWidth / 2 : 0;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea6.x; y: centerTopArea6.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }

        PanelDropArea {
            id: centerTopArea7;
            objectName: "centerTop7"

            anchors.left: centerTopArea6.right;
            anchors.leftMargin: Constants.IsLandscape ? -Constants.GridWidth / 2: 0;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea7.x; y: centerTopArea7.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }

        PanelDropArea {
            id: centerTopArea8;
            objectName: "centerTop8"

            anchors.left: centerTopArea7.right;
            y: Constants.IsLandscape ? Constants.GridHeight / 2 : 0;
            anchors.leftMargin: Constants.IsLandscape ? -Constants.GridWidth / 2 : 0;
            width: Constants.GridWidth;
            height: Constants.GridHeight / 2;
        }
        Rectangle {
            opacity: dropOverlay.opacity; Behavior on opacity { NumberAnimation { } }
            x: centerTopArea8.x; y: centerTopArea8.y
            width: Constants.GridWidth; height: Constants.GridHeight / 2;
            color: Settings.theme.color("panels/dropArea/fill");
            border.color: Settings.theme.color("panels/dropArea/border"); border.width: 2;
        }
    //}

    Item {
        id: dropOverlay;
        anchors.fill: parent;

        opacity: 0;
        Behavior on opacity { NumberAnimation { } }

        Rectangle {
            id: leftOverlay;
            x: leftArea.x;
            width: leftArea.width;
            height: leftArea.height

            color: Settings.theme.color("panels/dropArea/fill");

            border.color: Settings.theme.color("panels/dropArea/border");
            border.width: 2;
        }

//         Row {
//             x: panelTopRow.x;
//             y: panelTopRow.y;
//             width: panelTopRow.width;
//             Repeater {
//                 model: 8;
//
//                 delegate:
//             }
//         }

        Rectangle {
            id: rightOverlay;
            x: rightArea.x
            y: rightArea.y
            width: rightArea.width;
            height: rightArea.height;

            color: Settings.theme.color("panels/dropArea/fill");

            border.color: Settings.theme.color("panels/dropArea/border");
            border.width: 2;
        }
    }

    QtObject {
        id: d;

        property variant panels: [ presetsPanel, layersPanel, filterPanel, selectPanel, toolPanel, colorPanel ];
        property variant panelAreas: [
            centerTopArea1,
            centerTopArea2,
            centerTopArea3,
            centerTopArea4,
            centerTopArea5,
            centerTopArea6,
            centerTopArea7,
            centerTopArea8,
            leftArea,
            rightArea ];
        property Item peeking: null;
        property Item dragParent: null;
    }

    PanelConfiguration {
        id: panelConfiguration;

        panels: d.panels;
        panelAreas: d.panelAreas;
    }

    function beginPeek( item ) {
        for( var i in d.panels ) {
            var obj = d.panels[i];
            if ( obj.state == "peek" && obj.objectName != item.objectName ) {
                obj.state = "collapsed";
            }
        }

        d.peeking = item;
        item.parent.z = 11;
    }

    function endPeek( item ) {
        if (d.peeking == item)
        {
            d.peeking = null;
        }
        item.parent.z = 0;
    }

    function beginDrag( item ) {
        dropOverlay.opacity = 1;
        item.parent.z = 0;
        d.dragParent = item.parent;
        item.parent = null;
        item.opacity = 0;
    }

    function endDrag( item, action ) {
        dropOverlay.opacity = 0;
        item.opacity = 1;
        if (action == Qt.IgnoreAction) {
            item.parent = d.dragParent;
        }
    }

    states: [
        State {
            name: "portrait";
            when: base.panelHeight > base.width;
            PropertyChanges { target: centerTopArea1; x: Constants.GridWidth * 4 + 4; }
            PropertyChanges { target: leftArea; width: Constants.GridWidth * 4; }
            PropertyChanges { target: rightArea; width: Constants.GridWidth * 4; }
            PropertyChanges { target: leftArea; height: base.panelHeight / 2; }
            AnchorChanges { target: rightArea; anchors.right: undefined; anchors.left: parent.left; anchors.top: leftArea.bottom; }
            PropertyChanges { target: rightArea; height: base.panelHeight / 2; }
        }
    ]
}
