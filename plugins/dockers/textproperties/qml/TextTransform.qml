/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    property alias fullWidth: fullWidthChk.checked;
    property alias fullSizeKana: fullSizeKanaChk.checked;
    property int capitals;

    onPropertiesUpdated: {
        blockSignals = true;
        fullWidth = properties.textTransform.fullWidth;
        fullSizeKana = properties.textTransform.fullSizeKana;
        capitals = properties.textTransform.capitals;
        blockSignals = false;
    }

    onCapitalsChanged: {
        textTransformCaseCmb.currentIndex = textTransformCaseCmb.indexOfValue(capitals)
        if (!blockSignals) {
            properties.textTransform.capitals = capitals;
        }
    }

    onFullWidthChanged: {
        if (!blockSignals) {
            properties.textTransform.fullWidth = fullWidth;
        }
    }

    onFullSizeKanaChanged: {
        if (!blockSignals) {
            properties.textTransform.fullSizeKana = fullSizeKana;
        }
    }

    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.textTransformState === KoSvgTextPropertiesModel.PropertySet;
        }

        Label {
            text: i18nc("@title:group", "Text Transform:")
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        Label {
            text:  i18nc("@label:listbox", "Case:")
        }

        ComboBox {
            id: textTransformCaseCmb;
            model: [
                {text: i18nc("@label:inlistbox", "None"), value: KoSvgText.TextTransformNone},
                {text: i18nc("@label:inlistbox", "Capitalize"), value: KoSvgText.TextTransformCapitalize},
                {text: i18nc("@label:inlistbox", "Upper Case"), value: KoSvgText.TextTransformUppercase},
                {text: i18nc("@label:inlistbox", "Lower Case"), value: KoSvgText.TextTransformLowercase}
            ]
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: capitals = currentValue;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {
            id: fullWidthChk;
            text: i18nc("@option:check", "Full-width")
            Layout.fillWidth: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {
            id: fullSizeKanaChk;
            text: i18nc("@option:check", "Full-size Kana")
            Layout.fillWidth: true;
        }
    }
}
