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
    propertyName: i18nc("@label:spinbox", "Letter Spacing");
    property alias letterSpacing: letterSpacingSpn.value;

    onPropertiesUpdated: {
        blockSignals = true;
        letterSpacing = properties.letterSpacing.value;
        visible = properties.letterSpacingState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }
    onLetterSpacingChanged: {
        if (!blockSignals) {
            properties.letterSpacing.value = letterSpacing;
        }
    }

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.letterSpacingState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.letterSpacingState = KoSvgTextPropertiesModel.PropertyUnset;
        }

            Label {
                text: propertyName;
                elide: Text.ElideRight;
                Layout.fillWidth: true;
            }

            SpinBox {
                id: letterSpacingSpn
                editable: true;
                Layout.fillWidth: true;
            }

            ComboBox {
                model: ["Pt", "Em", "Ex"];
                Layout.fillWidth: true;
            }
    }
}
