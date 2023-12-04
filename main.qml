import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtMultimedia 5.5
import QtQuick.Dialogs 1.2

Window {
    id: mainWindow
//    flags: Qt.FramelessWindowHint |
//           Qt.WindowMinimizeButtonHint |
//           Qt.Window
    x: 0
    y: 0
    width: 1500//(2500 < Screen.desktopAvailableWidth) ? 1500 : Screen.desktopAvailableWidth
    height: 1000//Screen.desktopAvailableHeight
    visible: true
    title: "Катучий шаблон М-009 v1.2.1"

    onSceneGraphInitialized: showMaximized();

    property int  gaugeMargin: 10
    property string dPath: ""
    property real stepLength: 0.00075
    property int currentPicket: 0
    onCurrentPicketChanged: {
        tLength.text = getLengthDisplay();
    }
    property bool startDiscretSet : false
    property int numberMeasFromPicket: 0
    property int currentDiscret: 0
    onCurrentDiscretChanged: {
        if (!startDiscretSet)
        {
            startDiscret = currentDiscret;
            startDiscretSet = true;
        }
        if (measuring && !pause)
        {
            var len = currentDiscret - prevSavedDiscret;
            len = len > 0 ? len : -len;
            if (len > 0x7FFFFFFF)
                len = len - 0x0100000000;
            if (len >= numberMeasFromPicket*1000/3)
            {
                saveMeasure();
                numberMeasFromPicket++;
            }
        }
    }
    property int prevSavedDiscret: 0
    property int startDiscret: 0
    property real currentLength: getLength()
    onCurrentLengthChanged: {
        tLength.text = getLengthDisplay();
    }
    property bool directionForward: true
    onDirectionForwardChanged: {
        tLength.text = getLengthDisplay();
    }
    property bool pause: false
    property bool measuring: false
    onMeasuringChanged: {
        btRecord.text = getNameCommandRecord();
        itPicket.enabled = measuring;
        itPickCaption.enabled = !measuring;
        if (!measuring)
            saveMeasToFile();
        else
        {
            startDiscret = currentDiscret;
            saveMeasure();
        }
    }
    property bool boolBuzzerPlayV: getBuzzerPlayV()
    onBoolBuzzerPlayVChanged:
    {
        if (boolBuzzerPlayV)
        {
            timerBuzzerV.interval = getLongPlay(vCaption.longPlay);
            timerBuzzerV.start();
        }
    }
    property bool boolBuzzerPlayH: getBuzzerPlayH()
    onBoolBuzzerPlayHChanged:
    {
        if (boolBuzzerPlayH)
        {
            timerBuzzerH.interval = getLongPlay(hCaption.longPlay);
            timerBuzzerH.start();
        }
    }

    function getLength() {
        var result = (currentDiscret - startDiscret) * stepLength;
        return result > 0 ? result : -result;
    }

    function getGaugeSize(){
        var h = itGauge.height - gaugeMargin - txVertical.font.pixelSize * 2 - itPicket.height;
        var w = (itGauge.width - 2 * gaugeMargin) / 2;
        return h > w ? w : h;
    }

    function getPicketHeigth(){
        var hscreen = Screen.desktopAvailableHeight / 10;
        var hmin = 30;
        return hscreen > hmin ? hscreen : hmin;
    }

    function getLengthDisplay(){
        var result = "ПК " + currentPicket.toString();
        result += directionForward ? " + " : " - ";
        result += gaugeVertical.round10(currentLength, 1) + " м";
        return result;
    }

    function getNameCommandRecord() {
        return measuring ? "Остановить запись" : "Начать запись"
    }

    function saveMeasToFile() {
        fileDialog.open();
    }

    function saveMeasure() {
        mcRoutine.saveMeasure(currentPicket,
                              directionForward,
                              currentLength,
                              inputVertical.mess + inputVertical.zeroShift,
                              inputHorizontal.mess + inputHorizontal.zeroShift);
    }

    function getBuzzerPlayV()
    {
        return (((inputVertical.mess + inputVertical.zeroShift) > inputVertical.highLimit) |
                ((inputVertical.mess + inputVertical.zeroShift) < inputVertical.lowLimit)) ? vCaption.audioEnable : false;
    }

    function getBuzzerPlayH()
    {
        return (((inputHorizontal.mess + inputHorizontal.zeroShift) > inputHorizontal.highLimit) |
                ((inputHorizontal.mess + inputHorizontal.zeroShift) < inputHorizontal.lowLimit)) ? hCaption.audioEnable : false;
    }

    function getLongPlay(longPlay)
    {
        return longPlay ? 500 : 100;
    }

    Timer
    {
        id: timerBuzzerV
        repeat: false
        interval: getLongPlay(vCaption.longPlay)
        onRunningChanged:
        {
            if (running)
                auBuzzer.play();
            else
                auBuzzer.stop();
        }
    }

    Timer
    {
        id: timerBuzzerH
        repeat: false
        interval: getLongPlay(hCaption.longPlay)
        onRunningChanged:
        {
            if (running)
                auBuzzer.play();
            else
                auBuzzer.stop();
        }
    }


    FileDialog
    {
        id: fileDialog
        title: "Журнал измерений : Укажите имя файла"
        nameFilters: [ "CSV files (*.csv)" ]
        selectExisting: false
        selectFolder: false
        selectMultiple: false
        folder: shortcuts.home
        onAccepted: {
                mcRoutine.saveProtocol(fileDialog.fileUrl.toString());
            }
    }


    Audio
    {
        id: auBuzzer
        source: "qrc:/1kHz_44100Hz_16bit_05sec.wav"
    }


    Connections {
        target: mcRoutine
    }


    Timer
    {
        id: timerInd
        repeat: true
        running: true
        interval: 20
        onTriggered: {
            inputVertical.mess = mcRoutine.currentV();
            inputHorizontal.mess = mcRoutine.currentH();
            currentDiscret = mcRoutine.currentS();
            if (mcRoutine.dataReceived())
                receiveIndicator.color = "light yellow";
            else
                receiveIndicator.color = "black";
            if (mcRoutine.measKeeped())
            {
                keepIndicator.color = "blue";
                keepIndTimer.start();
            }
            inputVertical.blDetect1WorkInput = mcRoutine.workV();
            inputHorizontal.blDetect1WorkInput = mcRoutine.workH();
        }
    }


    Timer
    {
        id: keepIndTimer
        repeat: false
        running: false
        interval: 200
        onTriggered: keepIndicator.color = "white"
    }


    InputIndicator {
        id: inputVertical
        unitPoint: 0.4
        messUnit: "мм"
        accuracy: 1
        lowLimit: -6
        highLimit: 6
        blDetect1EnableInput: true
    }

    InputIndicator {
        id: inputHorizontal
        unitPoint: inputVertical.unitPoint
        messUnit: inputVertical.messUnit
        accuracy: inputVertical.accuracy
        lowLimit: -8
        highLimit: 8
        blDetect1EnableInput: true
    }

    TabBar {
        id: tbMenu
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Repeater
        {
            model: ["ИНДИКАТОР",
                    "НАСТРОЙКА",
                    "ОПИСАНИЕ"]
            TabButton
            {
                text: modelData
                ToolTip.visible: hovered
                ToolTip.text: text
            }
        }
    }

    StackLayout {
        id: slTabs
        anchors.top: tbMenu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: tbMenu.currentIndex

        // Непосредственно индикатор
        Item {
            id: itGauge
            Item {
                id: itPicket
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                height: getPicketHeigth()
                enabled: measuring
                Text {
                    id: tLabel
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    text: "S:"
                    font.pixelSize: parent.height - 2
                }
                Text {
                    id: tLength
                    anchors.left: tLabel.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    text: getLengthDisplay()
                    font.pixelSize: tLabel.font.pixelSize
                }
                Button {
                    id: btPicket
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    text: "ПИКЕТ"
                    font.pixelSize: tLabel.font.pixelSize
                    onClicked:{
                        currentPicket += directionForward ? 1 : -1;
                        numberMeasFromPicket = 0;
                        prevSavedDiscret = currentDiscret;
                        startDiscret = currentDiscret;
                    }
                }
                Button {
                    id:btPause
                    anchors.right: btPicket.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.rightMargin: 5
                    font.pixelSize: pause ? tLabel.font.pixelSize/2.25 : tLabel.font.pixelSize
                    text: pause ? "ПРОДОЛЖИТЬ" : "ПАУЗА"
                    onClicked: pause = !pause
                }
            }
            ImpGauge{
                id: gaugeVertical
                inputIndicator: inputVertical
                anchors.left: parent.left
                anchors.leftMargin: gaugeMargin
                anchors.top: parent.top
                anchors.topMargin: gaugeMargin
                width: getGaugeSize()
                height: getGaugeSize()
            }
            ImpGauge{
                id: gaugeHorizontal
                inputIndicator: inputHorizontal
                anchors.top: parent.top
                anchors.topMargin: gaugeMargin
                anchors.right: parent.right
                anchors.rightMargin: gaugeMargin
                width: getGaugeSize()
                height: getGaugeSize()
            }
            Text {
                id: txVertical
                text: "Вертикаль"
                font.pixelSize: 20
                anchors.top: gaugeVertical.bottom
                anchors.horizontalCenter: gaugeVertical.horizontalCenter
            }
            Text {
                id: txHorizontal
                text: "Горизонталь"
                font.pixelSize: txVertical.font.pixelSize
                anchors.top: gaugeHorizontal.bottom
                anchors.horizontalCenter: gaugeHorizontal.horizontalCenter
            }
            Rectangle {
                id: receiveIndicator
                color: "light yellow"
                anchors.top: parent.top
                anchors.topMargin: height/2
                anchors.horizontalCenter: parent.horizontalCenter
                height: 40
                width: height
                radius: height / 2
                border.color: "black"
                border.width: 2
                ToolTip
                {
                    id: ttIReceiveIndicator
                    text: "Связь с оборудованием"
                }
                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: ttIReceiveIndicator.visible = true
                    onExited:  ttIReceiveIndicator.visible = false

                }
            }
            Rectangle {
                id: recordIndicator
                color: measuring ? (pause ? "red" : "green") : "light grey"
                anchors.top: parent.top
                anchors.topMargin: receiveIndicator.anchors.topMargin
                anchors.right: receiveIndicator.left
                anchors.rightMargin: receiveIndicator.border.width
                height: receiveIndicator.height
                width: receiveIndicator.width
                radius: receiveIndicator.radius
                border.color: receiveIndicator.border.color
                border.width: receiveIndicator.border.width
                ToolTip
                {
                    id: ttIRecordIndicator
                    text: "Режим работы"
                }
                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: ttIRecordIndicator.visible = true
                    onExited:  ttIRecordIndicator.visible = false

                }
            }
            Rectangle {
                id: keepIndicator
                color: "white"
                anchors.top: parent.top
                anchors.topMargin: receiveIndicator.anchors.topMargin
                anchors.left: receiveIndicator.right
                anchors.leftMargin: receiveIndicator.border.width
                height: receiveIndicator.height
                width: receiveIndicator.width
                radius: receiveIndicator.radius
                border.color: receiveIndicator.border.color
                border.width: receiveIndicator.border.width
                ToolTip
                {
                    id: ttIKeepIndicator
                    text: "Запись показаний"
                }
                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: ttIKeepIndicator.visible = true
                    onExited:  ttIKeepIndicator.visible = false

                }
            }
        } // Item itGauge

        Item {
            id: itCaptions
            Button {
                id: btRecord
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: itPicket.height
                text: getNameCommandRecord()
                font.pixelSize: tLabel.font.pixelSize
                onClicked: measuring = !measuring;
            }
            Item {
                id: itPickCaption
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: btRecord.top
                height: itPicket.height
                Grid {
                    rows: 1
                    columns: 4
                    spacing: 10
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        id: txStartPicket
                        text: "Стартовый пикет:"
                        font.pixelSize: getPicketHeigth() / 4
                    }
                    TextField {
                        id: startPicket
                        placeholderText: "0"
                        validator: IntValidator{bottom: 0; top: 999;}
                        inputMethodHints: Qt.ImhDigitsOnly
                        font.pixelSize: txStartPicket.font.pixelSize
                        onEditingFinished: {
                            var picket = text;
                            currentPicket = picket;
                            startDiscret = currentDiscret;
                        }
                    }
                    Text {
                        text: "Направление движения:"
                        font.pixelSize: txStartPicket.font.pixelSize
                    }
                    ComboBox {
                        model: ["Прямое", "Обратное"]
                        font.pixelSize: txStartPicket.font.pixelSize
                        width: parent.width / 5
                        onCurrentIndexChanged:
                        {
                            switch (currentIndex)
                            {
                            case 0:
                                directionForward = true;
                                break;
                            case 1:
                                directionForward = false;
                                break;
                            }
                        }
                        popup.font.pixelSize: txStartPicket.font.pixelSize
                    }
                }
            }
            GaugeCaption {
                id: vCaption
                anchors.top: parent.top
                anchors.bottom: itPickCaption.top
                anchors.left: parent.left
                width: parent.width / 2
                capName: "Настройка индикатора\n          вертикали"
                inputIndicator: inputVertical
            }
            GaugeCaption {
                id: hCaption
                anchors.top: parent.top
                anchors.bottom: itPickCaption.top
                anchors.right: parent.right
                anchors.left: vCaption.right
                capName: "Настройка индикатора\n         горизонтали"
                inputIndicator: inputHorizontal
            }
        } // Item itCaptions

        Item {
            id: itManual
            ScrollView {
                anchors.fill: parent
                clip: true
                TextArea {
                    readOnly: true
                    text:
"
                  Измеритель контакта М-009
                  Руководство пользователя

    1. Запуск датчика и программы
    Датчик начинает работать сразу после включения компьютера,
    поэтому сразу после запуска программы индицируются показания датчиков.
    Программу запускать следует только после подключения датчика к компьютеру.
    Программа отслеживает, чтобы был запущен только один экземпляр программы MetroContact.
    Запуск одновременно нескольких экземпляров программы не возможен!

    2. Начало измерения
    Для начала измерения, т.е. формирования журнала измерений с привязкой к пикетам,
    необходимо открыть вкладку НАСТРОЙКА, указать стартовый пикет, с которого начнется измерение.
    Указать направление движения:
    прямое - значит по ходу измерения номера пикетов будут увеличиваться,
    обратное - значит по ходу измерения номера пикетов будут уменьшаться.
    Также можно изменить предустановленные допуски для визуальной и звуковой сигнализации
    на индикаторах горизонтального и вертикального датчиков.
    Громкость звуковой сигнализации регулировать с помощью стандартных средств операционной системы.
    После нажмите кнопку НАЧАТЬ ЗАПИСЬ.

    3. Процесс измерения
    В процессе измерения на вкладке ИНДИКАТОР отображаются текущие показания вертикального и
    горизонтального датчиков, а также расстояние от последнего отмеченного пикета.
    Прохождение пикета отмечается нажатием кнопки ПИКЕТ.
    Если требуется повторить замеры без зенесения их в журнал измерений, то нажать
    кнопку ПАУЗА. Для продолжения измерений нажать кнопку ПРОДОЛЖИТЬ.

    В процессе измерения на экране доступна индикация работы оборудования.

    Внутри каждого стрелочного циферблата должна мигать индикация 1, это говорит о том,
    что показания вертикального/горизонтального смещения контактного рельса достоверны.
    Если мигание отстуствует, этио говорит о том, что данные с соответствующего датчика
    не поступают, либо не достоверны, т.е. имеет место отказ в работе соответствующего датчика.

    В середине, в верхней части окна программы отображаются три индикатора.
    Левый индикатор из группы сигнализирует о режиме работы программы. Никакой информации об
    отказах он не содержит.
    Центральный индикатор при нормальной работе программы и оборудования должен мигать.
    Если он не мигает, значит связь с измерительным оборудованием или не установлена, или
    данные с измерительного оборудования не поступают. Требуется выключить и включить
    измерительное оборудование, потом перезапустить программу.
    Правый индикатор загорается синим, если произведена запись в протокол. Запись должна
    производиться каждые 0,25 метров пути. Если индикатор загорается реже, значит требуется снизить
    скорость передвижения тележки.

    4. Окончание измерения
    По окончании измерения во вкладке НАСТРОКА нажать кнопку ОСТАНОВИТЬ ЗАПИСЬ.
    Программа предложит выбрать файл для сохранения измерений.

    5. Журнал измерений
    Журналы измерений сохраняются в соответствии с пунктом 4 в формате CSV.
    Этот формат открывается с помощью программы Microsoft Excel, в которой может
    быть подвергнут дополнительной обработке.
    Если программа неожиданно завершит свою работу, или возникнут ошибки в работе программы,
    то можно проанализировать работу программы и оборудования с помощью файла metrocontact.log.
    Этот файл располагается в рабочей папке программы, и перезаписывается после очередного
    запуска программы.
"
                }
            }
        } // Item itManual
    }

}
