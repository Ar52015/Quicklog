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
        Qt.callLater(frontBox.selectDefault);
    }

    Backend {
        id: backend
        onFrontsChanged: Qt.callLater(frontBox.selectDefault)
        onPhasesChanged: Qt.callLater(phaseBox.selectDefault)
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

    ComboBox {
        id: frontBox
        anchors.top: preview.bottom
        anchors.left: field.left
        anchors.topMargin: 8
        model: backend.fronts
        enabled: backend.logExists
        implicitContentWidthPolicy: ComboBox.WidestText
        onCurrentTextChanged: backend.currentFront = currentText

        function selectDefault() {
            currentIndex = Math.max(0, find(backend.defaultFront));
        }
    }

    ComboBox {
        id: phaseBox
        anchors.top: frontBox.top
        anchors.left: frontBox.right
        anchors.leftMargin: 8
        width: frontBox.width
        model: backend.phases
        enabled: backend.logExists

        function selectDefault() {
            currentIndex = Math.max(0, find(backend.defaultPhase));
        }
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
