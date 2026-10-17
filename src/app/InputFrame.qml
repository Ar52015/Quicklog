import QtQuick

Rectangle {
    property bool focused
    property bool hovered

    radius: 4
    color: hovered ? RosePine.overlay : RosePine.surface
    border.color: focused ? RosePine.iris : RosePine.highlightHigh
}
