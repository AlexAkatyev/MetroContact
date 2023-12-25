#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class QFile;

class Logger : public QObject
{
  Q_OBJECT
public:
  static Logger* GetInstance(QObject *parent = nullptr);
  void WriteLog(QString input = "");
  void WriteLnLog(QString input = "");
  void WriteBytes(QByteArray input = "");
  void SetTimeLabel();
  static QString CurrentTimeToLog();

protected:
  explicit Logger(QObject *parent = nullptr);
  QFile* _log;
};

#endif // LOGGER_H
