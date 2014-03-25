import QtQuick 1.1
import "../../components"

Column {
    Item {
        width: parent.width;
        height: Constants.GridHeight;
    }
    Label {
        width: parent.width;
        wrapMode: Text.WordWrap;
        horizontalAlignment: Text.AlignHCenter;
        elide: Text.ElideNone;
        text: "This filter requires no configuration. Click below to apply it.";
    }
    Item {
        width: parent.width;
        height: Constants.GridHeight / 2;
    }
    Button {
        width: height;
        height: Constants.GridHeight
        anchors.horizontalCenter: parent.horizontalCenter;
        image: Settings.theme.icon("apply");
        onClicked: fullFilters.model.activateFilter(fullFilters.currentIndex);
    }
}
