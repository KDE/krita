/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

Column {
    id: root;
    property int firstColumnWidth: 32;
    property int columnSpacing: 5;
    padding: columnSpacing;
    width: ListView.view? ListView.view.width - (padding * 2): 100;
    height: visible? implicitHeight: 0;

    property KoSvgTextPropertiesModel properties : textPropertiesModel;
    property int defaultVisibilityState : TextPropertyConfigModel.FollowDefault;
    property double dpi: 72.0;
    signal propertiesUpdated; ///< Used by each text property panel to update the data on the controls.
    signal enableProperty; ///< Set the property to a default value.
    property bool blockSignals; ///< When setting the data on the controls, this needs to be enabled and checked while returning data from the controls.

    property string propertyName; ///< name of the property used for config purposes. Needs to be unique!
    property string propertyTitle; ///< Translated name of the property.
    property int propertyType; ///< PropertyType of the current property.
    property int parentPropertyType; ///< PropertyType of the current property list it is in.
    property string toolTip; ///< Tooltip associated with the property.
    property string searchTerms; ///< Any additional search terms associated with the property.

    property int visibilityState: TextPropertyConfigModel.FollowDefault;
    property var propertyState: []; /// All property states related to the current property.

    function setVisibleFromProperty() {
        let visibleState = visibilityState === TextPropertyConfigModel.FollowDefault? defaultVisibilityState: visibilityState;
        if (visibleState === TextPropertyConfigModel.AlwaysVisible) {
            visible = true;
        } else if (visibleState === TextPropertyConfigModel.NeverVisible) {
            visible = false;
        } else {
            visible = false;
            for (let propState of propertyState) {
                if (propState === KoSvgTextPropertiesModel.PropertySet
                        || propState === KoSvgTextPropertiesModel.PropertyTriState) {
                    visible = true;
                    break;
                } else if (propState !== KoSvgTextPropertiesModel.PropertyUnset
                           && visibleState === TextPropertyConfigModel.WhenRelevant) {
                    visible = true;
                    break;
                }
            }
        }
    }

    function autoEnable() {
        enabled = true;
    }
    Component.onCompleted: {
        children.push(seperator);
    }

    property Kis.ThemedControl propertyBasePalette: Kis.ThemedControl {
    }

    MenuSeparator {
        id: seperator;
        width: parent && typeof parent != 'undefined'? parent.width: 100;
        topPadding: parent.padding;
    }
}
