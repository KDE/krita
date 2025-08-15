/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

/**
  This should be a dialog, but because qml doesn't get proper dialog handling till 6.8,
  it only is an item that we put inside a kodialog.
  **/

Control {
    id: control;
    property TextPropertyConfigModel configModel;
    onConfigModelChanged: {
        defaultVisibilityCmb.currentIndex = defaultVisibilityCmb.indexOfValue(configModel.defaultVisibilityState);
    }

    Kis.ThemedControl {
        id: pal;
    }
    palette: pal.palette;
    background: Rectangle {
        color: palette.window;
    }

    ColumnLayout {

        anchors.fill: parent;

        RowLayout {
            Layout.fillWidth: true;
            Label {
                text: i18nc("@label:listbox", "Default Visibility:");
                Layout.fillWidth: true;
            }

            ComboBox {
                id: defaultVisibilityCmb;
                model: [
                    {text: i18nc("@label:inlistbox", "When Relevant"), value: TextPropertyConfigModel.WhenRelevant},
                    {text: i18nc("@label:inlistbox", "When Set"), value: TextPropertyConfigModel.WhenSet},
                    {text: i18nc("@label:inlistbox", "Always Show"), value: TextPropertyConfigModel.AlwaysVisible}
                ]

                textRole: "text";
                valueRole: "value";
                palette: control.palette;
                Layout.preferredWidth: implicitWidth;
                onActivated: {
                    if (control.configModel) {
                        control.configModel.defaultVisibilityState = currentValue;
                    }
                }
            }

        }

        Frame {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            padding: 0;
            background: Rectangle {
                color: control.palette.base;
            }

            ListView {
                id: configListView;
                model: configModel;
                anchors.fill: parent;
                clip: true;

                ScrollBar.vertical: ScrollBar {
                }

                delegate: ItemDelegate {
                    id: configDelegate;
                    width: configListView.width;
                    height: implicitHeight;
                    required property var model;
                    required property int index;

                    palette: control.palette;

                    background: Rectangle {
                        visible: false;
                    }

                    contentItem: RowLayout {
                        Label {
                            text: configDelegate.model.title;
                            Layout.fillWidth: true;
                            palette: control.palette;
                        }
                        ComboBox {
                            id: visibilityCmb;
                            Layout.minimumWidth: implicitWidth;
                            Layout.preferredWidth: implicitWidth * 1.1;
                            Layout.maximumWidth: configDelegate.width * 0.5;
                            model: [
                                {text: i18nc("@label:inlistbox", "Follow Default"), value: TextPropertyConfigModel.FollowDefault},
                                {text: i18nc("@label:inlistbox", "When Relevant"), value: TextPropertyConfigModel.WhenRelevant},
                                {text: i18nc("@label:inlistbox", "When Set"), value: TextPropertyConfigModel.WhenSet},
                                {text: i18nc("@label:inlistbox", "Always Show"), value: TextPropertyConfigModel.AlwaysVisible},
                                {text: i18nc("@label:inlistbox", "Never Show"), value: TextPropertyConfigModel.NeverVisible}
                            ]

                            palette: control.palette;
                            textRole: "text";
                            valueRole: "value";
                            Component.onCompleted: {
                                currentIndex = indexOfValue(configDelegate.model.visibility);
                            }

                            onActivated: configDelegate.model.visibility = currentValue;
                        }
                    }
                }
            }
        }
    }
}
