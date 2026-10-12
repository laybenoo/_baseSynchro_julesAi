import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LottoSynchro

ApplicationWindow {
    id: window
    width: 1100
    height: 750
    minimumWidth: 850
    minimumHeight: 600
    visible: true
    title: "Module de Synchronisation SQLite Lotto (Qt 6)"

    // Palette Palette Definition
    readonly property color colorPrimary: "#1e88e5"
    readonly property color colorPrimaryHover: "#1565c0"
    readonly property color colorAccent: "#ff9800"
    readonly property color colorBg: "#f4f6f9"
    readonly property color colorCardBg: "#ffffff"
    readonly property color colorTextPrimary: "#2c3e50"
    readonly property color colorTextSecondary: "#7f8c8d"

    readonly property color colorStatusGreen: "#2ecc71"
    readonly property color colorStatusOrange: "#e67e22"
    readonly property color colorStatusBlue: "#3498db"
    readonly property color colorStatusRed: "#e74c3c"

    background: Rectangle {
        color: window.colorBg
    }

    LottoModel {
        id: lottoModel
        onLogEmitted: (msg) => {
            var timeStr = Qt.formatDateTime(new Date(), "hh:mm:ss");
            logListModel.append({ "time": timeStr, "message": msg });
            if (autoScrollCheck.checked) {
                logListView.positionViewAtEnd();
            }
        }
    }

    ListModel {
        id: logListModel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // --- HEADER BAR ---
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 70
            color: window.colorCardBg
            radius: 10
            border.color: "#e0e0e0"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                spacing: 15

                ColumnLayout {
                    spacing: 2
                    Text {
                        text: "🎰 SQLite Lotto Sync"
                        font.pixelSize: 18
                        font.bold: true
                        color: window.colorTextPrimary
                    }
                    Text {
                        text: lottoModel.totalTablesCount + " tables synchronisables"
                        font.pixelSize: 12
                        color: window.colorTextSecondary
                    }
                }

                Item { Layout.fillWidth: true }

                // Domain Config Box
                RowLayout {
                    spacing: 8
                    Text {
                        text: "Domaine API:"
                        font.pixelSize: 13
                        font.bold: true
                        color: window.colorTextPrimary
                    }
                    TextField {
                        id: domainInput
                        implicitWidth: 260
                        text: lottoModel.domain
                        placeholderText: "https://votre-domaine.com"
                        selectByMouse: true
                        onEditingFinished: {
                            lottoModel.domain = text
                        }
                    }
                }

                // Batch Sync Button
                Button {
                    id: btnSyncAll
                    enabled: !lottoModel.isSyncing
                    implicitHeight: 42
                    implicitWidth: 190

                    contentItem: RowLayout {
                        spacing: 8
                        anchors.centerIn: parent
                        Text {
                            text: lottoModel.isSyncing ? "⌛ Synchronisation..." : "⚡ TOUT SYNCHRONISER"
                            font.bold: true
                            font.pixelSize: 13
                            color: "#ffffff"
                        }
                    }

                    background: Rectangle {
                        color: btnSyncAll.enabled ? (btnSyncAll.hovered ? window.colorPrimaryHover : window.colorPrimary) : "#bdc3c7"
                        radius: 8
                    }

                    onClicked: {
                        lottoModel.domain = domainInput.text;
                        lottoModel.syncAll();
                    }
                }
            }
        }

        // --- GLOBAL PROGRESS BAR ---
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 45
            color: window.colorCardBg
            radius: 8
            border.color: "#e0e0e0"
            visible: lottoModel.isSyncing || lottoModel.totalCount > 0

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                spacing: 12

                Text {
                    text: "Progression Globale:"
                    font.bold: true
                    font.pixelSize: 13
                    color: window.colorTextPrimary
                }

                ProgressBar {
                    id: progressBar
                    Layout.fillWidth: true
                    value: lottoModel.progressPercentage
                    background: Rectangle {
                        implicitHeight: 12
                        color: "#e0e0e0"
                        radius: 6
                    }
                    contentItem: Item {
                        implicitHeight: 12
                        Rectangle {
                            width: progressBar.visualPosition * parent.width
                            height: parent.height
                            radius: 6
                            color: window.colorPrimary
                        }
                    }
                }

                Text {
                    text: lottoModel.processedCount + " / " + lottoModel.totalCount + " tables (" + Math.round(lottoModel.progressPercentage * 100) + "%)"
                    font.pixelSize: 12
                    font.bold: true
                    color: window.colorTextPrimary
                }
            }
        }

        // --- FILTER BAR & STATS SUMMARY ---
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 50
            color: window.colorCardBg
            radius: 8
            border.color: "#e0e0e0"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10

                // Filter Country Radio Buttons / Chips
                RowLayout {
                    spacing: 5
                    property var countries: ["Tous", "Togo", "Ghana & National", "Côte d'Ivoire", "Niger", "Italie"]

                    Repeater {
                        model: parent.countries
                        delegate: Button {
                            property bool isSelected: lottoModel.filterCountry === modelData
                            implicitHeight: 32
                            contentItem: Text {
                                text: modelData
                                font.pixelSize: 12
                                font.bold: isSelected
                                color: isSelected ? "#ffffff" : window.colorTextPrimary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: isSelected ? window.colorPrimary : (parent.hovered ? "#eaeded" : "#f8f9f9")
                                border.color: isSelected ? window.colorPrimary : "#d5dbdb"
                                radius: 16
                            }
                            onClicked: lottoModel.filterCountry = modelData
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Search Box
                TextField {
                    id: searchInput
                    placeholderText: "🔍 Rechercher table..."
                    implicitWidth: 180
                    implicitHeight: 32
                    onTextChanged: lottoModel.searchFilter = text
                }
            }
        }

        // --- TABLE CARDS GRID / LIST ---
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridView {
                id: gridView
                anchors.fill: parent
                cellWidth: gridView.width > 900 ? gridView.width / 3 : (gridView.width > 600 ? gridView.width / 2 : gridView.width)
                cellHeight: 110
                model: lottoModel

                delegate: Rectangle {
                    width: gridView.cellWidth - 10
                    height: gridView.cellHeight - 10
                    color: window.colorCardBg
                    radius: 8
                    border.color: status === "Synchronisation..." ? window.colorStatusBlue :
                                  (status === "Erreur" ? window.colorStatusRed :
                                  (status === "Nouveaux tirages" ? window.colorStatusOrange : "#e0e0e0"))
                    border.width: status === "Synchronisation..." ? 2 : 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                spacing: 6
                                Text {
                                    text: tableName
                                    font.bold: true
                                    font.pixelSize: 15
                                    color: window.colorTextPrimary
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                Rectangle {
                                    implicitWidth: countryText.implicitWidth + 10
                                    implicitHeight: 18
                                    radius: 9
                                    color: "#f0f3f4"
                                    border.color: "#bdc3c7"
                                    Text {
                                        id: countryText
                                        anchors.centerIn: parent
                                        text: country
                                        font.pixelSize: 10
                                        color: window.colorTextSecondary
                                    }
                                }
                            }

                            Text {
                                text: "Dernier tirage (MAX): " + (maxDraw > 0 ? maxDraw : "Aucun (0)")
                                font.pixelSize: 12
                                color: window.colorTextSecondary
                            }

                            // Status Pill Badge
                            Rectangle {
                                implicitWidth: statusLabel.implicitWidth + 16
                                implicitHeight: 22
                                radius: 11
                                color: status === "À jour" ? window.colorStatusGreen :
                                       (status === "Nouveaux tirages" ? window.colorStatusOrange :
                                       (status === "Synchronisation..." ? window.colorStatusBlue : window.colorStatusRed))

                                Text {
                                    id: statusLabel
                                    anchors.centerIn: parent
                                    text: status === "Nouveaux tirages" ? ("+ " + newDrawsCount + " tirages") : status
                                    font.pixelSize: 11
                                    font.bold: true
                                    color: "#ffffff"
                                }
                            }
                        }

                        Button {
                            id: btnSyncItem
                            enabled: !lottoModel.isSyncing
                            implicitWidth: 40
                            implicitHeight: 40

                            contentItem: Text {
                                text: "🔄"
                                font.pixelSize: 16
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                color: btnSyncItem.hovered ? "#eaeded" : "#f8f9f9"
                                border.color: "#bdc3c7"
                                radius: 20
                            }

                            onClicked: {
                                lottoModel.syncTable(index);
                            }
                        }
                    }
                }
            }
        }

        // --- LOG CONSOLE PANEL ---
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: logHeader.height + (logContent.visible ? 150 : 0)
            color: "#1e1e1e"
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Log Header Bar
                Rectangle {
                    id: logHeader
                    Layout.fillWidth: true
                    implicitHeight: 36
                    color: "#2d2d2d"
                    radius: 8

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10

                        Text {
                            text: "📋 Logs de Synchronisation en temps réel (" + logListModel.count + " entrées)"
                            font.pixelSize: 12
                            font.bold: true
                            color: "#dcdcdc"
                        }

                        Item { Layout.fillWidth: true }

                        CheckBox {
                            id: autoScrollCheck
                            text: "Défilement auto"
                            checked: true
                            contentItem: Text {
                                text: autoScrollCheck.text
                                font.pixelSize: 11
                                color: "#aaaaaa"
                                anchors.left: autoScrollCheck.indicator.right
                                anchors.leftMargin: 4
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Button {
                            text: "Effacer"
                            implicitHeight: 24
                            contentItem: Text { text: "Effacer"; font.pixelSize: 11; color: "#ffffff" }
                            background: Rectangle { color: "#444444"; radius: 4 }
                            onClicked: logListModel.clear()
                        }

                        Button {
                            text: logContent.visible ? "▼ Masquer" : "▲ Afficher"
                            implicitHeight: 24
                            contentItem: Text { text: logContent.visible ? "Masquer" : "Afficher"; font.pixelSize: 11; color: "#ffffff" }
                            background: Rectangle { color: "#444444"; radius: 4 }
                            onClicked: logContent.visible = !logContent.visible
                        }
                    }
                }

                // Log Output Area
                Item {
                    id: logContent
                    Layout.fillWidth: true
                    implicitHeight: 150
                    visible: true

                    ListView {
                        id: logListView
                        anchors.fill: parent
                        anchors.margins: 8
                        clip: true
                        model: logListModel

                        delegate: RowLayout {
                            width: logListView.width
                            spacing: 8

                            Text {
                                text: "[" + model.time + "]"
                                font.pixelSize: 11
                                font.family: "Monospace"
                                color: "#7f8c8d"
                            }

                            Text {
                                text: model.message
                                font.pixelSize: 11
                                font.family: "Monospace"
                                color: model.message.indexOf("ERREUR") !== -1 ? "#e74c3c" :
                                       (model.message.indexOf("réussie") !== -1 ? "#2ecc71" : "#ecf0f1")
                                wrapMode: Text.Wrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }
    }
}
