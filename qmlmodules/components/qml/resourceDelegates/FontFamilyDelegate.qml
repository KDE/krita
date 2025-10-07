/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.components 1.0 as Kis

/**
  Item Delegate to handle FontFamilies;
  */
ResourceDelegateBase {
    /**
      Ideally we'd just use required properties for the metadata etc.
      However, when changing the modelwrapper in the font list,
      there's a short moment where the model data is unavailable, so
      instead we ensure we're not accessing undefined objects.

      Not great, would like a better solution...
      */
    id: fontDelegateItem;
    property var locales: [];
    property string name: typeof model.name !== "undefined"? model.name: "";
    property int index: model.index;
    property string fontName: name;
    property var metadata: model.metadata;
    // TODO: change this to use the text locale, if possible.
    property string sample: "";

    onMetadataChanged: {
        updateSample();
    }

    Component.onCompleted: {
        updateSample();
    }

    signal activated();

    Kis.FontFunctions {
        id: fontFunctions;
    }

    function updateSample() {
        if (typeof metadata === "undefined") return;
        let samples = fontFunctions.getMapFromQVariant(metadata.sample_svg);
        let sampleKeys = Object.keys(samples);
        sample = "";
        const latn = samples["s_Latn"];
        if (sampleKeys.length > 0) {
            if (latn) {
                sample = latn;
            } else {
                let tag = sampleKeys[0];
                sample = samples[tag];
            }
        }
        if (fontDelegateItem.locales.length > 0) {
            for (const locale of fontDelegateItem.locales) {
                let tag = fontFunctions.sampleTagForQLocale(Qt.locale(locale));
                let svg = samples[tag];
                if (svg) {
                    sample = svg;
                    break;
                }


            }
        }
    }

    width: ListView.view.width;

    contentItem: Control {
        id: delegateControl;

        property alias highlighted: fontDelegateItem.highlighted;
        palette: fontDelegateItem.palette;

        property alias fontName: fontDelegateItem.fontName;

        implicitHeight: nameLabel.height*4;

        Loader {
            anchors.fill: parent;
            asynchronous: true;

            sourceComponent: Component {
                Kis.KoShapeQtQuickLabel {
                    id: fontFamilyDelegate;

                    Binding {
                           target: fontFamilyDelegate
                           property: "meta"
                           when: fontDelegateItem && typeof fontDelegateItem !== 'undefined' && typeof fontDelegateItem.metadata !== 'undefined';
                           value: fontDelegateItem.metadata;
                    }

                    property var meta: ({});
                    property bool colorBitmap : typeof meta.color_bitmap === 'boolean'? meta.color_bitmap: false;
                    property bool colorCLRV0 : typeof meta.color_clrv0 === 'boolean'?  meta.color_clrv0: false;
                    property bool colorCLRV1 : typeof meta.color_clrv1 === 'boolean'?  meta.color_clrv1: false;
                    property bool colorSVG : typeof meta.color_svg === 'boolean'?  meta.color_svg: false;
                    property bool isVariable : typeof meta.is_variable === 'boolean'?  meta.is_variable: false;
                    property int type : typeof meta.font_type === 'number'? meta.font_type: 0;

                    scalingType: Kis.KoShapeQtQuickLabel.FitHeight;
                    svgData: fontDelegateItem.sample;
                    foregroundColor: delegateControl.highlighted? palette.highlightedText: palette.text;
                    fillColor: delegateControl.highlighted? fontDelegateItem.palette.highlight: fontDelegateItem.palette.window;
                    fullColor: colorBitmap || colorCLRV0 || colorCLRV1 || colorSVG;
                    padding: nameLabel.height;


                    Row {
                        id: featureRow;
                        spacing: fontDelegateItem.spacing;
                        anchors.bottom: parent.bottom;
                        anchors.left: parent.left;
                        property int imgHeight: 12;
                        ToolButton {
                            enabled: false;
                            padding:0;
                            width: parent.imgHeight;
                            height: parent.imgHeight;
                            property int type: fontFamilyDelegate.type;
                            icon.width: parent.imgHeight;
                            icon.height: parent.imgHeight;
                            palette: fontDelegateItem.palette;
                            icon.color: nameLabel.color;
                            icon.source: fontFunctions.isBitmapType(type)? "qrc:///font-type-bitmap.svg"
                                                                  : fontFunctions.isPostScriptType(type)? "qrc:///font-type-postscript.svg"
                                                                                                    : fontFunctions.isOpenType(type)? "qrc:///font-type-opentype.svg":"qrc:///light_system-help.svg";
                        }
                        ToolButton {
                            enabled: false;
                            padding: 0;
                            width: parent.imgHeight;
                            height: parent.imgHeight;
                            icon.width: parent.imgHeight;
                            icon.height: parent.imgHeight;
                            palette: fontDelegateItem.palette;
                            icon.color: nameLabel.color;
                            visible: fontFamilyDelegate.isVariable;
                            icon.source: "qrc:///font-type-opentype-variable.svg"
                        }

                        ToolButton {
                            enabled: false;
                            padding: 0;
                            width: parent.imgHeight;
                            height: parent.imgHeight;
                            icon.width: parent.imgHeight;
                            icon.height: parent.imgHeight;
                            visible: fontFamilyDelegate.colorBitmap;
                            icon.source: "qrc:///font-color-type-clr-bitmap.svg"
                        }
                        ToolButton {
                            enabled: false
                            padding: 0;
                            width: parent.imgHeight;
                            height: parent.imgHeight;
                            icon.width: parent.imgHeight;
                            icon.height: parent.imgHeight;
                            visible: fontFamilyDelegate.colorCLRV0;
                            icon.source: "qrc:///font-color-type-clr-v0.svg"
                        }
                        ToolButton {
                            padding: 0;
                            enabled: false;
                            width: parent.imgHeight;
                            height: parent.imgHeight;
                            icon.width: parent.imgHeight;
                            icon.height: parent.imgHeight;
                            visible: fontFamilyDelegate.colorCLRV1;
                            icon.source: "qrc:///font-color-type-clr-v1.svg"
                        }
                        ToolButton {
                            enabled: false;
                            padding: 0;
                            width: parent.imgHeight;
                            height: parent.imgHeight;
                            icon.width: parent.imgHeight;
                            icon.height: parent.imgHeight;
                            visible: fontFamilyDelegate.colorSVG;
                            icon.source: "qrc:///font-color-type-svg.svg"
                        }
                    }
                }
            }
        }

        Label {
            id: nameLabel;
            palette: fontDelegateItem.palette;
            text: delegateControl.fontName;
            anchors.top: parent.top;
            anchors.left: parent.left;
            elide: Text.ElideRight;
            width: parent.width;
            color: parent.highlighted? palette.highlightedText: palette.text;
        }

    }
    background: Rectangle {
        id: fontDelegateBackground
        color: highlighted? fontDelegateItem.palette.highlight: fontDelegateItem.palette.window;
    }

    MouseArea {
        id: delegateMouseArea;
        acceptedButtons: Qt.RightButton | Qt.LeftButton;
        anchors.fill: parent;
        hoverEnabled: true;
        // Itemdelegate can also handle hovers, but we don't use those.
        preventStealing: true;
        onClicked: {
            if (mouse.button === Qt.RightButton) {
                resourceView.openContextMenu(mouse.x, mouse.y, parent.model.name, parent.index);
            } else {
                resourceView.applyHighlightedIndex();
                fontDelegateItem.resourceLeftClicked();
            }
        }
        onDoubleClicked: {
            fontDelegateItem.resourceDoubleClicked();
        }

        onContainsMouseChanged: {
            if(containsMouse) {
                resourceView.highlightedIndex = parent.index;
            }
        }

        Kis.ToolTipBase {
            text: fontDelegateItem.fontName;
            visible: delegateMouseArea.containsMouse;
        }
    }
}
