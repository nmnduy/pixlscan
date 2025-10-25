#include <QApplication>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Load and apply QDarkStyleSheet
    {
        const QString qssPath = QCoreApplication::applicationDirPath() + "/darkstyle.qss";
        QFile qssFile(qssPath);
        if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
            const QString style = QString::fromUtf8(qssFile.readAll());
            app.setStyleSheet(style);
        } else {
            qWarning() << "Could not load style sheet:" << qssPath;
        }
    }
    MainWindow w;
    w.show();
    return app.exec();
}
