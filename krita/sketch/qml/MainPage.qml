/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 1.1
import org.krita.sketch 1.0
import "components"
import "panels"

Page {
    property string pageName: "MainPage";
    SketchView {
        id: sketchView;
        width: parent.width;
        height: parent.height;

        onInteractionStarted: { panelBar.collapse(); Krita.VirtualKeyboardController.requestHideKeyboard(); }
        onLoadingFinished: {
            loadingDialog.hide("Done!");
            savingDialog.hide("Done!");
        }
        onSavingFinished: {
            loadingDialog.hide("Done!");
            savingDialog.hide("Done!");
        }
        onProgress: {
            if (value === -1 || value === 100) {
                loadingDialog.hide("Done!");
                savingDialog.hide("Done!");
            }
            loadingDialog.progress = value;
            savingDialog.progress = value;
        }
        onViewChanged: if (window.sketchKisView !== undefined) { window.sketchKisView = view; }
    }
    Connections {
        target: window;
        onSwitchedToSketch: sketchView.activate();
    }


    ToolManager {
        id: toolManager;
        view: sketchView.view;
    }

    LayerModel {
        id: layerModel;
        // Notice - the model needs to know about the engine before the view, hence it is set first
        // This could be rectified, but for now know that the order here is important.
        engine: QMLEngine;
        view: sketchView.view;
    }

    PanelBar {
        id: panelBar;
        height: parent.height;
        width: parent.width;
    }

    NewImagePanel {
        id: newPanel;
        anchors.left: parent.left;
        width: Constants.GridWidth * 4;
        height: parent.height;
    }

    OpenImagePanel {
        id: openPanel;
        anchors.left: parent.left;
        width: Constants.GridWidth * 4;
        height: parent.height;

        onOpenClicked: pageStack.push(openImagePage);
    }

    MenuPanel {
        id: menuPanel;

        anchors.bottom: parent.bottom;

        width: parent.width;
        z: 10;

        newButtonChecked: !newPanel.collapsed;
        openButtonChecked: !openPanel.collapsed;

        onCollapsedChanged: if ( collapsed ) {
            newPanel.collapsed = true;
            openPanel.collapsed = true;
        }

        onButtonClicked: {
            switch( button ) {
                case "new": {
                    d.previousFile = Settings.currentFile;
                    newPanel.collapsed = !newPanel.collapsed;
                    openPanel.collapsed = true;
                }
                case "open": {
                    d.previousFile = Settings.currentFile;
                    openPanel.collapsed = !openPanel.collapsed;
                    newPanel.collapsed = true;
                }
                case "save":
                    if (!Settings.temporaryFile) {
                        savingDialog.show("Saving file...");
                        sketchView.save();
                    } else {
                        pageStack.push( saveAsPage, { view: sketchView } );
                    }
                case "saveAs":
                    pageStack.push( saveAsPage, { view: sketchView } );
                case "settings":
                    pageStack.push( settingsPage );
                case "help":
                    pageStack.push( helpPage );
                case "undo":
                    sketchView.undo();
                case "redo":
                    sketchView.redo();
                case "minimize":
                    Krita.Window.minimize();
                case "close":
                    Krita.Window.close();
                case "zoomIn":
                    sketchView.zoomIn();
                case "zoomOut":
                    sketchView.zoomOut();
                case "switchToDesktop":
                    switchToDesktopAction.trigger();
            }
        }
    }

    MessageStack {
        id: messageStack;
        anchors {
            horizontalCenter: parent.horizontalCenter;
            bottom: menuPanel.top;
            bottomMargin: Constants.GridHeight;
        }
        Connections {
            target: sketchView;
            onFloatingMessageRequested: messageStack.showMessage(message, iconName);
        }
    }

    Dialog {
        id: loadingDialog;
        title: "Loading";
        message: "Please wait...";
        textAlign: Text.AlignHCenter;
        progress: 0;
        modalBackgroundColor: "#ffffff";
    }

    Dialog {
        id: savingDialog;
        title: "Saving";
        message: "Please wait...";
        textAlign: Text.AlignHCenter;

        modalBackgroundColor: "#ffffff";
    }

    Dialog {
        id: progressDialog;

        title: Krita.ProgressProxy.taskName != "" ? Krita.ProgressProxy.taskName : "Applying...";
    }

    Dialog {
        id: modifiedDialog;
        title: "Image was modified";
        message: "The image was modified. Do you want to save your changes?";

        buttons: [ "Save", "Discard", "Cancel" ];

        onButtonClicked: {
            switch(button) {
                case 0: {
                    if (Settings.temporaryFile) {
                        pageStack.push( saveAsPage, { view: sketchView, updateCurrentFile: false } );
                    } else {
                        savingDialog.show("Please wait...");
                        sketchView.save();

                        if (d.closeRequested) {
                            d.closeWindow();
                        } else {
                            d.loadNewFile();
                        }
                    }
                }
                case 1: {
                    if (d.closeRequested) {
                        d.closeWindow();
                    } else {
                        d.loadNewFile();
                    }
                }
                default: {
                    Settings.currentFile = d.previousFile;
                    d.saveRequested = false;
                    d.closeRequested = false;
                }
            }
        }

        onCanceled: {
            Settings.currentFile = d.previousFile;
            d.saveRequested = false;
            d.closeRequested = false;
        }
    }

    Connections {
        target: Krita.ProgressProxy;

        onTaskStarted: progressDialog.show();
        onTaskEnded: progressDialog.hide();

        onValueChanged: progressDialog.progress = value;
    }

    Connections {
        target: Settings;

        onCurrentFileChanged: {
            if (sketchView.modified) {
                d.saveRequested = true;
                modifiedDialog.show();
            } else {
                d.loadNewFile();
            }
        }
        onTemporaryFileChanged: if (window.temporaryFile !== undefined) window.temporaryFile = Settings.temporaryFile;
    }

    onStatusChanged: {
        if (status == 0) {
            if (d.saveRequested) {
                d.loadNewFile();
                return;
            }

            if (d.closeRequested) {
                d.closeWindow();
                return;
            }

            sketchView.file = Settings.currentFile;
        }
    }

    Connections {
        target: Krita.Window;

        onCloseRequested: {
            if (sketchView.modified) {
                d.closeRequested = true;
                modifiedDialog.show();
            } else {
                d.closeWindow();
            }
        }
    }

    Component.onCompleted: {
        Krita.Window.allowClose = false;
        loadingDialog.show("Please wait...");
    }

    Component { id: openImagePage; OpenImagePage { } }
    Component { id: settingsPage; SettingsPage { } }
    Component { id: helpPage; HelpPage { } }
    Component { id: saveAsPage; SaveImagePage { } }

    QtObject {
        id: d;

        property string previousFile;
        property bool closeRequested;
        property bool saveRequested;

        function loadNewFile() {
            d.saveRequested = false;
            loadingDialog.show("Loading " + Settings.currentFile);
            loadingDialog.progress = 0;

            sketchView.file = Settings.currentFile;
            menuPanel.collapsed = true;
        }

        function closeWindow() {

            Krita.Window.allowClose = true;
            Krita.Window.closeWindow();
        }
    }
}
