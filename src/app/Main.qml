pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
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
    height: statusRow.y + statusRow.height + 12
    visible: true
    title: "Quicklog"
    color: palette.window
    palette {
        window: RosePine.base
        base: RosePine.surface
        alternateBase: RosePine.highlightLow
        light: RosePine.surface
        mid: RosePine.highlightHigh
        dark: RosePine.muted
        placeholderText: RosePine.subtle
        button: RosePine.surface
        highlight: RosePine.highlightMed
        highlightedText: RosePine.text
        link: RosePine.iris
        active {
            windowText: RosePine.text
            text: RosePine.text
            buttonText: RosePine.text
        }
        inactive {
            windowText: RosePine.text
            text: RosePine.text
            buttonText: RosePine.text
        }
        disabled {
            windowText: RosePine.muted
            text: RosePine.muted
            buttonText: RosePine.muted
        }
    }

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
        onPhaseAdded: phase => Qt.callLater(() => phaseBox.currentIndex = phaseBox.find(phase))
        onPhaseStatusChanged: phase => Qt.callLater(() => {
                const index = phaseBox.find(phase);
                if (index >= 0) {
                    phaseBox.currentIndex = index;
                }
            })
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
        background: InputFrame {
            focused: field.activeFocus
        }
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

    Item {
        id: previewArea

        anchors.top: field.bottom
        anchors.left: field.left
        anchors.right: field.right
        anchors.topMargin: 8
        height: Math.ceil(previewMetrics.lineSpacing) * 2
        clip: true

        Text {
            id: preview

            width: parent.width
            y: Math.min(0, parent.height - contentHeight)
            wrapMode: Text.Wrap
            color: window.errorText !== "" ? RosePine.love : window.noticeText !== "" ? RosePine.foam : field.color
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
                return backend.previewBullet(window.entryType, field.text);
            }
        }

        FontMetrics {
            id: previewMetrics
            font: preview.font
        }
    }

    ComboBox {
        id: frontBox

        readonly property string front: currentIndex >= 0 && currentIndex < count - 1 ? currentText : ""

        function selectDefault() {
            currentIndex = count > 1 ? Math.max(0, find(backend.defaultFront)) : -1;
        }

        anchors.top: previewArea.bottom
        anchors.left: field.left
        anchors.topMargin: 8
        width: (actionRow.x - field.x - 16) / 2
        model: backend.fronts.concat(["+ front"])
        focusPolicy: Qt.TabFocus
        background: InputFrame {
            focused: frontBox.visualFocus
            hovered: frontBox.hovered
        }
        enabled: backend.logExists
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
        readonly property string trackLabel: backend.phases.includes(phase) ? backend.trackLabel(frontBox.front, phase) : ""
        property string chosen: ""

        function selectDefault() {
            currentIndex = find(backend.defaultPhase);
        }

        anchors.top: frontBox.top
        anchors.left: frontBox.right
        anchors.leftMargin: 8
        width: frontBox.width
        model: backend.phases.concat(["+ phase"])
        focusPolicy: Qt.TabFocus
        background: InputFrame {
            focused: phaseBox.visualFocus
            hovered: phaseBox.hovered
        }
        displayText: currentIndex < 0 ? "no phase in progress" : trackLabel !== "" ? currentText + " › " + trackLabel : currentText
        contentItem: Text {
            leftPadding: 4 - phaseBox.padding
            rightPadding: 4 - phaseBox.padding
            topPadding: 4
            bottomPadding: 4
            text: phaseBox.displayText
            font: phaseBox.font
            color: phaseBox.palette.buttonText
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        palette.buttonText: enabled && currentIndex < 0 ? field.placeholderTextColor : undefined
        enabled: backend.logExists
        onPhaseChanged: {
            if (phase !== "") {
                chosen = phase;
            }
            statusRow.reset();
        }
        onActivated: index => {
            if (index === count - 1) {
                const previous = find(chosen);
                newPhasePopup.parentPhase = previous >= 0 ? chosen : "";
                newPhasePopup.open();
                if (previous >= 0) {
                    currentIndex = previous;
                } else {
                    selectDefault();
                }
            }
        }
    }

    Row {
        id: actionRow

        anchors.top: previewArea.bottom
        anchors.right: field.right
        anchors.topMargin: 8
        spacing: 8

        Row {
            spacing: 2

            Repeater {
                model: ["📌", "❓", "💡", "🐛"]

                EnterButton {
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

        EnterButton {
            id: addButton
            width: 48
            text: "+"
            onClicked: window.addEntry()
        }

        EnterButton {
            width: 48
            text: "↻"
            onClicked: {
                window.errorText = "";
                window.noticeText = "";
                backend.refresh();
            }
        }
    }

    NamePopup {
        id: newFrontPopup

        width: window.width - 24
        placeholderText: "New front name"
        acceptText: "Create"
        onNameAccepted: name => backend.addFront(name)
        onClosed: field.forceActiveFocus()
    }

    Row {
        id: statusRow

        readonly property string current: backend.pausedPhases.includes(phaseBox.phase) ? "Paused" : "In-Progress"
        property string target: "In-Progress"

        function reset() {
            target = current;
        }

        function verb(phaseStatus) {
            switch (phaseStatus) {
            case "In-Progress":
                return "Resume";
            case "Paused":
                return "Pause";
            case "Abandoned":
                return "Abandon";
            }
            return "Finish";
        }

        anchors.top: frontBox.bottom
        anchors.left: field.left
        anchors.topMargin: 8
        spacing: 2
        onCurrentChanged: reset()

        Repeater {
            model: ListModel {
                ListElement {
                    glyph: "▶️"
                    phaseStatus: "In-Progress"
                }
                ListElement {
                    glyph: "⏸️"
                    phaseStatus: "Paused"
                }
                ListElement {
                    glyph: "⏭️"
                    phaseStatus: "Done"
                }
                ListElement {
                    glyph: "⏹️"
                    phaseStatus: "Abandoned"
                }
            }

            EnterButton {
                required property string glyph
                required property string phaseStatus

                width: 30
                text: glyph
                highlighted: statusRow.target === phaseStatus
                enabled: backend.logExists && phaseBox.phase !== ""
                opacity: enabled ? 1 : 0.4
                onClicked: statusRow.target = phaseStatus
            }
        }
    }

    EnterButton {
        id: statusButton

        anchors.top: statusRow.top
        anchors.right: field.right
        width: 112
        text: (statusRow.target === statusRow.current ? "finish" : statusRow.verb(statusRow.target).toLowerCase()) + " phase"
        enabled: backend.logExists && phaseBox.phase !== "" && statusRow.target !== statusRow.current
        onClicked: {
            statusPopup.initialText = backend.phaseApproach(frontBox.front, phaseBox.phase);
            statusPopup.open();
        }
    }

    PhasePopup {
        id: newPhasePopup

        width: window.width - 24
        onPhaseAccepted: (name, approach) => backend.addPhase(frontBox.front, name, approach)
        onSubphaseAccepted: name => backend.startSubphase(frontBox.front, newPhasePopup.parentPhase, name)
        onOpenClicked: backend.openFront(frontBox.front)
        onClosed: field.forceActiveFocus()
    }

    NamePopup {
        id: statusPopup

        width: window.width - 24
        placeholderText: "Approach: what was done"
        acceptText: statusRow.verb(statusRow.target)
        onNameAccepted: approach => backend.setPhaseStatus(frontBox.front, phaseBox.phase, statusRow.target, approach)
        onClosed: field.forceActiveFocus()
    }

    FolderDialog {
        id: folderDialog
        title: "Choose the vault folder"
        onAccepted: {
            if (!backend.setVaultFolderUrl(selectedFolder)) {
                Qt.callLater(folderDialog.open);
            }
        }
    }
}
