/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
/**
  This is a QML Interpretation of KisSqueezedComboBox, where the main combobox button is resized and elided,
  but the pop-up is scaled so all items fit inside.

  It can also handle an icon and tooltip.
  */
ComboBox {
    id: squeezedComboBox;
    property string iconRole: ""; ///< model roleName string to icon source.
    property string toolTipRole: ""; ///< model roleName to tooltip.
    property int iconSize: 12;
    wheelEnabled: true;

    height: contentItem.height;

    contentItem: ItemDelegate {
        id: contentItemDelegate;
        width: parent.width;
        palette: squeezedComboBox.palette;
        property var mData: squeezedComboBox.model !== undefined? squeezedComboBox.model[squeezedComboBox.currentIndex]: {};
        property string tooltipText: squeezedComboBox.toolTipRole && mData !== undefined ? mData[squeezedComboBox.toolTipRole] : "";
        icon.height: squeezedComboBox.iconSize;
        icon.width: squeezedComboBox.iconSize;
        icon.color: highlighted? palette.highlightedText: palette.text;
        icon.source: squeezedComboBox.iconRole && mData !== undefined ? mData[squeezedComboBox.iconRole] : "";
        text: squeezedComboBox.textRole && mData !== undefined? mData[squeezedComboBox.textRole] : "";

        background: Rectangle{ color:"transparent";}

        ToolTip.text: tooltipText;
        ToolTip.visible: highlighted && tooltipText.length > 0;
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
        onClicked: {
            squeezedComboBox.popup.visible? squeezedComboBox.popup.close(): squeezedComboBox.popup.open();
        }
    }

    TextMetrics {
        id: textMetrics;
        font: contentItemDelegate.font;
    }

    // It is surprisingly hard to get the listview to be the same size as the child rect, if we also want the children
    // to grow if there's any more space. This function calculates that.
    function recalculatePopupWidth() {
        if (model === undefined) return;
        let maxValue = 0;
        for (let i = 0; i < count; i++) {
            textMetrics.text = squeezedComboBox.textRole ? model[i][squeezedComboBox.textRole] : "";
            maxValue = Math.max(maxValue, textMetrics.width);
        }
        let iconWidth = iconRole? iconSize: 0;
        let total = iconWidth + maxValue + contentItemDelegate.leftPadding+contentItemDelegate.rightPadding+(contentItemDelegate.spacing*2);
        view.maxWidth = total;
    }

    Component.onCompleted: recalculatePopupWidth();
    onModelChanged: recalculatePopupWidth();

    delegate: ItemDelegate {
        id: squeezedComboDelegate;
        width: parent && parent != undefined? parent.width: 0;
        palette: squeezedComboBox.palette;
        highlighted: squeezedComboBox.highlightedIndex === index;
        // Depending on the model, either modelData (read only) or model(read/write) is available.
        // See https://bugreports.qt.io/browse/QTBUG-111176
        required property var modelData;
        required property var model;
        property var mData: modelData && modelData !== undefined? modelData: model;
        required property int index;
        property string tooltipText: squeezedComboBox.toolTipRole ? mData[squeezedComboBox.toolTipRole] : "";
        icon.height: squeezedComboBox.iconSize;
        icon.width: squeezedComboBox.iconSize;
        icon.color: highlighted? palette.highlightedText: palette.text;
        icon.source: squeezedComboBox.iconRole ? mData[squeezedComboBox.iconRole] : "";
        text: squeezedComboBox.textRole ? mData[squeezedComboBox.textRole] : "";

        ToolTip.text: tooltipText;
        ToolTip.visible: highlighted && tooltipText.length > 0;
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
    }

    popup: Popup {
        id: squeezedComboPopup;
        y: squeezedComboBox.height - 1;
        x: squeezedComboBox.width - width;
        width: contentWidth;
        height: contentHeight;
        padding: 1;

        palette: squeezedComboBox.palette;

        contentItem: ListView {
            leftMargin: 0;
            rightMargin: 0;
            id: view;
            property int maxWidth: 100;
            implicitWidth: Math.max(maxWidth, squeezedComboBox.width);
            width: implicitWidth;
            implicitHeight: contentHeight;
            height: implicitHeight;
            clip: true;
            model: squeezedComboBox.delegateModel;
            currentIndex: squeezedComboBox.currentIndex;
            ScrollBar.vertical: ScrollBar {
            }
        }
    }
}
