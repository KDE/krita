/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

var values = {
    base: {
        base: "#a0a0a0",
        text: "#ffffff",
        header: "#000000",
        headerText: "#ffffff",
    },

    components: {
        button: {
            base: "gray",
            text: "#ffffff",
            highlight: "lightgray",
            checked: Qt.rgba(1.0, 1.0, 1.0, 0.7),
        },
        colorSwatch: {
            border: "silver",
        },
        dialog: {
            modalOverlay: Qt.rgba(0.0, 0.0, 0.0, 0.5),
            background: {
                start: "#F7F8FC",
                stop: "#F0F0FA",
            },
            header: "#9AA1B2",
            headerText: "#ffffff",
            progress: {
                background: "#ffffff",
                border: "silver",
                bar: "gray",
            },
            button: "#9AA1B2",
            buttonText: "#ffffff",
        },
        expandingListView: {
            selection: {
                border: "#d2d2d2",
                fill: Qt.rgba(1.0, 1.0, 1.0, 0.4),
                text: Qt.rgba(0.0, 0.0, 0.0, 0.65),
            },
            list: {
                background: Qt.rgba(1.0, 1.0, 1.0, 0.4),
                item: "#ffffff",
                itemBorder: "#d2d2d2",
                itemText: Qt.rgba(0.0, 0.0, 0.0, 0.65),
            },
        },
        header: "#ffffff",
        label: "#323232",
        listItem: {
            background: {
                start: "#FBFBFB",
                stop: "#F0F0F0",
            },
            title: "#000000",
            description: "#333333",
        },
        messageStack: {
            background: "gray",
            border: "silver",
            text: "white",
            button: {
                fill: "gray",
                border: "silver",
            },
        },
        newImageList: {
            start: "#FBFBFB",
            stop: "#F0F0F0",
        },
        newsList: {
            listItem: {
                start: "#FBFBFB",
                stop: "#F0F0F0",
                moreLink: "#999999",
            },
            title: "#000000",
            date: "#666666",
            description: "#000000",
            backLink: "#999999",
        },
        panelTextField: {
            border: "#d2d2d2",
            background: Qt.rgba(1.0, 1.0, 1.0, 0.4),
            enabled: Qt.rgba(0.0, 0.0, 0.0, 0.0),
            disabled: Qt.rgba(0.0, 0.0, 0.0, 0.7),
        },
        rangeInput: {

        },
        recentFilesList: {
            start: "#FBFBFB",
            stop: "#F0F0F0",
        },
        scrollDecorator: {
            base: "silver",
            border: "transparent"
        },
        slider: {
            background: {
                fill: Qt.rgba(1.0, 1.0, 1.0, 0.75),
                border: "silver",
            },
            handle: {
                fill: "silver",
                border: "transparent",
            },
        },
        textField: {
            background: Qt.rgba(1.0, 1.0, 1.0, 0.75),
            border: {
                normal: "silver",
                error: "red",
                focus: "gray",
            },
            placeholder: Qt.rgba(0.0, 0.0, 0.0, 0.5),
        },
        textFieldMultiline: {
            background: Qt.rgba(1.0, 1.0, 1.0, 0.75),
            border: "silver",
        },
    },

    panels: {
        dropArea: {
            fill: Qt.rgba(1.0, 1.0, 1.0, 0.25),
            border: "#ffffff",
        },

        base: {
            base: "#a0a0a0",
            text: "#ffffff",
            header: "#000000",
            headerText: "#ffffff",
        },

        presets: {
            base: "#a0a0a0",
            text: "#ffffff",
            header: "#000000",
            headerText: "#ffffff",

            preset: {
                active: "#D7D7D7",
                inactive: "transparent",
            }
        },

        layers: {
            base: "#a0a0a0",
            text: "#ffffff",
            header: "#000000",
            headerText: "#ffffff",
            subheader: "#cfcfcf",
            editorButtons: {
                active: "#EAEAEA",
                inactive: "transparent",
                text: "gray",
                border: "silver",
            },
            layer: {
                active: Qt.rgba(1.0, 1.0, 1.0, 0.5),
                inactive: Qt.rgba(1.0, 1.0, 1.0, 0.2),
                background: "#d7d7d7",
                text: "#000000",
                visible: "#ffffff",
                locked: "#ffffff",
            },
        },

        selection: {
            base: "#a0a0a0",
            text: "#ffffff",
            header: "#000000",
            headerText: "#ffffff",

            buttons: {
                color: Qt.rgba(1.0, 1.0, 1.0, 0.4),
                text: "black",
                border: "silver"
            }
        },

        filter: {
            base: "#a0a0a0",
            text: "#ffffff",
            header: "#000000",
            headerText: "#ffffff",
        },

        color: {
            base: "#a0a0a0",
            text: "#ffffff",
            header: "#000000",
            headerText: "#ffffff",
        },

        tool: {
            base: "#a0a0a0",
            text: "#ffffff",
            header: "#000000",
            headerText: "#ffffff",
            subheader: "#cfcfcf",
        },

        menu: {
            base: "#000000",
            text: "#ffffff",
            buttonHighlight: "#dcdcdc",
        },

        newImage: {
            background: "#ffffff",
            header: {
                start: "#707070",
                stop: "#565656",
                text: "#ffffff",
            }
        },

        openImage: {
            background: "#ffffff",
            header: {
                start: "#707070",
                stop: "#565656",
                text: "#ffffff",
            }
        },
    },

    pages: {
        welcome: {
            background: "#ffffff",
            open:{
                header: {
                    start: "#707070",
                    stop: "#565656",
                    text: "#ffffff",
                },
            },
            create: {
                header: {
                    start: "#565656",
                    stop: "#707070",
                    text: "#ffffff",
                },
            },
            news: {
                header: {
                    start: "#707070",
                    stop: "#565656",
                    text: "#ffffff",
                },
            },
        },
        open: {
            background: "#ffffff",
            location: "#ffffff",
        },
        save: {
            background: "#ffffff",
            location: "#ffffff",
            footer: "#000000",
        },
        customImagePage: {
            background: "#ffffff",
            groupBox: "lightGray",
            controls: {
                background: Qt.rgba(1.0, 1.0, 1.0, 0.4),
                border: "white",
            },
        }
    },
}
