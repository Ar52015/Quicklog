import QtQuick
import QtQuick.Controls.Fusion

Popup {
    id: popup

    signal phaseAccepted(string name, string approach)
    signal openClicked

    anchors.centerIn: Overlay.overlay
    modal: true
    focus: true
    onOpened: {
        nameField.clear();
        approachField.clear();
        nameField.forceActiveFocus();
    }

    Column {
        spacing: 8

        Row {
            spacing: 8

            TextField {
                id: nameField
                width: (popup.availableWidth - 8) * 2 / 5
                placeholderText: "New phase name"
                background: InputFrame {
                    focused: nameField.activeFocus
                }
                onAccepted: addButton.clicked()
            }

            TextField {
                id: approachField
                width: (popup.availableWidth - 8) * 3 / 5
                placeholderText: "Approach (optional)"
                background: InputFrame {
                    focused: approachField.activeFocus
                }
                onAccepted: addButton.clicked()
            }
        }

        Row {
            anchors.right: parent.right
            spacing: 8

            EnterButton {
                id: addButton
                text: "Add"
                onClicked: {
                    popup.phaseAccepted(nameField.text, approachField.text);
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
