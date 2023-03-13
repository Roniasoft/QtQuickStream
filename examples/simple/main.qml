import QtQuick
import QtQuick.Controls
import QtQuickStream
import QtQuick.Dialogs

Window {
    id: window
    width: 1280
    height: 960
    visible: true
    title: qsTr("Simple QtQuickStream Example")
    color: "#1e1e1e"


    //! 1. Create QSCore
    QSCore {
        id: core
        defaultRepo: createDefaultRepo([ "QtQuickStream" ]);
    }

    //! 2. Create QSObjects
    QSObject {
        id: mainObj

        property string qqsTestString: "QQSTest"
        property bool   qqsTestBool: false
        property int    qqsTestInt: 50
        property real   qqsTestReal: 15.25325

        _qsRepo: core.defaultRepo
    }

    QSObject {
        id: rootObj

        property string qqsTest: "QQSTestTwo"
        property bool   qqsTestBool: true
        property real   qqsTestReal: 15.2532585898


        _qsRepo: core.defaultRepo
    }

    //! 2. Save and load handlers
    Rectangle {
        anchors.centerIn: parent

        width: 400
        height: 40

        color: "#b0aeab"

        Button {
            text: "Save"

            width: 150
            anchors.left: parent.left
            anchors.margins: 20
            anchors.verticalCenter: parent.verticalCenter

            background: Rectangle {
                radius: 5
                color: "#6899e3"
            }

            onClicked: {
                core.defaultRepo.initRootObject("QSObject");
                saveDialog.visible = true
            }
        }

        Button {
            text: "Load"
            anchors.right: parent.right
            anchors.margins: 20
            anchors.verticalCenter: parent.verticalCenter

            width: 150
            background: Rectangle {
                radius: 5
                color: "#eb5e65"
            }
            onClicked: loadDialog.visible = true
        }
    }

    //Save
    FileDialog {
        id: saveDialog
        currentFile: "QtQuickStream.QQS.json"
        fileMode: FileDialog.SaveFile
        nameFilters: [ "QtQuickStream Files (*.QQS.json)" ]
        onAccepted: {
            core.defaultRepo.saveToFile(saveDialog.currentFile);
        }
    }

    //! Load
    FileDialog {
        id: loadDialog
        currentFile: "QtQuickStream.QQS.json"
        fileMode: FileDialog.OpenFile
        nameFilters: [ "QtQuickStream Files (*.QQS.json)" ]
        onAccepted: {
            core.defaultRepo.loadFromFile(loadDialog.currentFile);

            console.log("Repo qsUuid = ",core.defaultRepo.qsRootObject._qsUuid)
            console.log("Repo Objs:  ",core.defaultRepo._qsObjects)

        }
    }
}
