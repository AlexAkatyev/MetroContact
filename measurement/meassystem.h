#ifndef MEASSYSTEM_H
#define MEASSYSTEM_H

#include <QObject>
#include <QVariant>

class MeasSystem : public QObject
{
  Q_OBJECT
public:
  explicit MeasSystem(QObject *parent = nullptr);
  virtual std::vector<QVariant> CurrentMeas();
  virtual bool SetEnable(bool enable) = 0;
    virtual QString PortName();
  static QByteArray GetStart();

signals:
  void sigCurrentMeas(std::vector<QVariant>);
  void sigDataReceived();

 protected:
  std::vector<QVariant> _result;

};

#endif // MEASSYSTEM_H
