#include "logging.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "qapplication.h"
#include "serializers/abstractserializer.h"
#include "tools/selectobjectstool.h"
#include "tools/toolbox.h"
#include <QApplication>
#include <QDirIterator>
#include <QSettings>
#include <QVariant>
#include <iostream>

QString level = "debug";
QFile logfile;
bool print_long_message = true;

int main(int argc, char* argv[])
{
  QApplication qt_app(argc, argv);
  qt_app.setWindowIcon(QIcon(":/icons/omm_48.png"));

  omm::setup_logfile(logfile);
  qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    omm::handle_log(logfile, level, print_long_message, type, ctx, msg);
  });

  auto options = std::make_unique<omm::Options>(false,  // is_cli
                                                true  // have_opengl
  );
  omm::Application app(qt_app, std::move(options));

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  if (argc > 1) {
    app.scene.load_from(argv[1]);
  }

  app.scene.tool_box().set_active_tool(omm::SelectObjectsTool::TYPE);

  return qt_app.exec();
}
