#include "meassystem.h"

MeasSystem::MeasSystem(QObject *parent)
  : QObject(parent)
{
  _result.clear();
}


std::vector<QVariant> MeasSystem::CurrentMeas()
{
  return _result;
}


QByteArray MeasSystem::GetStart()
{
  return "INIT";
}


QString MeasSystem::PortName()
{
    return "Порт не определён";
}
