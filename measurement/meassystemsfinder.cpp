#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTimer>

//debug
#include <QFile>
#include <QTextStream>
//debug

#include "meassystemsfinder.h"


MeasSystemsFinder::MeasSystemsFinder(QObject *parent)
  : QObject(parent)
  , _parent(parent)
  , _cachFile(new QFile("finderLog.txt", this))
{
  _measSystems.clear();
}


void MeasSystemsFinder::Find()
{
  _mapUsbMetroMeasSystem.clear();
  _measSystems.clear();
  QList<QSerialPortInfo> listPort = QSerialPortInfo::availablePorts();
  int lenListPort = listPort.size();
  if (lenListPort)
  {
    recordLog("Список найденных портов:");
    // UsbMetroMeasSystem
    for (QSerialPortInfo& info : listPort)
    {
      QSerialPort* spTest = new QSerialPort(info, this);

      _mapUsbMetroMeasSystem.insert(std::pair<QString, QSerialPort*>(info.portName(), spTest));
      recordLog(info.portName());
    }
  }
  _itMMS = _mapUsbMetroMeasSystem.begin();
  startAnalizePort();
}


void MeasSystemsFinder::startAnalizePort()
{
  if (_itMMS == _mapUsbMetroMeasSystem.end())  // not have used ports
  {
    recordLog("Поиск устройства окончен");
    emit Ready();
    return;
  }

  if ((*_itMMS).second->open(QIODevice::ReadWrite)) // ReadOnly
  {

    recordLog("Открываю порт " + (*_itMMS).second->portName());
    QSerialPort::SerialPortError error = (*_itMMS).second->error();
    recordLog("Ошибка порта: " + QString::number(error));

    (*_itMMS).second->setBaudRate(UsbMetroMeasSystem::BaudRate(), UsbMetroMeasSystem::Direction());
    (*_itMMS).second->setParity(UsbMetroMeasSystem::Parity());
    (*_itMMS).second->setDataBits(UsbMetroMeasSystem::DataBits());
    (*_itMMS).second->setStopBits(UsbMetroMeasSystem::StopBits());
    (*_itMMS).second->setReadBufferSize(UsbMetroMeasSystem::BuferSize());
    (*_itMMS).second->setFlowControl(UsbMetroMeasSystem::FlowControl());


    (*_itMMS).second->write("0");
    (*_itMMS).second->flush();
    (*_itMMS).second->waitForReadyRead(9000);
    QTimer::singleShot(3 * UsbMetroMeasSystem::Period() + 1000,
                       this,
                       &MeasSystemsFinder::finishAnalizePort);

  }
  else
  {
    ++_itMMS;
    startAnalizePort();
  }
}


void MeasSystemsFinder::finishAnalizePort()
{
  recordLog("После паузы считываю с порта");
  QByteArray receiveData = (*_itMMS).second->readAll();
  QSerialPort::SerialPortError error = (*_itMMS).second->error();
  recordLog("Ошибка порта: " + QString::number(error));
  QString str;
  str.append(receiveData);
  recordLog("Считано: " + str);
  if ((receiveData.size() >= UsbMetroMeasSystem::MessageLength())
      && receiveData.contains(UsbMetroMeasSystem::Header()))
  {
    recordLog("Устройство найдено: " + (*_itMMS).first);
    UsbMetroMeasSystem* detect = new UsbMetroMeasSystem((*_itMMS).first, _parent);
    _measSystems.push_back(detect);
  }
  else
    recordLog("Устройство не определено");
  (*_itMMS).second->close();
  (*_itMMS).second->deleteLater();

  ++_itMMS;
  startAnalizePort();
}


std::vector<MeasSystem*> MeasSystemsFinder::MeasSystems()
{
  return _measSystems;
}


void MeasSystemsFinder::recordLog(QString record)
{
  if (!_cachFile->isOpen())
  {
    _cachFile->open(QIODevice::WriteOnly);
  }
  QTextStream out(_cachFile);
  out.setCodec("Windows-1251");
  out << record << "\n";
  _cachFile->flush();
}

