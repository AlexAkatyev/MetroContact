import QtQuick 2.5
import QtQuick.Controls 2.1

Item
{
    property real centerX: 0
    property real centerY: 0
    property real radiusDial: 0
    property real lengthMark: 10
    property real thicknessMark: 1
    property color colorMark: "black"
    property real angleMark: 0
    property bool visibleMark: true
    anchors.fill: parent
    function degToRad(degrees) {
        return degrees * (Math.PI / 180);
    }
    function getX()
    {
        var result;
        var a;
        a = angleMark - 90;
        result = 0;
        if (a >= -90)
        {
            result = centerX + radiusDial*Math.cos(degToRad(a)) - lengthMark;
        }
        else
        {
            result = centerX + radiusDial*Math.cos(degToRad(a));
        }
        return result;
    }
    function getY()
    {
        var result;
        var a;
        a = angleMark - 90;
        result = centerY + radiusDial*Math.sin(degToRad(a)) - thicknessMark/2;
        return result;
    }
    function getTransformOrigin(a)
    {
        if (a >= 0)
        {
                return Item.Right;
        }
        else
        {
                return Item.Left;
        }
    }
    function getRotation(a)
    {
        if (a >= 0)
        {
                return a - 90;
        }
        else
        {
                return a + 90;
        }
    }
    Rectangle
    {
        antialiasing: true
        transformOrigin: getTransformOrigin(angleMark)
        color: colorMark
        rotation: getRotation(angleMark)
        visible: visibleMark
        width: lengthMark
        height: thicknessMark
        x: getX()
        y: getY()
    }
}
