pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Window {
    id: window

    property string errorText: ""
    property string noticeText: ""
    property string entryType: ""

    function addEntry() {
        backend.addEntry(frontBox.front, phaseBox.phase, entryType, field.text);
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
        onFailed: message => window.errorText = message
        onNotice: message => window.noticeText = message
        onFrontAdded: front => Qt.callLater(() => frontBox.currentIndex = frontBox.find(front))
    }

    TextField {
        id: field
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        placeholderText: "make an entry?"
        enabled: backend.logExists
        focus: true
        onTextChanged: {
            window.errorText = "";
            window.noticeText = "";
        }
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
        textFormat: window.errorText !== "" || window.noticeText !== "" ? Text.PlainText : Text.MarkdownText
        text: {
            if (window.errorText !== "") {
                return window.errorText;
            }
            if (window.noticeText !== "") {
                return window.noticeText;
            }
            if (backend.logExists && addButton.hovered && field.text === "") {
                return "Add front without logs?";
            }
            return backend.formatBullet(window.entryType, field.text);
        }
    }

    ComboBox {
        id: frontBox

        readonly property string front: currentIndex >= 0 && currentIndex < count - 1 ? currentText : ""

        function selectDefault() {
            currentIndex = count > 1 ? Math.max(0, find(backend.defaultFront)) : -1;
        }

        anchors.top: preview.bottom
        anchors.left: field.left
        anchors.topMargin: 8
        model: backend.fronts.concat(["+ front"])
        focusPolicy: Qt.TabFocus
        enabled: backend.logExists
        implicitContentWidthPolicy: ComboBox.WidestText
        onFrontChanged: {
            if (front !== "") {
                backend.currentFront = front;
            }
        }
        onActivated: index => {
            if (index === count - 1) {
                currentIndex = find(backend.currentFront);
                newFrontPopup.open();
            }
        }
    }

    ComboBox {
        id: phaseBox

        readonly property string phase: currentIndex >= 0 && currentIndex < count - 1 ? currentText : ""
        property string chosen: ""

        function selectDefault() {
            currentIndex = count > 1 ? Math.max(0, find(backend.defaultPhase)) : -1;
        }

        anchors.top: frontBox.top
        anchors.left: frontBox.right
        anchors.leftMargin: 8
        width: frontBox.width
        model: backend.phases.concat(["+ phase"])
        focusPolicy: Qt.TabFocus
        enabled: backend.logExists
        onPhaseChanged: {
            if (phase !== "") {
                chosen = phase;
            }
        }
        onActivated: index => {
            if (index === count - 1) {
                backend.openFront(frontBox.front);
                const previous = find(chosen);
                if (previous >= 0) {
                    currentIndex = previous;
                } else {
                    selectDefault();
                }
            }
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
                    opacity: enabled ? 1 : 0.4
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
            onClicked: {
                window.errorText = "";
                window.noticeText = "";
                backend.refresh();
            }
        }
    }

    Popup {
        id: newFrontPopup

        anchors.centerIn: Overlay.overlay
        width: window.width - 24
        modal: true
        focus: true
        onOpened: {
            newFrontName.clear();
            newFrontName.forceActiveFocus();
        }
        onClosed: field.forceActiveFocus()

        Row {
            spacing: 8

            TextField {
                id: newFrontName
                width: newFrontPopup.availableWidth - createButton.width - cancelButton.width - 16
                placeholderText: "New front name"
                onAccepted: createButton.clicked()
            }

            Button {
                id: createButton
                text: "Create"
                onClicked: {
                    backend.addFront(newFrontName.text);
                    newFrontPopup.close();
                }
            }

            Button {
                id: cancelButton
                text: "Cancel"
                onClicked: newFrontPopup.close()
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Choose the vault folder"
        onAccepted: backend.setVaultFolderUrl(selectedFolder)
    }
}
