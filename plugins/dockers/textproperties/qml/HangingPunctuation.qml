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
    propertyName: i18nc("@label", "Hanging-punctuation");
    property alias hangStart: paragraphStartCbx.checked;
    property alias hangEnd: paragraphEndCbx.checked;
    property int hangComma: KoSvgTextPropertiesModel.NoHang;

    onPropertiesUpdated: {
        blockSignals = true;
        hangStart = properties.hangingPunctuationFirst;
        hangEnd = properties.hangingPunctuationLast;
        hangComma = properties.hangingPunctuationComma;
        visible = properties.hangingPunctuationState !== KoSvgTextPropertiesModel.PropertySet;
        blockSignals = false;
    }

    onHangStartChanged: {
        if (!blockSignals) {
            properties.hangingPunctuationFirst = hangStart;
        }
    }

    onHangEndChanged: {
        if (!blockSignals) {
            properties.hangingPunctuationLast = hangEnd;
        }
    }

    onHangCommaChanged: {
        lineEndCmb.currentIndex = lineEndCmb.indexOfValue(hangComma);
        if (!blockSignals) {
            properties.hangingPunctuationComma = hangComma
        }
    }
    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.hangingPunctuationState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.hangingPunctuationState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Hang First")
            id: paragraphStartCbx;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@label:listbox", "Line End:")
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        ComboBox {
            model: [
                {text: i18nc("@label:inlistbox", "No Hanging"), value: KoSvgTextPropertiesModel.NoHang},
                {text: i18nc("@label:inlistbox", "Allow hanging commas"), value: KoSvgTextPropertiesModel.AllowHang},
                {text: i18nc("@label:inlistbox", "Force hanging commas"), value: KoSvgTextPropertiesModel.ForceHang}
            ]
            Layout.fillWidth: true;
            id: lineEndCmb;
            textRole: "text";
            valueRole: "value";
            onActivated: hangComma = currentValue;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }

        CheckBox {
            text: i18nc("@option:check", "Hang Last")
            id: paragraphEndCbx;
        }

    }
}
