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
    propertyTitle: i18nc("@label", "Hanging Punctuation");
    propertyName: "hanging-punctuation";
    propertyType: TextPropertyConfigModel.Paragraph;
    toolTip: i18nc("@info:tooltip",
                   "Hanging punctuation allows hanging opening and closing punctuation as well as commas. This implementation only implements East-Asian style hanging punctuation.");
    searchTerms: i18nc("comma separated search terms for the hanging-punctuation property, matching is case-insensitive",
                       "hanging-punctuation, burasagari");
    property alias hangStart: paragraphStartCbx.checked;
    property alias hangEnd: paragraphEndCbx.checked;
    property int hangComma: KoSvgTextPropertiesModel.NoHang;

    Connections {
        target: properties;
        function onHangingPunctuationFirstChanged() {
            updateHangStart();
            updateVisibility();
        }
        function onHangingPunctuationLastChanged() {
            updateHangEnd();
            updateVisibility();
        }
        function onHangingPunctuationCommaChanged() {
            updateHangComma();
            updateVisibility();
        }

        function onHangingPunctuationStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateHangStart();
        updateHangEnd();
        updateHangComma();
        updateVisibility();
    }

    function updateHangStart() {
        blockSignals = true;
        hangStart = properties.hangingPunctuationFirst;
        blockSignals = false;
    }

    function updateHangEnd() {
        blockSignals = true;
        hangEnd = properties.hangingPunctuationLast;
        blockSignals = false;
    }

    function updateHangComma() {
        blockSignals = true;
        hangComma = properties.hangingPunctuationComma;
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.hangingPunctuationState];
        setVisibleFromProperty();
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

    onEnableProperty: properties.hangingPunctuationState = KoSvgTextPropertiesModel.PropertySet;

    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.hangingPunctuationState;
            onClicked: properties.hangingPunctuationState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
            font.italic: properties.hangingPunctuationState === KoSvgTextPropertiesModel.PropertyTriState;
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
            wheelEnabled: true;
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
