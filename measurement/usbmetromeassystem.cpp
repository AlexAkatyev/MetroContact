#include <QTimer>
#include <QFile>

#include "usbmetromeassystem.h"
#include "Logger/logger.h"

const int WATCH_DOG_PERIOD = 500;
const int BAD_MEASURE = 0x0ABC;
const int INIT_PERIOD = 5000;


UsbMetroMeasSystem::UsbMetroMeasSystem(QString portName, QObject* parent)
  : MeasSystem(parent)
  , _portInfo(QSerialPortInfo(portName))
  , _logTimer(new QTimer(this))
  , _saveToLog(true)
{
  _port = new QSerialPort(_portInfo, this);
  connect(_logTimer, &QTimer::timeout, this, [=]()
  {
    if (_saveToLog)
      Logger::GetInstance()->WriteLnLog(QString("За последние %1 мс данные в порт не приходили").arg(WATCH_DOG_PERIOD));
    _saveToLog = true;
  });
  _logTimer->start(WATCH_DOG_PERIOD);

  QTimer* initSender = new QTimer(this);
  connect(initSender, &QTimer::timeout, this, &UsbMetroMeasSystem::sendINIT);
  initSender->start(INIT_PERIOD);
}


void UsbMetroMeasSystem::sendINIT()
{
  if (_port
      && _port->isOpen())
  {
    QByteArray data = MeasSystem::GetStart();
    _port->write(data);
    Logger::GetInstance()->WriteLog("Send: ");
    Logger::GetInstance()->WriteBytes(data);
  }
}


QSerialPort::BaudRate UsbMetroMeasSystem::BaudRate()
{
  return QSerialPort::Baud38400;
}


QSerialPort::Direction UsbMetroMeasSystem::Direction()
{
  return QSerialPort::AllDirections;
}


QSerialPort::Parity UsbMetroMeasSystem::Parity()
{
  return QSerialPort::NoParity;
}


QSerialPort::DataBits UsbMetroMeasSystem::DataBits()
{
  return QSerialPort::Data8;
}


QSerialPort::StopBits UsbMetroMeasSystem::StopBits()
{
  return QSerialPort::OneStop;
}


int UsbMetroMeasSystem::BuferSize()
{
  return 0;
}


QSerialPort::FlowControl UsbMetroMeasSystem::FlowControl()
{
  return QSerialPort::NoFlowControl;
}


int UsbMetroMeasSystem::Period()
{
  return 50;
}

QByteArray UsbMetroMeasSystem::Header()
{
  return "RAIL";
}


int UsbMetroMeasSystem::MessageLength()
{
  return 12;
}


bool UsbMetroMeasSystem::SetEnable(bool enable)
{

  if (enable)
  {
    if (!_port->isOpen())
    {
      bool result = _port->open(QIODevice::ReadWrite);
      if (result)
      {

        _port->setBaudRate(UsbMetroMeasSystem::BaudRate(), UsbMetroMeasSystem::Direction());
        _port->setParity(UsbMetroMeasSystem::Parity());
        _port->setDataBits(UsbMetroMeasSystem::DataBits());
        _port->setStopBits(UsbMetroMeasSystem::StopBits());
        _port->setReadBufferSize(UsbMetroMeasSystem::BuferSize());
        _port->setFlowControl(UsbMetroMeasSystem::FlowControl());

        sendINIT();
        _port->flush();
        connect(_port, &QSerialPort::readyRead, this, &UsbMetroMeasSystem::readMeas);
      }
      return result;
    }
    else
      return true;
  }
  else
  {
    _port->close();
    return true;
  }
}


void UsbMetroMeasSystem::readMeas()
{
  static QByteArray input;
  input.push_back(_port->readAll()); // читаю всё, что пришло в порт
  if (_saveToLog)
  {
    Logger::GetInstance()->WriteLog("Пришло в порт ");
    Logger::GetInstance()->WriteBytes(input);
  }

  for (int i = input.indexOf(Header(), 0); // ищу очередное начало кадра
       i > -1
       && (i+MessageLength() <= input.size());
       i = input.indexOf(Header(), 0))
  {
    _result.clear();
    int v = (static_cast<unsigned char>(input.at(i + 4)) * 256) + static_cast<unsigned char>(input.at(i + 5));
    if (v > 0x7FFF)
      v -= 0x10000;
    _result.push_back(v != BAD_MEASURE); // вертикаль истинна
    _result.push_back(static_cast<double>(v) / 100); // вертикаль
    int h = (static_cast<unsigned char>(input.at(i + 6)) * 256) + static_cast<unsigned char>(input.at(i + 7));
    if (h > 0x7FFF)
      h -= 0x10000;
    _result.push_back(h != BAD_MEASURE); // горизонталь истинна
    _result.push_back(static_cast<double>(h) / 100); // горизонталь
    int s = (static_cast<unsigned char>(input.at(i + 8))) * 256 * 256 * 256;
    s += static_cast<unsigned char>(input.at(i + 9)) * 256 * 256;
    s += static_cast<unsigned char>(input.at(i + 10)) * 256;
    s += static_cast<unsigned char>(input.at(i + 11));
    if (s > 0x7FFFFFFF)
      s -= 0x100000000;
    _result.push_back(s); // дискреты пути

    if (_saveToLog)
    {
      Logger::GetInstance()->WriteLnLog(QString::number(v, 10).toLatin1()
                                        + ";"
                                        + QString::number(h, 10).toLatin1()
                                        + ";"
                                        + QString::number(s, 10).toLatin1()
                                        + ";");
      _saveToLog = !_saveToLog;
      emit sigDataReceived();
    }
    emit sigCurrentMeas(_result); // отправляю результат в обработку
    input.remove(0, i + MessageLength()); // убираю из буфера прочитанный кадр
  }
}


QString UsbMetroMeasSystem::PortName()
{
    return _portInfo.portName();
}
