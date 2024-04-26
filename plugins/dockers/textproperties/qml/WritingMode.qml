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
    propertyName: i18nc("@label:listbox", "Writing Mode");
    property int writingMode;

    onPropertiesUpdated: {
        blockSignals = true;
        writingMode = properties.writingMode;
        visible = properties.writingModeState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onWritingModeChanged: {
        writingModeCmb.currentIndex = writingModeCmb.indexOfValue(writingMode)
        if (!blockSignals) {
            properties.writingMode = writingMode;
        }
    }

    RowLayout {
        spacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertEnabled: properties.writingModeState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.writingModeState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyName;
            elide: Text.ElideRight;
            Layout.fillWidth: true;
        }


        ComboBox {
            id: writingModeCmb
            model: [
                {text: i18nc("@label:inlistbox", "Horizontal"), value: KoSvgText.HorizontalTB},
                {text: i18nc("@label:inlistbox", "Vertical, Right to Left"), value: KoSvgText.VerticalRL},
                {text: i18nc("@label:inlistbox", "Vertical, Left To Right"), value: KoSvgText.VerticalLR}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: writingMode = currentValue;
        }
    }
}
