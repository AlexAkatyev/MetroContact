import QtQuick 2.5

Item
{
    id: inputIndicator
    objectName: "inputIndicator"
    property real input: 0
    onInputChanged: mess = input * convertation;
    property real convertation: 1
    onConvertationChanged: mess = input * convertation;
    property real mess: input * convertation
    property int accuracy: 0
    property real lowLimit: -50
    property real highLimit: 50
    property real zeroShift: 0
    property real unitPoint: 1 // Менять цену деления для перерисовки шкалы
    property string messUnit: "_____"
    property int accurDivision: 0 //количество отображаемых знаков после запятой на метках циферблата

    property bool blDetect1EnableInput: false
    property bool blDetect2EnableInput: false
    property bool blDetect1WorkInput: true
    property bool blDetect2WorkInput: true

    property bool blOverRange1: false  // выход датчика 1 за пределы таблицы калибровки
    property bool blOverRange2: false  // выход датчика 2 за пределы таблицы калибровки



    // Анимация запускается только при изменении зачения для показаний на шкале
    Behavior on mess {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

}
