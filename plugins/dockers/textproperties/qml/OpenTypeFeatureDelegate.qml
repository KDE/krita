/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.flake.text 1.0

Control {
    id: root;

    height: childrenRect.height;
    property string display;
    property string tag;
    property string sample;
    property string toolTip;

    property alias containsMouse: mouseArea.containsMouse;

    background: Rectangle {
        color: containsMouse? sysPalette.highlight: "transparent";
    }

    signal featureClicked (QtObject mouse);

    RowLayout {
        height: childrenRect.height;
        width: parent.width;

        Label {
            Layout.alignment: Qt.AlignCenter | Qt.AlignVCenter
            textFormat: Text.MarkdownText;
            // This is somehow the easiest way I've found it to require monospace...
            text: "`"+root.tag+"`";
            Layout.fillHeight: true;
            Layout.maximumWidth: contentWidth;
        }
        ToolSeparator {
            Layout.fillHeight: true;
        }

        ColumnLayout {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            Label {
                text: root.display;
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                elide: Text.ElideRight;
                Layout.fillWidth: true;
            }

            Label {
                text: root.sample;
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                elide: Text.ElideRight;
                Layout.fillWidth: true;
            }
        }
    }
    MouseArea {
        id: mouseArea;
        anchors.fill: parent;
        hoverEnabled: true;
        onClicked: (mouse)=>{
            featureClicked(mouse);
        }

        ToolTip.text: root.toolTip;
        ToolTip.visible: containsMouse;
        ToolTip.delay: 1000;
    }
}
