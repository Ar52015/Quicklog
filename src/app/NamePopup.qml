import QtQuick
import QtQuick.Controls.Fusion

Popup {
    id: popup

    property alias placeholderText: nameField.placeholderText
    property string acceptText
    property string extraText

    signal nameAccepted(string name)
    signal extraClicked

    anchors.centerIn: Overlay.overlay
    modal: true
    focus: true
    onOpened: {
        nameField.clear();
        nameField.forceActiveFocus();
    }

    Row {
        spacing: 8

        TextField {
            id: nameField
            width: popup.availableWidth - buttons.width - 8
            background: InputFrame {
                focused: nameField.activeFocus
            }
            onAccepted: acceptButton.clicked()
        }

        Row {
            id: buttons
            spacing: 8

            EnterButton {
                id: acceptButton
                text: popup.acceptText
                onClicked: {
                    popup.nameAccepted(nameField.text);
                    popup.close();
                }
            }

            EnterButton {
                visible: popup.extraText !== ""
                text: popup.extraText
                onClicked: {
                    popup.extraClicked();
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
