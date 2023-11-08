import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Item
{
    id: gaugeCaption
    property InputIndicator inputIndicator: null
    property string capName: "text"
    property bool audioEnable: false
    property bool longPlay: false
    Column {
        spacing: 5
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: " "
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: capName
        }
        Grid {
            anchors.horizontalCenter: parent.horizontalCenter
            rows: 3
            columns: 2
            spacing: 2
            Text {
                text: qsTr("\nВерхнее предельное отклонение   ")
            }
            TextField {
                id: tfHiLimit
                width: gaugeCaption.width/4
                text: inputIndicator.highLimit
                validator: IntValidator{bottom: -20; top: 20;}
                inputMethodHints: Qt.ImhDigitsOnly
                font.pixelSize: 12
                onEditingFinished: {
                    var hLimit = text;
                    inputIndicator.highLimit = hLimit;
                }
            }
            Text {
                text: qsTr("\nНижнее предельное отклонение   ")
            }
            TextField {
                id: tfLoLimit
                objectName: "tfLoLimit"
                width: tfHiLimit.width
                text: inputIndicator.lowLimit
                validator: IntValidator{bottom: -20; top: 20;}
                inputMethodHints: Qt.ImhDigitsOnly
                font.pixelSize: tfHiLimit.font.pixelSize
                onEditingFinished: {
                    var lLimit = text;
                    inputIndicator.lowLimit = lLimit;
                }
            }
            Text {
                text: qsTr("\nЗвуковое оповещение   ")
            }
            ComboBox {
                model: [qsTr("Выключено"), qsTr("Короткое"), qsTr("Длинное")]
                width: tfHiLimit.width
                onCurrentIndexChanged:
                {
                    switch (currentIndex)
                    {
                    case 0:
                        audioEnable = false;
                        break;
                    case 1:
                        longPlay = false;
                        audioEnable = true;
                        break;
                    case 2:
                        longPlay = true;
                        audioEnable = true;
                    }
                }
            }
        }
    }
}
