#ifndef USBMETROMEASSYSTEM_H
#define USBMETROMEASSYSTEM_H

#include <QSerialPort>
#include <QSerialPortInfo>

#include "meassystem.h"

class QTimer;

class UsbMetroMeasSystem : public MeasSystem
{
  Q_OBJECT
public:
  UsbMetroMeasSystem(QString portName, QObject* parent = nullptr);

  virtual bool SetEnable(bool enable) override;

  static QSerialPort::BaudRate BaudRate();
  static QSerialPort::Direction Direction();
  static QSerialPort::Parity Parity();
  static QSerialPort::DataBits DataBits();
  static QSerialPort::StopBits StopBits();
  static int BuferSize();
  static QSerialPort::FlowControl FlowControl();
  static int Period(); // ms
  static QByteArray Header();
  static int MessageLength();

protected:
  void readMeas();
  void sendINIT();

private:
  QSerialPortInfo _portInfo;
  QSerialPort* _port;
  QTimer* _logTimer;
  bool _saveToLog;
};

#endif // USBMETROMEASSYSTEM_H
