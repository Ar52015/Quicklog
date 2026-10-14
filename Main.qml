import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    width: 640
    height: 110
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
        enabled: backend.logExists
    }

    Text {
        id: preview
        color: field.color
        anchors.top: field.bottom
        anchors.left: field.left
        anchors.right: field.right
        anchors.topMargin: 8
        textFormat: Text.MarkdownText
        text: backend.formatBullet(field.text)
    }

    Row {
        anchors.top: preview.bottom
        anchors.right: field.right
        anchors.topMargin: 8
        spacing: 8

        Button {
            text: "+"
        }

        Button {
            text: "↻"
            onClicked: backend.refresh()
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Choose the vault folder"
        onAccepted: backend.setVaultFolderUrl(selectedFolder)
    }
}
