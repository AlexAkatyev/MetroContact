#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>

#include <QSystemSemaphore>
#include <QSharedMemory>

#include <QQmlContext>

#include "metrocontact.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QQuickStyle::setStyle("Material"); //Flat Material

  QGuiApplication app(argc, argv);
  app.setOrganizationName("micromech");
  app.setOrganizationDomain("micromech");
  app.setApplicationName("metrocontact");

  // Проверка на наличие запущенных программ, работающих с измерителями микроперемещений

  QSystemSemaphore semaphore("<metrocontact.micromech>", 1);  // создаём семафор
  semaphore.acquire(); // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью

#ifndef Q_OS_WIN32
  // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
  // поэтому необходимо избавиться от данного мусора
  QSharedMemory nix_fix_shared_memory("<memory_metrocontact.micromech>");
  if(nix_fix_shared_memory.attach()){
      nix_fix_shared_memory.detach();
  }
#endif

  QSharedMemory sharedMemory("<memory_metrocontact.micromech>");  // Создаём экземпляр разделяемой памяти
  bool is_running;            // переменную для проверки уже запущенного приложения
  if (sharedMemory.attach())
  { // пытаемся присоединить экземпляр разделяемой памяти
                              // к уже существующему сегменту
      is_running = true;      // Если успешно, то определяем, что уже есть запущенный экземпляр
  }
  else
  {
      sharedMemory.create(1); // В противном случае выделяем 1 байт памяти
      is_running = false;     // И определяем, что других экземпляров не запущено
  }
  semaphore.release();        // Опускаем семафор
  // Если уже запущен один экземпляр приложения,
  // то завершаем работу текущего экземпляра приложения
  if(is_running){
      return 1; // Программу не запускаем
  }

  app.setWindowIcon(QIcon(":ruler_84678.ico"));

  QQmlApplicationEngine engine;
  const QUrl url(QStringLiteral("qrc:/main.qml"));
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                   &app, [url](QObject *obj, const QUrl &objUrl) {
    if (!obj && url == objUrl)
      QCoreApplication::exit(-1);
  }, Qt::QueuedConnection);

  MetroContact mcroutine;

  QQmlContext* context = engine.rootContext();    // Создаём корневой контекст
      /* Загружаем объект в контекст для установки соединения,
       * а также определяем имя, по которому будет происходить соединение
       * */
  context->setContextProperty("mcRoutine", &mcroutine);

  engine.load(url);

  mcroutine.MeasFind();

  return app.exec();
}
