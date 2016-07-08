// Copyright (c) 2016 Deepin Ltd. All rights reserved.
// Use of this source is governed by General Public License that can be found
// in the LICENSE file.

#ifndef DEEPIN_INSTALLER_APPLICATION_H
#define DEEPIN_INSTALLER_APPLICATION_H

#include <QApplication>

namespace service {
class SignalManager;
}  // namespace service

namespace ui {
class MainWindow;
}  // namespace ui

// Application is used as a container to holder global single instance of
// service objects and main window object. Both in main thread and background
// threads.
class Application : public QApplication {
  Q_OBJECT
 public:
  Application(int& argc, char** argv) : QApplication(argc, argv) { }
  // Destroy service objects in destructor.
  ~Application();

  service::SignalManager* signal_manager = nullptr;
  ui::MainWindow* main_window = nullptr;

  void initServiceObjects();
  void initMainWindow();
};

// Global instance.
extern Application* app;

#endif  // DEEPIN_INSTALLER_APPLICATION_H
