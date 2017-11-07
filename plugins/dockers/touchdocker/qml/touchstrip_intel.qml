import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Rectangle {
    id: base
    color: "#545454"


    Button {
        id: brushSmall
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 13
        anchors.left: base.left
        anchors.top: base.top
        anchors.margins: 2
        image: Settings.theme.icon("brush_small");
        onClicked: {
            mainWindow.slotButtonPressed("brushSmall")
        }
    }

    Button {
        id: brushMedium
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 13
        anchors.top: brushSmall.bottom
        anchors.left: base.left
        anchors.margins: 2
        image: Settings.theme.icon("brush_medium");
        onClicked: {
            mainWindow.slotButtonPressed("brushMedium")
        }
    }

    Button {
        id: brushLarge
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 13
        anchors.top: brushMedium.bottom
        anchors.left: base.left
        anchors.margins: 2
        image: Settings.theme.icon("brush_large");
        onClicked: {
            mainWindow.slotButtonPressed("brushLarge")
        }
    }

    Button {
        id: preset1
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 13
        anchors.top: brushLarge.bottom
        anchors.left: base.left
        anchors.margins: 2
        text: "Optimized Brush"
        //image: Settings.theme.icon("brush_large");
        onClicked: {
            mainWindow.slotButtonPressed("preset1")
        }
    }

    Button {
        id: preset2
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 13
        anchors.top: preset1.bottom
        anchors.left: base.left
        anchors.margins: 2
        text: "Unoptimized Brush"
        //image: Settings.theme.icon("brush_large");
        onClicked: {
            mainWindow.slotButtonPressed("preset2")
        }
    }

    Button {
        id: colorWhite
        color: "white"
        radius: 8
        width: base.width
        height: base.height / 18
        anchors.top: preset2.bottom
        anchors.left: base.left
        anchors.margins: 2
        onClicked: {
            mainWindow.slotButtonPressed("white")
        }
    }

    Button {
        id: colorBlack
        color: "black"
        radius: 8
        width: base.width
        height: base.height / 18
        anchors.top: colorWhite.bottom
        anchors.left: base.left
        anchors.margins: 2
        onClicked: {
            mainWindow.slotButtonPressed("black")
        }
    }

    Button {
        id: colorBlue
        color: "blue"
        radius: 8
        width: base.width
        height: base.height / 18
        anchors.top: colorBlack.bottom
        anchors.left: base.left
        anchors.margins: 2
        onClicked: {
            mainWindow.slotButtonPressed("blue")
        }
    }

    Button {
        id: colorGreen
        color: "green"
        radius: 8
        width: base.width
        height: base.height / 18
        anchors.top: colorBlue.bottom
        anchors.left: base.left
        anchors.margins: 2
        onClicked: {
            mainWindow.slotButtonPressed("green")
        }
    }

    Button {
        id: colorRed
        color: "red"
        radius: 8
        width: base.width
        height: base.height / 18
        anchors.top: colorGreen.bottom
        anchors.left: base.left
        anchors.margins: 2
        onClicked: {
            mainWindow.slotButtonPressed("red")
        }
    }

    Button {
        id: colorYellow
        color: "yellow"
        radius: 8
        width: base.width
        height: base.height / 18
        anchors.top: colorRed.bottom
        anchors.left: base.left
        anchors.margins: 2
        onClicked: {
            mainWindow.slotButtonPressed("yellow")
        }
    }

    Button {
        id: colorGrey
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 18
        anchors.top: colorYellow.bottom
        anchors.left: base.left
        anchors.margins: 2
        onClicked: {
            mainWindow.slotButtonPressed("grey")
        }
    }

    Button {
        id: clear
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 10
        anchors.top: colorGrey.bottom
        anchors.left: base.left
        anchors.margins: 2
        image: Settings.theme.icon("erase");
        onClicked: {
            mainWindow.slotButtonPressed("clear")
        }
    }


    Button {
        id: close
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 10
        anchors.top: clear.bottom
        anchors.left: base.left
        anchors.margins: 2
        image: Settings.theme.icon("close");
        onClicked: {
            mainWindow.slotButtonPressed("close")
        }
    }

    Button {
        id: fps
        color: "grey"
        radius: 8
        width: base.width
        height: base.height / 10
        anchors.top: clear.bottom
        anchors.left: base.left
        anchors.margins: 2
        checkable: true
        text: "FPS/Brush Speed"
        onClicked: {
            mainWindow.slotButtonPressed("fps")
        }
    }
}
