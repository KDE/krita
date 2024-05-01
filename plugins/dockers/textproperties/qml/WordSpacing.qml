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
    propertyName: i18nc("@title:group", "Word Spacing");
    propertyType: TextPropertyBase.Character;
    property alias wordSpacing: wordSpacingSpn.value;
    visible: properties.wordSpacingState !== KoSvgTextPropertiesModel.PropertyUnset;

    onPropertiesUpdated: {
        blockSignals = true;
        wordSpacing = properties.wordSpacing.value;
        visible = properties.wordSpacingState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onWordSpacingChanged: {
        if (!blockSignals) {
            properties.wordSpacing.value = wordSpacing;
        }
    }

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.wordSpacingState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.wordSpacingState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
        }

        SpinBox {
            id: wordSpacingSpn
            editable: true;
            Layout.fillWidth: true;
        }

        ComboBox {
            model: ["Pt", "Em", "Ex"];
            Layout.fillWidth: true;
        }
    }
}
