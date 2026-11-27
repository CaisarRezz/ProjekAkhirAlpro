#include "MainWindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("FreshGuard"));

    QFont defaultFont(QStringLiteral("Segoe UI"), 10);
    app.setFont(defaultFont);

    MainWindow window;
    window.show();

    return app.exec();
}
