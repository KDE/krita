/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.flake.text 1.0

Column {
    id: root;
    property int firstColumnWidth: 32;
    property int columnSpacing: 5;
    padding: columnSpacing;
    width: parent? parent.width - (padding*2): 100;
    height: visible? childrenRect.height: 0;

    enum PropertyType {
        Character, ///< This property can be applied on a character level.
        Paragraph, ///< This property only does something when applied to a paragraph.
        Mixed ///< This property can be in either.
    }


    property KoSvgTextPropertiesModel properties : textPropertiesModel;
    signal propertiesUpdated; ///< Used by each text property panel to update the data on the controls.
    signal enableProperty; ///< Set the property to a default value.
    property bool blockSignals; ///< When setting the data on the controls, this needs to be enabled and checked while returning data from the controls.

    property string propertyName; ///< Translated name of the property.
    property int propertyType; ///< PropertyType of the current property.
    property int parentPropertyType; ///< PropertyType of the current property list it is in.
    property string toolTip; ///< Tooltip associated with the property.
    property string searchTerms; ///< Any additional search terms associated with the property.

    function autoEnable() {
        enabled = true;
    }
    Component.onCompleted: {
        children.push(seperator);
    }

    MenuSeparator {
        id: seperator;
        width: parent.width;
        topPadding: parent.padding;
        bottomPadding:parent.padding;
    }
}
