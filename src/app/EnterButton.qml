import QtQuick
import QtQuick.Controls.Fusion

Button {
    id: control

    focusPolicy: Qt.TabFocus
    background: Rectangle {
        implicitWidth: 80
        implicitHeight: 24
        radius: 4
        color: control.down ? RosePine.highlightMed : control.hovered || control.highlighted ? RosePine.overlay : RosePine.surface
        border.color: control.visualFocus || control.highlighted ? RosePine.iris : RosePine.highlightHigh
    }
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            event.accepted = true;
            if (!event.isAutoRepeat) {
                click();
            }
        }
    }
}
