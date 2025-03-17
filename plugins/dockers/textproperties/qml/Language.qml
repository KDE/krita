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

TextPropertyBase {
    propertyName: i18nc("@title:group", "Language");
    propertyType: TextPropertyBase.Mixed;
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

    onPropertiesUpdated: {
        blockSignals = true;
        localeHandler.bcp47Tag = properties.language;
        visible = properties.languageState !== KoSvgTextPropertiesModel.PropertyUnset;
        blockSignals = false;
    }

    onEnableProperty: properties.languageState = KoSvgTextPropertiesModel.PropertySet;
    GridLayout {
        columns: 2;
        columnSpacing: columnSpacing;
        width: parent.width;

        RevertPropertyButton {
            revertState: properties.languageState;
            onClicked: properties.languageState = KoSvgTextPropertiesModel.PropertyUnset;
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
                    localeHandler.searchString = "";
                    filterPopup.close();
                    blockSignals = false;
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

                            onClicked: {
                                localeHandler.language = model.code;
                                languageTxtEdit.accepted();
                                filterPopup.close();
                            }
                        }

                        ScrollBar.vertical: ScrollBar {
                        }
                    }
                }


            }
            delegate: CheckDelegate {
                id: favoriteDelegate;
                required property var model;
                required property var index;
                palette: languageCmb.palette;

                width: languageCmb.width;
                hoverEnabled: true;
                highlighted: hovered;

                text: model.display + " ("+favoriteDelegate.model.code+")";
                checked: favoriteDelegate.model.favorite;

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

        ComboBox {
            id: scriptCmb;
            model: localeHandler.scriptModel();
            textRole: "name";
            valueRole: "code";
            Layout.fillWidth: true;

            enabled: localeHandler.localeValid;

            editable: true;

            PaletteControl {
                id: scriptPalette;
                colorGroup: scriptCmb.enabled? SystemPalette.Active: SystemPalette.Disabled;
            }
            palette: scriptPalette.palette;

            onCurrentValueChanged: localeHandler.script = currentValue;
        }
    }
}
