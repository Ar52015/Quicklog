import QtQuick
import QtQuick.Controls

Button {
    focusPolicy: Qt.TabFocus
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            event.accepted = true;
            if (!event.isAutoRepeat) {
                click();
            }
        }
    }
}
