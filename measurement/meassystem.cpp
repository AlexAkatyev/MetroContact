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

