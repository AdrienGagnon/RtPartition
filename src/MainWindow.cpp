#include "MainWindow.h"
#include "LomseViewWidget.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* widget = new LomseWidget(this);
    setCentralWidget(widget);
}
