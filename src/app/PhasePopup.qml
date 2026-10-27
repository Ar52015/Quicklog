import QtQuick
import QtQuick.Controls.Fusion

Popup {
    id: popup

    property string parentPhase

    signal phaseAccepted(string name, string approach)
    signal subphaseAccepted(string name)
    signal openClicked

    anchors.centerIn: Overlay.overlay
    modal: true
    focus: true
    onOpened: {
        nameField.clear();
        approachField.clear();
        subphaseBox.checked = false;
        nameField.forceActiveFocus();
    }

    Column {
        spacing: 8

        Row {
            spacing: 8

            TextField {
                id: nameField
                width: (popup.availableWidth - 8) * 2 / 5
                placeholderText: subphaseBox.checked ? "Subphase name" : "New phase name"
                background: InputFrame {
                    focused: nameField.activeFocus
                }
                onAccepted: addButton.clicked()
            }

            TextField {
                id: approachField
                width: (popup.availableWidth - 8) * 3 / 5
                placeholderText: "Approach (optional)"
                enabled: !subphaseBox.checked
                background: InputFrame {
                    focused: approachField.activeFocus
                }
                onAccepted: addButton.clicked()
            }
        }

        Item {
            width: popup.availableWidth
            height: buttons.height

            CheckBox {
                id: subphaseBox
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - buttons.width - 8
                visible: popup.parentPhase !== ""
                focusPolicy: Qt.TabFocus
                text: "subphase of " + popup.parentPhase
            }

            Row {
                id: buttons
                anchors.right: parent.right
                spacing: 8

                EnterButton {
                    id: addButton
                    text: "Add"
                    onClicked: {
                        if (subphaseBox.checked) {
                            popup.subphaseAccepted(nameField.text);
                        } else {
                            popup.phaseAccepted(nameField.text, approachField.text);
                        }
                        popup.close();
                    }
                }

                EnterButton {
                    text: "Open in Obsidian"
                    onClicked: {
                        popup.openClicked();
                        popup.close();
                    }
                }

                EnterButton {
                    text: "Cancel"
                    onClicked: popup.close()
                }
            }
        }
    }
}
