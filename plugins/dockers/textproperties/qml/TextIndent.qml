/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

CollapsibleGroupProperty {
    propertyName: i18nc("@title:group", "Text Indent:");

    property alias textIndentValue: textIndentSpn.value;
    property alias hanging: indentHangingCkb.checked;
    property alias eachLine: eachLineCkb.checked;

    titleItem: RowLayout {
        spacing: columnSpacing;
        Layout.fillWidth: true;
        SpinBox {
            id: textIndentSpn;
            Layout.fillWidth: true;
        }
        ComboBox {
            model: ["Pt", "Em", "Ex"]
        }
    }

    contentItem: GridLayout {
        columns: 2
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }


        CheckBox {
            id: indentHangingCkb;
            text: i18nc("@option:check", "Hanging indentation")
            Layout.fillWidth: true
        }

        Item {
            width: firstColumnWidth;
            height: firstColumnWidth;
            Layout.columnSpan: 1;
        }

        CheckBox {
            id: eachLineCkb;
            text: i18nc("@option:check", "Indent after hardbreaks")
            Layout.fillWidth: true
        }

    }
}


