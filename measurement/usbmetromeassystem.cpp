#include <QTimer>

// debug
//#include <QDebug>
//#include <QFile>
// debug

#include "usbmetromeassystem.h"

UsbMetroMeasSystem::UsbMetroMeasSystem(QString portName, QObject* parent)
  : MeasSystem(parent)
  , _portInfo(QSerialPortInfo(portName))
//  , _cachFile(new QFile("sensorLog.csv", this))
{
  _port = new QSerialPort(_portInfo, this);
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
      bool result = _port->open(QIODevice::ReadWrite); // ReadOnly
      if (result)
      {

        _port->setBaudRate(UsbMetroMeasSystem::BaudRate(), UsbMetroMeasSystem::Direction());
        _port->setParity(UsbMetroMeasSystem::Parity());
        _port->setDataBits(UsbMetroMeasSystem::DataBits());
        _port->setStopBits(UsbMetroMeasSystem::StopBits());
        _port->setReadBufferSize(UsbMetroMeasSystem::BuferSize());
        _port->setFlowControl(UsbMetroMeasSystem::FlowControl());

        _port->write("0");
        _port->flush();
        _port->waitForReadyRead();
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &UsbMetroMeasSystem::readMeas);
        timer->start(50);
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

  for (int i = input.indexOf(Header(), 0); // ищу очередное начало кадра
       i > -1
       && (i+MessageLength() <= input.size());
       i = input.indexOf(Header(), 0))
  {
    _result.clear();
    int v = (static_cast<unsigned char>(input.at(i + 4)) * 256) + static_cast<unsigned char>(input.at(i + 5));
    if (v > 0x7FFF)
      v -= 0x10000;
    _result.push_back(v); // вертикаль
    int h = (static_cast<unsigned char>(input.at(i + 6)) * 256) + static_cast<unsigned char>(input.at(i + 7));
    if (h > 0x7FFF)
      h -= 0x10000;
    _result.push_back(h); // горизонталь
    int s = (static_cast<unsigned char>(input.at(i + 8))) * 256 * 256 * 256;
    s += static_cast<unsigned char>(input.at(i + 9)) * 256 * 256;
    s += static_cast<unsigned char>(input.at(i + 10)) * 256;
    s += static_cast<unsigned char>(input.at(i + 11));
    if (s > 0x7FFFFFFF)
      s -= 0x100000000;
    _result.push_back(s); // дискреты пути

    // debug
//    qDebug() << "input " << input.toHex();
//    qDebug() << "v: " << v << " h: " << h << " s: " << s;
//    if (!_cachFile->isOpen())
//    {
//      _cachFile->open(QIODevice::WriteOnly);
//      _cachFile->write("Вертикаль;Горизонталь;Расстояние;\n");
//    }
//    _cachFile->write(QString::number(v, 16).toLatin1()
//                + ";"
//                + QString::number(h, 16).toLatin1()
//                + ";"
//                + QString::number(s, 16).toLatin1()
//                + ";\n");
//    _cachFile->flush();
    // debug

    emit sigCurrentMeas(_result); // отправляю результат в обработку
    input.remove(0, i + MessageLength()); // убираю из буфера прочитанный кадр
  }
}

