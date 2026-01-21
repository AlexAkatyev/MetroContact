#ifndef METROCONTACT_H
#define METROCONTACT_H

#include <QObject>

#include "measurement/meassystemsfinder.h"

class IniSettings;
class QFile;

struct Measure
{
  int picket = 0;
  bool direct = false;
  float length = 0;
  float vert = 0;
  float horiz = 0;
  Measure(int p, bool d, float l, float v, float h)
  {
    picket = p;
    direct = d;
    length = l;
    vert = v;
    horiz = h;
  }
};

class MetroContact : public QObject
{
  Q_OBJECT
public:
  explicit MetroContact(QObject *parent = nullptr);

  Q_INVOKABLE double currentV() const;
  Q_INVOKABLE double currentH() const;
  Q_INVOKABLE int currentS() const;
  Q_INVOKABLE bool dataReceived() const;
  Q_INVOKABLE bool measKeeped();
  Q_INVOKABLE bool workV();
  Q_INVOKABLE bool workH();
    Q_INVOKABLE QString activePort();

  void MeasFind();

public slots:
  void saveProtocol(QString urlName);
  void saveMeasure(int picket, bool direction, float length, float v, float h);
  void connectDevice();

private:
  void measPick();
  void saveProtokolHeader(QFile* file);
  void saveProtokolRecord(QFile* file, Measure measure);
  void indicateCurrentMeas(std::vector<QVariant> meas);

  IniSettings* _settings;

  MeasSystemsFinder* _finder;
  MeasSystem* _measurement;

  std::vector<Measure> _protokol;

  double _currentV;
  double _currentH;
  unsigned int _currentS;

  bool _dataReceivedFlipFlop;
  bool _dataReceivedState;
  bool _measKeeped;
  bool _workV;
  bool _workH;

  QTimer* _wdt;
};

#endif // METROCONTACT_H
