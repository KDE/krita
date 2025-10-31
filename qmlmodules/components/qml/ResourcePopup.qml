/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

/**
  \qmltype ResourcePopup
  This is a button with an editable text that has a resource view attached inside
  a popup. Users can type in text and have the popup show the closest names.
  */
Button {
    id: resourceCmb;
    wheelEnabled: true;

    //--- Model setup. ---//

    /*
        \qmlProperty modelWrapper
        The resource model wrapper of the resource view. Can be replaced
        to have two resource views have the same model.
     */
    property alias modelWrapper : resourceView.modelWrapper;

    /*
        \qmlProperty useFileName
        whether to search against filename or regular name.
     */
    property bool useFileName : true;

    /*
        \qmlProperty view
        The resource view inside the popup.
     */
    property alias view: resourceView;
    /*
        \qmlProperty resourceType
        A string corresponding to KisResourceTypes.h ResourceType.
     */
    property alias resourceType: resourceView.resourceType;
    /*
        \qmlProperty resourceDelegate
        The delegate used for the list inside the resource view.
     */
    property alias resourceDelegate: resourceView.resourceDelegate;
    /*
        \qmlProperty resourceDelegate
        Whether the add-resource row is visible.
        This contains the import and delete buttons.
     */
    property alias addResourceRowVisible: resourceView.addResourceRowVisible;

    /*
        \qmlProperty placeholderText
        The placeholder text when the text input is empty.
     */
    property alias placeholderText: textInput.placeholderText;

    Connections {
        target: modelWrapper;
        function onResourceFilenameChanged() {
            resourceCmb.resourceName = useFileName? modelWrapper.resourceFilename: modelWrapper.resourceName;
        }
        function onResourceNameChanged() {
            resourceCmb.resourceName = useFileName? modelWrapper.resourceFilename: modelWrapper.resourceName;
        }
    }

    property var locales: [];
    property alias resourceName: textInput.text;
    topPadding: 0;
    bottomPadding: 0;
    spacing: padding;
    leftPadding:  (resourceCmb.mirrored ? padding + indicator.width + spacing : 0);
    rightPadding: (!resourceCmb.mirrored ? padding + indicator.width + spacing : 0);

    Kis.ThemedControl {
        id: palControl;
    }
    palette: palControl.palette;

    /// Ideally we'd have the max popup height be Window-height - y-pos-of-item-to-window.
    /// But that's pretty hard to calculate (map to global is screen relative, but there's
    /// no way to get window.y relative to screen), so we'll just use 300 as the maximum.
    property int maxPopupHeight: 500;

    contentItem: Kis.InformingTextInput {
        id: textInput;

        warnColor: palControl.theme.window.neutralBackgroundColor;
        warnColorBorder: palControl.theme.window.neutralTextColor;
        readOnly: !resourceCmb.enabled;

        onEditingFinished: {
            if (completerPopup.visible) {
                completerView.applyHighlightedIndex();
            }

            testInput();
            resourceCmb.activated();
        }
        onTextChanged: {
            testInput();
        }

        onTextEdited: {
            stopWarning();
            modelWrapper.searchText = text;
            completerView.highlightedIndex = 0;
            completerPopup.open();
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                selectAll();
            }
        }

        warningTimeOut: activeFocus? 1000: 0;

        function testInput() {
            let test = resourceCmb.useFileName? modelWrapper.testFileName(text): modelWrapper.testName(text);

            if (typeof test === 'undefined') {
                if (!activeFocus) {
                    startWarning();
                }
            } else {
                stopWarning();
                text = test;
                if (resourceCmb.useFileName) {
                    modelWrapper.resourceFilename = text;
                } else {
                    modelWrapper.resourceName = text;
                }
            }
        }

        Keys.onDownPressed: {
            if (completerPopup.opened) {
                completerView.downPress();
            } else {
                resourceView.downPress();
            }
        }
        Keys.onUpPressed: {
            if (completerPopup.opened) {
                completerView.upPress();
            } else {
                resourceView.upPress();
            }
        }
    }

    indicator: Image {
        id: imgIndicator
        x: Math.round(resourceCmb.mirrored ? resourceCmb.padding : resourceCmb.width - width - resourceCmb.padding);
        y: Math.round(resourceCmb.topPadding + (resourceCmb.availableHeight - height) / 2);
        source: resourceCmb.palette.button.hslLightness < 0.5? "qrc:///light_groupOpened.svg": "qrc:///dark_groupOpened.svg";
        // Setting the source size is required for proper high-DPI scaling.
        // Qt automatically multiplies the size with the display scaling.
        sourceSize.width: 12;
        sourceSize.height: 12;
        width: 12;
        height: 12;
        clip: true;
    }

    onClicked: {
        if (resourceCmbPopup.opened) {
            resourceCmbPopup.close();
        } else {
            resourceCmbPopup.open();
        }
    }
    signal activated();
    onActivated: {
        resourceCmbPopup.close();
        completerPopup.close();
        modelWrapper.searchText = "";
    }

    //--- Pop up setup ---//
    Kis.PopupBase {
        id: resourceCmbPopup;
        y: resourceCmb.height - 1;
        x: resourceCmb.width - width;
        width: Math.max(implicitWidth, Math.max(resourceCmb.width, 200));
        height: (resourceCmb.maxPopupHeight - topMargin - bottomMargin);
        padding: 2;

        palette: resourceCmb.palette;
        parent: resourceCmb;

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent;

        contentItem: Kis.ResourceView {
            id: resourceView;
            palette: resourceCmbPopup.palette;
        }
    }

    Popup {
        id: completerPopup;
        y: resourceCmb.height - 1;
        x: resourceCmb.width - width;
        width: Math.max(implicitWidth, Math.max(resourceCmb.width, 200));
        property int windowMaximum: Window.height > 0? Math.min(Window.height, resourceCmb.maxPopupHeight) - resourceCmb.height*3: resourceCmb.maxPopupHeight;
        height: Math.min(contentItem.implicitHeight, windowMaximum - (topMargin + bottomMargin));
        padding: 2;

        palette: resourceCmb.palette;
        parent: resourceCmb;

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent;
        contentItem: Kis.ResourceView {
            id: completerView;
            palette: resourceCmbPopup.palette;
            modelWrapper: resourceCmb.modelWrapper;
            resourceDelegate: resourceCmb.resourceDelegate;
            showTagging: false;
            showSearch: false;
            addResourceRowVisible: resourceCmb.addResourceRowVisible;
        }
    }
}
