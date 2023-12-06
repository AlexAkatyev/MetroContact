#include <cmath>
#include <QVariant>
#include <QFile>
#include <QTextStream>

#include <QTimer>

#include "metrocontact.h"
#include "inisettings.h"
#include "Logger/logger.h"

const int IWVERT = 0;
const int IVERT = 1;
const int IWHOR = 2;
const int IHOR = 3;
const int ISTEP = 4;


QString measureToString(Measure measure)
{
  return QString::number(measure.picket).toLatin1()
         + (measure.direct ? "+" : "-")
         + ";"
         + QString::number(measure.length, 'f', 2).toLatin1()
         + ";"
         + QString::number(measure.vert, 'f', 3).toLatin1()
         + ";"
         + QString::number(measure.horiz, 'f', 3).toLatin1()
         + ";";
}

MetroContact::MetroContact(QObject *parent) : QObject(parent)
  , _settings(new IniSettings(this))
  , _finder(new MeasSystemsFinder(this))
  , _measurement(nullptr)
  , _currentV(0)
  , _currentH(0)
  , _currentS(0)
  , _dataReceived(false)
  , _measKeeped(false)
  , _workV(false)
  , _workH(false)
{
  Logger::GetInstance(this)->WriteLnLog("Запуск программы MetroContact");
  _protokol.clear();
  connect(_finder,
          &MeasSystemsFinder::Ready,
          this,
          &MetroContact::measPick);
}


void MetroContact::MeasFind()
{
  _finder->Find();
}


void MetroContact::measPick()
{
  if (_measurement == nullptr)
  {
    std::vector<MeasSystem*> detects = _finder->MeasSystems();
    if (!detects.empty())
    {
      _measurement = *detects.begin();
      connect(_measurement, &MeasSystem::sigCurrentMeas, this, &MetroContact::indicateCurrentMeas);
      connect(_measurement, &MeasSystem::sigDataReceived, this, [=]()
      {
        _dataReceived = !_dataReceived;
      });
      _measurement->SetEnable(true);
    }
  }
}


void MetroContact::saveProtocol(QString urlName)
{
  QString fileName = urlName.remove(0, 8);
  if (fileName.isEmpty())
    return;

  QFile protokol(fileName, this);

  protokol.open(QIODevice::WriteOnly);
  saveProtokolHeader(&protokol);
  for (auto meas : _protokol)
    saveProtokolRecord(&protokol, meas);
  protokol.close();

  _protokol.clear();
}


void MetroContact::saveMeasure(int picket, bool direction, float length, float v, float h)
{
  static float prevPosition = -1;
  float currentPosition = round(4 * length)/4;
  if (prevPosition != currentPosition)
  {
    _protokol.push_back(Measure(picket, direction, currentPosition, v, h));
    _measKeeped = true;
    Logger::GetInstance()->SetTimeLabel();
    Logger::GetInstance()->WriteLnLog(measureToString(*(--_protokol.end())));
  }
  prevPosition = currentPosition;
}


void MetroContact::saveProtokolHeader(QFile* file)
{
  QString head = "Пикет;Расстояние, м;Вертикаль, мм;Горизонталь, мм;\n";
  QTextStream out(file);
  out.setCodec("Windows-1251");
  out << head;
}


void MetroContact::saveProtokolRecord(QFile* file, Measure measure)
{
  file->write(measureToString(measure).toLatin1()
              + "\n");
}


void MetroContact::indicateCurrentMeas(std::vector<QVariant> meas)
{
  _workV = meas.at(IWVERT).toBool();
  _currentV = meas.at(IVERT).toDouble();
  _workH = meas.at(IWHOR).toBool();
  _currentH = meas.at(IHOR).toDouble();
  _currentS = meas.at(ISTEP).toInt();
}


double MetroContact::currentV() const
{
  return _currentV;
}


double MetroContact::currentH() const
{
  return _currentH;
}


int MetroContact::currentS() const
{
  return _currentS;
}


bool MetroContact::dataReceived() const
{
  return _dataReceived;
}


bool MetroContact::measKeeped()
{
  bool result = _measKeeped;
  _measKeeped = false;
  return result;
}


bool MetroContact::workV()
{
  return _workV;
}


bool MetroContact::workH()
{
  return _workH;
}

