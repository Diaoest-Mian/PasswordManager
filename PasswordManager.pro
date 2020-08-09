#-------------------------------------------------
#
# Project created by QtCreator 2019-12-25T22:30:17
#
#-------------------------------------------------

QT       += core gui
QT       += sql widgets
@
    android{
        QT += androidextras
    }
@
# MSVC force use utf-8
msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PasswordManager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qaesencryption.cpp

HEADERS += \
        mainwindow.h \
    qaesencryption.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = "logo.ico"

unix:!macx: LIBS += -L$$PWD/../build-QtCipherSqlitePlugin-Android_for_armeabi_v7a_arm64_v8a_x86_x86_64_Clang_Qt_5_14_0_android-Release/sqlitecipher/plugins/sqldrivers/ -lplugins_sqldrivers_sqlitecipher_armeabi-v7a
# 需自行编译安卓版本的sqlite加密插件
INCLUDEPATH += $$PWD/../build-QtCipherSqlitePlugin-Android_for_armeabi_v7a_arm64_v8a_x86_x86_64_Clang_Qt_5_14_0_android-Release/sqlitecipher/plugins/sqldrivers
DEPENDPATH += $$PWD/../build-QtCipherSqlitePlugin-Android_for_armeabi_v7a_arm64_v8a_x86_x86_64_Clang_Qt_5_14_0_android-Release/sqlitecipher/plugins/sqldrivers

DISTFILES += \
    android_resources/AndroidManifest.xml \
    android_resources/little_duck.png

# Android资源路径，比如manifest和图标
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android_resources

VERSION = 3.0
