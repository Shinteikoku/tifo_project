#include "src/main_window.hh"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    MainWindow window;
    window.showFullScreen();

    return app.exec();
}