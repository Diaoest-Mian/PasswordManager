#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void databaseInit();
    void terminate();
    void warningBox(QString);
    void infoBox(QString);
    void parameterInit();
    void generatePassword();
    char randomChar();
    void restartTimer();
    void updateInfo(QString);
    void showCount();
    bool confirm(QString);
    void show1stClass();
    QByteArray encrypt(QString, QByteArray);
    QString decrypt(QByteArray, QByteArray);

private slots:
    void search();
    void submit();
    void unlock();

    void on_length_valueChanged(int value);

    void on_letter_stateChanged(int arg1);

    void on_number_stateChanged(int arg1);

    void on_character_stateChanged(int arg1);

    void on_copy_clicked();

    void on_pwd4wt_textEdited(const QString &arg1);

    void on_how_textEdited(const QString &arg1);

    void on_urpwd_textEdited(const QString &arg1);

    void on_cbpwd4w_currentIndexChanged(const QString &arg1);

    void on_deleteBtn_clicked();

private:
    Ui::MainWindow *ui; // 以下4项分别是表名和3个字段名，尽量不要有空格，实在要空格的话后面语句用到时需加反引号
    QString table = "your_password_table_name";             // db数据库名
    QString colunm0 = "password_for_what";                  // 什么的密码，对应UI的"什么密码"
    QString colunm1 = "how_to_tell_the_difference";         // 对应UI的"怎么区分"
    QString colunm2 = "your_password";                      // 你的密码(加密存放)
    QString inputPwd;
    QString inputPwdMd5;
    QString key;

    QString sqlInsert;
    QString sqlSelect;
    QString sqlDelete;

    QByteArray hashKey;
    QByteArray hashIV;
    const QString IV = "your_IV_code"; // IV code, 可自定义

    QTimer *timer;//无操作定时锁定
    long time = 3 * 60;//单位秒
    int count = 0;
};

#endif // MAINWINDOW_H
