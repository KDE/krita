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
    propertyName: i18nc("@label:listbox", "Direction")
    property int direction;

    onPropertiesUpdated: {
        blockSignals = true;
        direction = properties.direction;
        visible = properties.directionState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onDirectionChanged: {
        directionCmb.currentIndex = directionCmb.indexOfValue(direction);
        if (!blockSignals) {
            properties.direction = direction;
        }
    }

    RowLayout {
        id: row
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.directionState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.directionState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
        }

        ComboBox {
            id: directionCmb
            Layout.fillWidth: true;
            model: [
                {text: i18nc("@label:inlistbox", "Left to Right"), value: KoSvgText.DirectionLeftToRight},
                {text: i18nc("@label:inlistbox", "Right to Left"), value: KoSvgText.DirectionRightToLeft}
            ]
            textRole: "text";
            valueRole: "value";
            onActivated: direction = currentValue;
        }
    }
}
