#include <cmath>
#include <QVariant>
#include <QFile>
#include <QTextStream>

#include <QTimer>

#include "metrocontact.h"
#include "inisettings.h"

QString CACH_FILE = "cachMetroContact.csv";
int IVERT = 0;
int IHOR = 1;
int ISTEP = 2;


MetroContact::MetroContact(QObject *parent) : QObject(parent)
  , _settings(new IniSettings(this))
  , _finder(new MeasSystemsFinder(this))
  , _measurement(nullptr)
  , _cachFile(new QFile(CACH_FILE, this))
  , _currentV(0)
  , _currentH(0)
  , _currentS(0)
{
  _protokol.clear();
  connect(_finder,
          &MeasSystemsFinder::Ready,
          this,
          &MetroContact::measPick);
  measFind();

//  QTimer* timer = new QTimer(this);
//  connect(timer, &QTimer::timeout, this, &MetroContact::testReadMeas);
//  timer->start(50);

}


void MetroContact::measFind()
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
      _measurement->SetEnable(true);
    }
  }
}


void MetroContact::saveProtocol(QString urlName)
{
  QString fileName = urlName.remove(0, 8);
  if (fileName.isEmpty())
    return;

  _cachFile->close();
  QFile protokol(fileName, this);

  protokol.open(QIODevice::WriteOnly);
  saveProtokolHeader(&protokol);
  for (auto meas : _protokol)
    saveProtokolRecord(&protokol, meas);
  protokol.close();

  _protokol.clear();
  QFile::remove(CACH_FILE);
}


void MetroContact::saveMeasure(int picket, bool direction, float length, float v, float h)
{
  static float prevlen = -1;
  float rLen = round(4 * length)/4;
  float len = abs(rLen - length);
  if (prevlen != rLen
      && len < 0.15)
  {
    _protokol.push_back(Measure(picket, direction, rLen, v, h));
    if (!_cachFile->isOpen())
    {
      _cachFile->open(QIODevice::WriteOnly);
      saveProtokolHeader(_cachFile);
    }
    saveProtokolRecord(_cachFile, *(--_protokol.end()));
    _cachFile->flush();
  }
  prevlen = rLen;
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
  file->write(QString::number(measure.picket).toLatin1()
              + (measure.direct ? "+" : "-")
              + ";"
              + QString::number(measure.length, 'f', 3).toLatin1()
              + ";"
              + QString::number(measure.vert, 'f', 3).toLatin1()
              + ";"
              + QString::number(measure.horiz, 'f', 3).toLatin1()
              + ";\n");
}


void MetroContact::indicateCurrentMeas(std::vector<QVariant> meas)
{
  _currentV = meas.at(IVERT).toDouble();
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


void MetroContact::testReadMeas()
{
  static int v = 0;
  static int h = 0;
  static uint s = 0;
  v++;
  h++;
  s += 94;
  _currentV = v;
  _currentH = h;
  _currentS = s;
}



