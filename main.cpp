#include <QApplication>
#include <QCommandLineParser>
#include <QTimer>

#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addPositionalArgument("file", "File to open");
    parser.process(QCoreApplication::arguments());

    const QStringList files = parser.positionalArguments();

    MainWindow window;
    window.resize(1000, 700);
    window.show();

    QTimer::singleShot(10, [&window, files]() {
        window.processArguments(files);
    });
    // window.processArguments(files);

    return app.exec();
}
