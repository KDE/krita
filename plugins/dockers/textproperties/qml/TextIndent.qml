/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

CollapsibleGroupProperty {
    propertyName: i18nc("@title:group", "Text Indent");
    propertyType: TextPropertyBase.Paragraph;

    property alias textIndentValue: textIndentSpn.value;
    property alias hanging: indentHangingCkb.checked;
    property alias eachLine: eachLineCkb.checked;

    onPropertiesUpdated: {
        blockSignals = true;
        textIndentValue = properties.textIndent.length.value;
        hanging = properties.textIndent.hanging;
        eachLine = properties.textIndent.eachLine;
        visible = properties.textIndentState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onTextIndentValueChanged: {
        if (!blockSignals) {
            properties.textIndent.length.value = textIndentValue;
        }
    }

    onHangingChanged: {
        if (!blockSignals) {
            properties.textIndent.hanging = hanging;
        }
    }

    onEachLineChanged: {
        if (!blockSignals) {
            properties.textIndent.eachLine = eachLine;
        }
    }

    titleItem: RowLayout {
        spacing: columnSpacing;
        Layout.fillWidth: true;
        SpinBox {
            id: textIndentSpn;
            Layout.fillWidth: true;
        }
        ComboBox {
            model: ["Pt", "Em", "Ex"]
            Layout.fillWidth: true;
        }
    }

    contentItem: GridLayout {
        columns: 2
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        RevertPropertyButton {
            revertEnabled: properties.textIndentState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.textIndentState = KoSvgTextPropertiesModel.PropertyUnset;
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


