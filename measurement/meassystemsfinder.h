#ifndef MEASSYSTEMSFINDER_H
#define MEASSYSTEMSFINDER_H

#include <QObject>

#include "usbmetromeassystem.h"

class QSerialPort;
class QByteArray;
class QFile; // debug !!!!!!!!!!!!!

class MeasSystemsFinder : public QObject
{
  Q_OBJECT
public:
  explicit MeasSystemsFinder(QObject *parent = nullptr);
  void Find();

  std::vector<MeasSystem*> MeasSystems();

signals:
  void Ready();

private:
  void startAnalizePort();
  void finishAnalizePort();
  void recordLog(QString);

  QObject* _parent;
  std::vector<MeasSystem*> _measSystems;
  std::map<QString, QSerialPort*> _mapUsbMetroMeasSystem;
  std::map<QString, QSerialPort*>::iterator _itMMS;
  QFile* _cachFile; // debug !!!!!!!
};

#endif // MEASSYSTEMSFINDER_H
