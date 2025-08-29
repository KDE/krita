/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

Control {
    id: root;

    height: implicitHeight;
    implicitHeight: layout.height + padding*2;
    clip: true;
    required property string display;
    required property string tag;
    required property string sample;
    required property string toolTip;
    property int featureValue: 0;

    onFeatureValueChanged: svgTextLabel.updateFeatureValue();

    property var fontFamilies: [];
    property double fontSize: 12.0;
    property double fontWeight: 400;
    property double fontWidth: 100;
    property int fontStyle: 0;
    property double fontSlant: 0.0;
    property var fontAxesValues: ({});
    property string language: "";

    property bool enableMouseEvents: true;

    property alias containsMouse: mouseArea.containsMouse;
    property bool highlighted: false;

    background: Rectangle {
        color: highlighted? palette.highlight: "transparent";
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
            color: root.highlighted? palette.highlightedText: palette.text;
        }

        ToolSeparator {
            Layout.fillHeight: true;
            palette.window: root.highlighted? root.palette.highlight: root.palette.window;
        }

        ColumnLayout {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            Layout.topMargin: layout.spacing;
            Layout.rightMargin: layout.spacing;
            Layout.bottomMargin: layout.spacing;
            Label {
                id: displayLabel;
                text: root.display;
                elide: Text.ElideRight;
                Layout.fillWidth: true;
                Layout.preferredHeight: implicitHeight
                color: root.highlighted? palette.highlightedText: palette.text;
            }

            Kis.SvgTextLabel {
                text: root.sample;
                id: svgTextLabel;
                implicitWidth: minimumRect.width;
                implicitHeight: minimumRect.height;
                Layout.preferredHeight: displayLabel.height * 2;
                Layout.preferredWidth: (displayLabel.height * 2) * (implicitWidth/implicitHeight);
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter;
                textColor: root.containsMouse? root.palette.highlightedText: root.palette.text;
                fillColor: "transparent";
                fontFamilies: root.fontFamilies;
                fontSize: root.fontSize;
                fontStyle: root.fontStyle;
                fontWeight: root.fontWeight;
                fontWidth: root.fontWidth;
                fontSlant: root.fontSlant;
                fontAxesValues: root.fontAxesValues;
                language: root.language;

                Component.onCompleted: {
                    updateFeatureValue();
                }

                function updateFeatureValue() {
                    var newFeatures = {};
                    newFeatures[root.tag] = root.featureValue;
                    openTypeFeatures = newFeatures;
                }
            }
        }
    }
    MouseArea {
        id: mouseArea;
        anchors.fill: parent;
        hoverEnabled: root.enableMouseEvents;
        enabled: root.enableMouseEvents;
        onClicked: (mouse)=>{ featureClicked(mouse); }
        Kis.ToolTipBase {
            parent: mouseArea;
            text: root.toolTip;
            visible: mouseArea.containsMouse;
        }
    }

}
