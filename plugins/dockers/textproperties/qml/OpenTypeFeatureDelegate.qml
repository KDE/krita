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

    height: visible? implicitHeight: 0;
    implicitHeight: layout.height + padding*2;
    property string display;
    property string tag;
    property string sample;
    property string toolTip;

    property bool enableMouseEvents: true;

    property alias containsMouse: mouseArea.containsMouse;

    background: Rectangle {
        color: containsMouse? sysPalette.highlight: "transparent";
    }

    signal featureClicked (QtObject mouse);

    RowLayout {
        id: layout;
        width: parent.availableWidth;

        Label {
            Layout.leftMargin: layout.spacing;
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            textFormat: Text.MarkdownText;
            // This is somehow the easiest way I've found it to require monospace...
            text: "`"+root.tag+"`";
            Layout.preferredHeight: implicitHeight;
            Layout.maximumWidth: contentWidth;
            color: root.containsMouse? palette.highlightedText: palette.text;
        }

        ToolSeparator {
            Layout.fillHeight: true;
        }

        ColumnLayout {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            Layout.topMargin: layout.spacing;
            Layout.rightMargin: layout.spacing;
            Layout.bottomMargin: layout.spacing;
            Label {
                text: root.display;
                elide: Text.ElideRight;
                Layout.fillWidth: true;
                Layout.preferredHeight: implicitHeight
                color: root.containsMouse? palette.highlightedText: palette.text;
            }

            Label {
                text: root.sample;
                elide: Text.ElideRight;
                Layout.fillWidth: true;
                Layout.preferredHeight: implicitHeight
                color: root.containsMouse? palette.highlightedText: palette.text;
            }
        }
    }
    MouseArea {
        id: mouseArea;
        anchors.fill: parent;
        hoverEnabled: root.enableMouseEvents;
        enabled: root.enableMouseEvents;
        onClicked: (mouse)=>{ featureClicked(mouse); }

        ToolTip.text: root.toolTip;
        ToolTip.visible: containsMouse;
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
    }
}
