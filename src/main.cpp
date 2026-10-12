#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Enable offscreen platform fallback if no display (for headless / test runner environments)
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM") && qEnvironmentVariableIsEmpty("DISPLAY")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
