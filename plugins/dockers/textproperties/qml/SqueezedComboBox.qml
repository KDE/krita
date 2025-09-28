/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.components 1.0 as Kis
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
        text: squeezedComboBox.displayText;

        background: Rectangle{ color:"transparent";}

        Kis.ToolTipBase {
            parent: contentItemDelegate
            visible: contentItemDelegate.hovered && contentItemDelegate.tooltipText.length > 0;
            text: contentItemDelegate.tooltipText;
        }
        onClicked: {
            squeezedComboBox.popup.visible? squeezedComboBox.popup.close(): squeezedComboBox.popup.open();
        }
    }

    // It is surprisingly hard to get the listview to be the same size as the child rect, if we also want the children
    // to grow if there's any more space. This function calculates that.


    delegate: ItemDelegate {
        id: squeezedComboDelegate;
        width: Math.max(implicitWidth, parent && parent != undefined? parent.width: 0);
        implicitWidth: leftPadding + textMetrics.advanceWidth + rightPadding + spacing + (squeezedComboBox.iconRole ? iconSize+spacing: 0);
        palette: squeezedComboBox.palette;
        highlighted: squeezedComboBox.highlightedIndex === index;
        // Depending on the model, either modelData (read only) or model(read/write) is available.
        // See https://bugreports.qt.io/browse/QTBUG-111176
        required property var model;
        property var modelData: model.modelData;
        property var mData: modelData && modelData !== undefined? modelData: model;
        required property int index;
        property string tooltipText: squeezedComboBox.toolTipRole ? mData[squeezedComboBox.toolTipRole] : "";
        icon.height: squeezedComboBox.iconSize;
        icon.width: squeezedComboBox.iconSize;
        icon.color: highlighted? palette.highlightedText: palette.text;
        icon.source: squeezedComboBox.iconRole ? mData[squeezedComboBox.iconRole] : "";
        text: squeezedComboBox.textRole ? mData[squeezedComboBox.textRole] : "";
        property alias textMetricText: textMetrics.text;
        textMetricText: text;

        background: Rectangle {
            color: squeezedComboDelegate.highlighted? palette.highlight: "transparent";
        }

        // To get the pop-up to be the same size as the maximum view, we first need to calculate
        // the implicit width via text-metrics, because icon label elides by default.
        // then, on completed, we need to set the neededwidth to the max of all completed items.
        // then, we need to set the width to the max of implicit and parent width, so the item fills.
        Component.onCompleted: view.neededWidth = Math.max(view.neededWidth, implicitWidth);

        TextMetrics {
            id: textMetrics;
            font: squeezedComboDelegate.font;
        }

        Kis.ToolTipBase {
            parent: squeezedComboDelegate;
            visible: highlighted && squeezedComboDelegate.tooltipText.length > 0;
            text: squeezedComboDelegate.tooltipText;
        }
    }

    popup.palette: squeezedComboBox.palette;
    popup.x: squeezedComboBox.width - popup.width;
    popup.width: popup.contentWidth + popup.leftPadding + popup.rightPadding;
    popup.contentItem: ListView {
        id: view;
        property int neededWidth: 100;
        implicitWidth: Math.max(neededWidth, squeezedComboBox.width);
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
