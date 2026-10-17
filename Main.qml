pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    id: window

    property string errorText: ""
    property string entryType: ""

    function addEntry() {
        backend.addEntry(frontBox.currentText, phaseBox.currentText, entryType, field.text);
    }

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
        onEntryAdded: {
            field.clear();
            window.entryType = "";
            window.errorText = "";
        }
        onEntryFailed: message => window.errorText = message
    }

    TextField {
        id: field
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        placeholderText: "make an entry?"
        enabled: backend.logExists
        onTextChanged: window.errorText = ""
        onAccepted: {
            if (text.trim() !== "") {
                window.addEntry();
            }
        }
    }

    Text {
        id: preview
        color: window.errorText !== "" ? "red" : field.color
        anchors.top: field.bottom
        anchors.left: field.left
        anchors.right: field.right
        anchors.topMargin: 8
        textFormat: window.errorText !== "" ? Text.PlainText : Text.MarkdownText
        text: {
            if (window.errorText !== "") {
                return window.errorText;
            }
            if (backend.logExists && addButton.hovered && field.text === "") {
                return "Add front without logs?";
            }
            return backend.formatBullet(window.entryType, field.text);
        }
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

        Row {
            spacing: 2

            Repeater {
                model: ["📌", "❓", "💡", "🐛"]

                Button {
                    required property string modelData

                    width: 30
                    text: modelData
                    highlighted: window.entryType === modelData
                    enabled: backend.logExists
                    onClicked: window.entryType = window.entryType === modelData ? "" : modelData
                }
            }
        }

        Button {
            id: addButton
            width: 48
            text: "+"
            onClicked: window.addEntry()
        }

        Button {
            width: 48
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
