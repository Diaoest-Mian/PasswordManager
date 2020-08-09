#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec *code = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(code);
    //保存成带BOM的UTF-8，用cl编译器时，汉字本身是UTF-8编码，但程序内保存时却是对应的GBK编码。
    QApplication a(argc, argv);
    MainWindow w;

#ifdef Q_OS_ANDROID
    w.showMaximized();
#else
    QSharedMemory singleton(a.applicationName());
    if(!singleton.create(1))
    {
        QMessageBox::warning(nullptr, QStringLiteral("警告"), QStringLiteral("程序已经在运行了，同时运行多个会出问题的"));
        return 0;
    }
    w.show();
#endif

    return a.exec();
}
