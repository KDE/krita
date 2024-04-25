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
    propertyName: i18nc("@title:group", "Tab Size");
    property alias tabSize: tabSizeSpn.value;
    property int tabSizeUnit: TabSizeModel.Spaces;

    onPropertiesUpdated: {
        blockSignals = true;
        tabSize = properties.tabSize.value;
        tabSizeUnit = properties.tabSize.unit;
        visible = properties.tabSizeState !== KoSvgTextPropertiesModel.PropertySet;
        blockSignals = false;
    }

    onTabSizeChanged: {
        if (!blockSignals) {
            properties.tabSize.value = tabSize;
        }
    }

    onTabSizeUnitChanged: {
        tabSizeUnitCmb.currentIndex = tabSizeUnitCmb.indexOfValue(tabSizeUnit);
        if (!blockSignals) {
            properties.tabSize.unit = tabSizeUnit;
        }
    }

    GridLayout {
        columns: 3
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        RevertPropertyButton {
            revertEnabled: properties.tabSizeState === KoSvgTextPropertiesModel.PropertySet;
            onClicked: properties.tabSizeState = KoSvgTextPropertiesModel.PropertyUnset;
        }
        Label {
            text: propertyName
            Layout.columnSpan: 2;
            Layout.fillWidth: true;
        }

        Item {
            width: 1;
            height: 1;
        }

        SpinBox {
            id: tabSizeSpn;
            Layout.fillWidth: true;
        }
        ComboBox {
            id: tabSizeUnitCmb
            model: [
                {text: i18nc("@label:inlistbox", "Spaces"), value: TabSizeModel.Spaces},
                {text: i18nc("@label:inlistbox", "Pt"), value: TabSizeModel.Absolute},
                {text: i18nc("@label:inlistbox", "Em"), value: TabSizeModel.Em},
                {text: i18nc("@label:inlistbox", "Ex"), value: TabSizeModel.Ex}
            ]
            textRole: "text";
            valueRole: "value";
            onActivated: tabSizeUnit = currentValue;
        }

    }
}
