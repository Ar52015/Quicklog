import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    width: 640
    height: 80
    visible: true
    title: "Quicklog"

    Component.onCompleted: {
        if (backend.vaultFolder === "") {
            folderDialog.open();
        }
    }

    Backend {
        id: backend
    }

    TextField {
        id: field
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        placeholderText: "make an entry?"
    }

    Text {
        anchors.top: field.bottom
        anchors.left: field.left
        anchors.right: field.right
        anchors.topMargin: 8
        textFormat: Text.MarkdownText
        text: backend.formatBullet(field.text)
    }

    FolderDialog {
        id: folderDialog
        title: "Choose the vault folder"
        onAccepted: backend.setVaultFolderUrl(selectedFolder)
    }
}
