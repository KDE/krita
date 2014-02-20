import QtQuick 1.1
import org.krita.sketch 1.0
import "colors.js" as Colors
import "sizes.js" as Sizes
import "fonts.js" as Fonts

Theme {
    id: "default"
    name: "Default"

    colors: Colors.values;
    sizes: Sizes.values;
    fonts: Fonts.values;

    iconPath: "icons/"
}
