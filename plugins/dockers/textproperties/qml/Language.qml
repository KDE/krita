/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

TextPropertyBase {
    propertyTitle: i18nc("@title:group", "Language");
    propertyName: "locale";
    propertyType: TextPropertyConfigModel.Mixed;
    toolTip: i18nc("@info:tooltip",
                   "The language of this text shape. Language affects a number of properties, like glyph shape, upper- and lowercase and line breaking");
    searchTerms: i18nc("comma separated search terms for the language property, matching is case-insensitive",
                       "language, locale, bcp47");

    LocaleHandler {
        id: localeHandler;
        onBcp47TagChanged: {
            if (!blockSignals) {
                properties.language = localeHandler.bcp47Tag;
            }
        }

        onScriptChanged: scriptCmb.currentIndex = scriptCmb.indexOfValue(script);
        onLanguageChanged: languageCmb.currentIndex = languageCmb.indexOfValue(language);
    }

    Connections {
        target: properties;
        function onLanguageChanged() {
            updateBcpTags();
            updateVisibility();
        }

        function onLanguageStateChanged() {
            updateVisibility();
        }
    }
    onPropertiesChanged: {
        updateBcpTags();
        updateVisibility();
    }

    function updateBcpTags() {
        blockSignals = true;
        localeHandler.bcp47Tag = properties.language
        blockSignals = false;
    }

    function updateVisibility() {
        propertyState = [properties.languageState];
        setVisibleFromProperty();
    }

    onEnableProperty: properties.languageState = KoSvgTextPropertiesModel.PropertySet;
    GridLayout {
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.languageState;
            onClicked: properties.languageState = KoSvgTextPropertiesModel.PropertyUnset;
        }

        Label {
            text: propertyTitle + ":";
            elide: Text.ElideRight;
            Layout.alignment: Qt.AlignRight;
            font.italic: properties.lineHeightState === KoSvgTextPropertiesModel.PropertyTriState;
        }

        ComboBox {
            id: languageCmb;
            model: localeHandler.favoritesModel;
            textRole: "display";
            valueRole: "code";
            Layout.fillWidth: true;

            onDisplayTextChanged: {
                languageTxtEdit.blockSignals = true;
                languageTxtEdit.text = currentText;
                languageTxtEdit.blockSignals = false;
            }

            contentItem: TextField {
                id: languageTxtEdit;
                verticalAlignment: Text.AlignVCenter
                font: languageCmb.font
                color: languageCmb.palette.text
                selectionColor: languageCmb.palette.highlight
                selectedTextColor: languageCmb.palette.highlightedText;

                property bool blockSignals: false;

                selectByMouse: true;

                onTextChanged: {
                    if (!blockSignals) {
                        localeHandler.searchString = text;
                        if (text !== "") {
                            filterPopup.open();
                        }
                    }
                }

                onAccepted: {
                    blockSignals = true;
                    if (localeHandler.validBcp47Tag(text)) {
                        localeHandler.language = text;
                    }
                    finalize();
                    blockSignals = false;
                }

                function finalize() {
                    localeHandler.searchString = "";
                    filterPopup.close();
                }

                Popup {
                    id: filterPopup;
                    y: -height;
                    x: 0;
                    padding: 0;
                    width: languageTxtEdit.width;
                    height: Math.min(contentItem.implicitHeight, 300);

                    contentItem: ListView {
                        id: filteredLanguages;
                        model: localeHandler.languagesModel;
                        width: filterPopup.width;
                        implicitHeight: contentHeight;
                        clip:true;

                        delegate: ItemDelegate {
                            required property var model;
                            id: languageDelegate;
                            text: model.display + " ("+model.code+")";
                            width: filterPopup.width;
                            hoverEnabled: true;
                            highlighted: hovered;

                            background: Rectangle {
                                color: languageDelegate.highlighted? languageDelegate.palette.highlight: "transparent";
                            }
                            palette: filterPopup.palette;

                            ToolTip.text: text;
                            ToolTip.visible: hovered;
                            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;

                            onClicked: {
                                localeHandler.language = model.code;
                                languageTxtEdit.finalize();
                                filterPopup.close();
                            }
                        }

                        ScrollBar.vertical: ScrollBar {
                        }
                    }
                }


            }
            delegate: ItemDelegate {
                id: favoriteDelegate;
                required property var model;
                required property var index;
                palette: languageCmb.palette;

                width: languageCmb.width;
                hoverEnabled: true;
                highlighted: hovered;

                text: model.display + " ("+favoriteDelegate.model.code+")";

                contentItem: RowLayout {
                    CheckBox {
                        Component.onCompleted: {
                            checked = favoriteDelegate.model.favorite;
                        }
                        onCheckedChanged: favoriteDelegate.model.favorite = checked;
                        palette: favoriteDelegate.palette;
                        hoverEnabled: true;
                        padding: 0;

                        ToolTip.text: i18nc("@info:tooltip", "Store this language between sessions.");
                        ToolTip.visible: hovered;
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
                    }
                    Label {
                        text: favoriteDelegate.model.display + " ("+favoriteDelegate.model.code+")";
                        palette: favoriteDelegate.palette;
                        color: favoriteDelegate.highlighted? palette.highlightedText: palette.text;
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter;
                        Layout.fillWidth: true;
                    }
                }

                background: Rectangle {
                    color: favoriteDelegate.highlighted? favoriteDelegate.palette.highlight: "transparent";
                }
                onClicked: languageCmb.currentIndex = index;

            }

            onCurrentValueChanged: localeHandler.language = currentValue;
        }

        Item {
            width: 1;
            height: 1;
        }

        Label {
            text: i18nc("@label:listbox", "Script:");
            elide: Text.ElideRight;
            Layout.alignment: Qt.AlignRight;

        }

        ComboBox {
            id: scriptCmb;
            model: localeHandler.scriptModel();
            textRole: "name";
            valueRole: "code";
            Layout.fillWidth: true;

            enabled: localeHandler.localeValid;

            editable: true;

            Kis.ThemedControl {
                id: scriptPalette;
            }
            palette: scriptPalette.palette;

            onCurrentValueChanged: localeHandler.script = currentValue;
        }
    }
}
