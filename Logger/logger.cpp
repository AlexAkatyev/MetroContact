
#include <QFile>
#include <QDateTime>
#include <QTextStream>

#include "logger.h"


QString Logger::CurrentTimeToLog()
{
  return QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss");
}


Logger::Logger(QObject *parent)
  : QObject(parent)
  , _log(new QFile("metrocontact.log", parent))
{
  _log->open(QIODevice::WriteOnly);
  WriteLnLog("Начало записи " + CurrentTimeToLog());
}


Logger* Logger::GetInstance(QObject *parent)
{
  static Logger* logger = nullptr;
  if (!logger && parent)
    logger = new Logger(parent);
  return logger;
}


void Logger::WriteLog(QString input)
{
  if(!_log->isOpen())
    return;

  QTextStream out(_log);
  out.setCodec("Windows-1251");
  out << input;
  _log->flush();
}


void Logger::WriteLnLog(QString input)
{
  WriteLog(input + "\n");
}


void Logger::WriteBytes(QByteArray input)
{
  QString strCode;
  for (auto& c : input)
  {
    int i = c;
    i &= 0x00000000000000FF;
    QString s = QString::number(i, 16);
    if (s.size() == 1)
      s.prepend("0");
    strCode.push_back(" " + s);
  }
  WriteLnLog(strCode);
}


void Logger::SetTimeLabel()
{
  WriteLnLog("Время " + CurrentTimeToLog());
}
