import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.1

ColumnLayout {
    property int propertyType: TextPropertyBase.Character;

    ListModel {
        id: propertyList;
    }

    Component.onCompleted: fillPropertyList();

    function fillPropertyList() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            propertyWidgetModel.get(i).parentPropertyType = propertyType;
            if (propertyWidgetModel.get(i).propertyType === propertyType ||
                    propertyWidgetModel.get(i).propertyType === TextPropertyBase.Mixed) {
            propertyList.append({"name": propertyWidgetModel.get(i).propertyName,
                                    "tooltip": propertyWidgetModel.get(i).toolTip,
                                    "searchTerms": propertyWidgetModel.get(i).searchTerms,
                                    "visible": propertyWidgetModel.get(i).visible,
                                    "item": propertyWidgetModel.get(i)});
            } else {
                propertyWidgetModel.get(i).visible = false;
            }
        }
    }

    function updateProperties() {
        for (var i = 0; i < propertyWidgetModel.count; i++) {
            if (propertyWidgetModel.get(i).propertyType === propertyType ||
                    propertyWidgetModel.get(i).propertyType === TextPropertyBase.Mixed) {
                propertyWidgetModel.get(i).propertiesUpdated();
            } else {
                propertyWidgetModel.get(i).visible = false;
            }
        }
    }

    Frame {
        id: frame;
        Layout.fillHeight: true;
        Layout.fillWidth: true;
        clip: true;
        padding: 0;

        ListView {
            id: propertiesWidgetView

            anchors.fill: parent;
            ScrollBar.vertical: ScrollBar {
            }

            model: ObjectModel {
                id: propertyWidgetModel;
                WritingMode {
                }
                Direction {
                }
                TextIndent{
                }
                TextAlign{
                }
                DominantBaseline {
                }
                /*
                WhiteSpace {
                }
                UnderlinePosition {
                }*/
                HangingPunctuation {
                }
                TabSize {
                }
                FontSize {
                }
                FontFamily {
                    fontFamilyModel: fontFamiliesModel;
                }
                FontStyle {
                }
                LetterSpacing {
                }
                WordSpacing {
                }
                LineHeight {
                }
                LineBreak {
                }
                WordBreak {
                }
                TextTransform {
                }
                TextDecoration {
                }
                /*
            OTLigatures {
            }
            OTPosition {
            }
            OTNumeric {
            }
            OTCaps {
            }
            OTEastAsian {
            }*/
                BaselineShift {
                }
                AlignmentBaseline {
                }
            }
        }
    }

    ComboBox {
        Layout.fillWidth: true;
        Layout.minimumHeight: implicitHeight;
        model: propertyList;
        textRole: "name";
        displayText: i18nc("@label:listbox", "Add Property");
        onActivated: {
            model.get(currentIndex).item.visible = true;
        }
    }
}
