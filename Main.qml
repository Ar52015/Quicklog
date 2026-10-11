import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 80
    visible: true
    title: "Quicklog"

    TextField {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        placeholderText: "make an entry?"
    }
}
