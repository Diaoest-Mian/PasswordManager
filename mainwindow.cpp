#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qaesencryption.h"
#include <QCryptographicHash>
#include <QClipboard>
#include <QMessageBox>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QProcess>

#pragma execution_character_set("utf-8")
//执行字符集需要是UTF-8

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{//构造函数，窗口初始化
    ui->setupUi(this);
    timer = new QTimer(this);
    ui->pwd4wt->setHidden(true);
    ui->how->setHidden(true);
    setMinimumSize(200, 200);

    databaseInit();//初始化数据库，不存在则创建，要提示，存在则可以正常操作。

    connect(ui->search, &QPushButton::clicked, this, &MainWindow::search);
    connect(ui->saveoupdate, &QPushButton::clicked, this, &MainWindow::submit);
    connect(ui->generate, &QPushButton::clicked, this, &MainWindow::generatePassword);

    connect(timer, SIGNAL(timeout()), this, SLOT(unlock()));
}

void MainWindow::search()
{// 查询
    // 查询前验证身份
    QString c0 = ui->cbpwd4w->currentText();
    QString c1 = ui->cbhow->currentText();
    unlock();
    QByteArray pwd4wt = encrypt(c0, hashKey);
    QByteArray how = encrypt(c1, hashKey);
    QSqlQuery query;
    query.prepare(sqlSelect);
    query.bindValue(0, pwd4wt);
    query.bindValue(1, how);
    query.exec();
    if(query.next())
    {
        ui->urpwd->setText(decrypt(query.value(colunm2).toByteArray(), hashKey));
    }
    else
    {
        ui->urpwd->setText("哎呀呀，你没告诉过这个密码呀~");
    }
    restartTimer();
}

void MainWindow::submit()
{
    if((ui->cbpwd4w->currentText().compare("MASTER9981") == 0) 
        && (ui->cbhow->currentText().compare("MASTER9981") == 0))
        // 更新主密码的关键字 可以自己修改
    {
        //更新主密码，更新前验证身份, 更新时需同步更新所有数据
        if(confirm("确定更新主密码？"))
        {
            QString newMaster = ui->urpwd->text();
            unlock();
            parameterInit();
            updateInfo(newMaster);
            restartTimer();
            return;
        }
    }

    QSqlQuery query;
    query.prepare(sqlInsert);
    query.bindValue(0, encrypt(ui->cbpwd4w->currentText(), hashKey));
    query.bindValue(1, encrypt(ui->cbhow->currentText(), hashKey));
    query.bindValue(2, encrypt(ui->urpwd->text(), hashKey));

    if(query.exec())
    {
        ui->urpwd->setText("^_^ ヾ(≧▽≦*)o (p≧w≦q) QAQ");
        ui->how->setText("(●'◡'●)");
        infoBox("嗯，记住了~");
        count++;
        showCount();
        show1stClass();
    }
    else
    {
        if (confirm("emmmm这个密码已经记过了，要更新吗?"))
        {
            QString sqlUpdate = "update " + table + " set " + colunm2 + " = ? where " + colunm0 + " = ? and " + colunm1 + " = ?";
            QSqlQuery query;
            query.prepare(sqlUpdate);
            query.bindValue(0, encrypt(ui->urpwd->text(), hashKey));
            query.bindValue(1, encrypt(ui->cbpwd4w->currentText(), hashKey));
            query.bindValue(2, encrypt(ui->cbhow->currentText(), hashKey));

            if(query.exec())
            {
                ui->urpwd->setText("^_^ ヾ(≧▽≦*)o (p≧w≦q) QAQ");
                ui->how->setText("(●'◡'●)");
                infoBox("好的，记住了~");
            }
            else
            {
                infoBox("emmmm更新好像出了什么问题...");
            }
        }
    }
    restartTimer();
}

void MainWindow::updateInfo(QString input)
{// 更新主密码后需更新所有数据
    QString oldMaster = inputPwd;
    QString newMaster = input;
    QString oldKey = key;
    QString newKey;
    QByteArray oldHashKey = hashKey;
    QByteArray newHashKey;


    QByteArray byte_array;
    byte_array.append(newMaster.append("end"));
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(byte_array);
    newKey = hash.result().toHex();
    newHashKey = QCryptographicHash::hash(newKey.toLocal8Bit(), QCryptographicHash::Sha256);

    QSqlQuery queryAll;// 取出除主密码外的所有数据
    queryAll.exec("select * from " + table + " where "
                  + colunm0 + " <> 'column0 for your master password' and "
                  + colunm1 + " <> 'column1 for your master password'");
    QSqlQuery queryCount;
    queryCount.exec("select count(*) from " + table);
    queryCount.next();
    int count = queryCount.value(0).toInt() - 1;
    int i = 0;
    while(queryAll.next())
    {// 对取出的每条数据进行重新加密
        QString c0 = decrypt(queryAll.value(colunm0).toByteArray(), hashKey);
        QString c1 = decrypt(queryAll.value(colunm1).toByteArray(), hashKey);
        QString c2 = decrypt(queryAll.value(colunm2).toByteArray(), hashKey);//旧hashkey解密

        QString sqlUpdate = "update " + table + " set " + colunm2 + " = ?, " + colunm1 + " = ?, "
                + colunm0 + " = ? where " + colunm0 + " = ? and " + colunm1 + " = ?";
        QSqlQuery query;
        query.prepare(sqlUpdate);
        query.bindValue(0, encrypt(c2, newHashKey));
        query.bindValue(1, encrypt(c1, newHashKey));
        query.bindValue(2, encrypt(c0, newHashKey));

        query.bindValue(3, encrypt(c0, hashKey));
        query.bindValue(4, encrypt(c1, hashKey));

        // if(query.exec())
        // {
        //     ui->info->setText("第" + QString::number(i + 1) + "条记录更新完毕");
        //     i++;
        // }
    }

    if(count == i)
    {
        infoBox("所有记录更新完毕, 共" + QString::number(i) + "条");
        showCount();
    }

    QByteArray byteArray;
    byteArray.append(newMaster);
    QCryptographicHash mhash(QCryptographicHash::Md5);
    mhash.addData(byteArray);  //添加数据到加密哈希值
    QString newMasterMd5 = mhash.result().toHex();

    QSqlQuery queryUM;// 更新存储的主密码
    queryUM.prepare("update " + table + " set " + colunm2 + " = ? where " + colunm0 + " = ? and " + colunm1 + " = ?");
    queryUM.bindValue(0, newMasterMd5);
    queryUM.bindValue(1, "column0 for your master password");
    queryUM.bindValue(2, "column1 for your master password");
    queryUM.exec();
    hashKey = newHashKey;
    key = newKey;

    ui->pwd4wt->clear();
    ui->how->clear();
    ui->urpwd->clear();
}

QByteArray MainWindow::encrypt(QString text, QByteArray hashKey)
{// 加密函数  // 和下面的解密函数好像是直接从网上copy的，可根据需要替换成其他加密算法
// 入参是待加密字符串和密钥
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);


    QByteArray encodeText = encryption.encode(text.toLocal8Bit(), hashKey, hashIV);

    return encodeText; // 返回参数是加密后的字符串
}

QString MainWindow::decrypt(QByteArray encodeText, QByteArray hashKey)
{// 解密函数
// 入参是待解密字符串和密钥
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray decodeText = encryption.decode(encodeText, hashKey, hashIV);

    QString decodedString = QString::fromLocal8Bit(encryption.removePadding(decodeText));
    
    return decodedString;
}

bool MainWindow::confirm(QString info)
{// 提示窗口，单独写个函数方便调用
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "提示",
                                    info,
                                    QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        return true;
    }
    return false;
}

void MainWindow::databaseInit()
{//初始化数据库
    //这个是db数据库插件，可以选择其他有加密功能的插件，或者用官方的不加密插件也可以:)
    QSqlDatabase db = QSqlDatabase::addDatabase("SQLITECIPHER");//sqlite不支持加密所以换成了sqlitecipher
    //db.setDatabaseName(":memory:");
#ifdef Q_OS_ANDROID
    //需要读写外部存储权限
    QStringList sysEnv = QProcess::systemEnvironment();
    QString filePath = sysEnv.filter(QRegularExpression("EXTERNAL_STORAGE")).replaceInStrings("EXTERNAL_STORAGE=", "").at(0) + "/Android/pwd.db";
    db.setDatabaseName(filePath);//use EXTERNAL_STORAGE/Android, need permission
#else
    db.setDatabaseName("pwd.db");//需要文件来长期存储pwd.db, 文件名与路径有需要可自行修改
#endif
    db.setPassword("your password for .db file"); // 连接db数据库的密码
    //db.setPassword("123");

    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "This example needs SQLite support. Please read "
                        "the Qt SQL driver documentation for information how "
                        "to build it.\nIf this is on Android Platform, "
                        "please grant READ_EXTERNAL_STORAGE permission.\n\n"
                        "Click Cancel to exit."), QMessageBox::Cancel);
        warningBox("出现异常，程序终止");
        terminate();
    }
    else
    {//这里判断是否存在目标表，存在则可以正常操作，否则需要创建表
        QSqlQuery query;
        Qt::WindowFlags windowFlags = Qt::MSWindowsFixedSizeDialogHint;
        QString sqlCmd;
        bool ok;
        // 拼装一下sql模板
        sqlInsert = "INSERT INTO " + table + " (" + colunm0 + ", " + colunm1 + ", " + colunm2 +
                ") VALUES (?, ?, ?)";
        sqlSelect = "SELECT " + colunm2 + " FROM " + table + " where " + colunm0 + " = ? and " + colunm1 + " = ?";
        sqlDelete = "DELETE FROM " + table + " where " + colunm0 + " = ? and " + colunm1 + " = ?";

        sqlCmd = "select count(*) from sqlite_master where type = 'table' and name = '" + table + "'";
        query.exec(sqlCmd);
        if(query.next())//移动到上一个exec的查询结果
        {
            if(query.value(0).toInt() == 0)//不存在目标表
            {
                sqlCmd = "create table " + table + " (" + colunm0 + " varchar(500) not null, "
                        + colunm1 + " varchar(500) not null, " + colunm2 + " varchar(500) not null, primary key ("
                        + colunm0 + ", " + colunm1 + "))";
                query.exec(sqlCmd);
                QInputDialog inputDialog;

                inputPwd = inputDialog.getText(this, "初始化完成",
                                                     "请输入足够复杂的主密码", QLineEdit::Normal,
                                                         "请牢记此密码...", &ok,
                                                   windowFlags);
                if (ok && !inputPwd.isEmpty())
                {//shezhi主密码
                    QByteArray byte_array;
                    byte_array.append(inputPwd);
                    QCryptographicHash hash(QCryptographicHash::Md5);
                    hash.addData(byte_array);  //添加数据到加密哈希值
                    QByteArray result_byte_array = hash.result();  //返回最终的哈希值
                    inputPwdMd5 = result_byte_array.toHex();

                    query.prepare(sqlInsert);// 记录主密码
                    query.bindValue(0, "column0 for your master password");
                    query.bindValue(1, "column1 for your master password");
                    query.bindValue(2, inputPwdMd5);
                    query.exec();

                    parameterInit();
                    infoBox("主密码千万记住了，忘了就什么都找不回了！"
                                           "\n以及保管好生成的db文件"
                                           "\n前两个框输入MASTER9981可以更新主密码");//修改主密码的关键字可自定义
                    //这里再次弹窗提醒记住主密码和保存好数据文件
                }
                else
                {
                    inputPwd = "TERMINATE";
                    warningBox("出现异常，程序终止");
                    terminate();
                    //这里要终止程序
                }
            }
            else
            {
                unlock();
                parameterInit();
            }

        }
        else
        {
            warningBox("出现异常，程序终止");
            terminate();
        }
    }
}

void MainWindow::unlock()
{// 解锁函数，要锁定也是调用此函数
    //ui->urpwd->setText("^_^");
    this->close();
    QSqlQuery query;
    QString masterpwdMd5;
    Qt::WindowFlags windowFlags = Qt::MSWindowsFixedSizeDialogHint;
    bool ok;
    query.prepare(sqlSelect);
    query.bindValue(0, "column0 for your master password");
    query.bindValue(1, "column1 for your master password");
    query.exec();
    if(query.next())
    {
        masterpwdMd5 = query.value(colunm2).toString();
    }
    int errorCount = 0;
    inputPwd = "";

#ifdef Q_OS_ANDROID
    QString unlockTip = "";
#else
    QString unlockTip = "木有提示";
#endif

    while(errorCount < 3)
    {
        inputPwd = QInputDialog::getText(this, QString::number(errorCount + 1),
                                     "输入你设置的主密码", QLineEdit::Password,
                                     unlockTip, &ok, windowFlags);
        QByteArray byte_array;
        byte_array.append(inputPwd);
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(byte_array);  //添加数据到加密哈希值
        QByteArray result_byte_array = hash.result();  //返回最终的哈希值
        inputPwdMd5 = result_byte_array.toHex();
        if (inputPwdMd5.compare(masterpwdMd5) == 0)
        {
            //密码对上了
            this->show();
            restartTimer();
            break;
        }
        errorCount++;

    }
    if(errorCount >= 3)
    {
        inputPwd = "TERMINATE";
        warningBox("输错3次了, 程序终止");
        terminate();
    }
}

void MainWindow::parameterInit()
{// 参数初始化
    key = "1234567";
    QByteArray byte_array;
    byte_array.append(inputPwd.append("end"));  // 主密码经处理后作为AES加解密密钥，处理方式可自己发挥想象
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(byte_array);
    key = hash.result().toHex();
    hashKey = QCryptographicHash::hash(key.toLocal8Bit(), QCryptographicHash::Sha256);
    hashIV = QCryptographicHash::hash(IV.toLocal8Bit(), QCryptographicHash::Md5);

    ui->info->setText(decrypt(encrypt(">>>>>Test text<<<<<", hashKey), hashKey));

    QSqlQuery queryCount;
    queryCount.exec("select count(*) from " + table);
    queryCount.next();
    count = queryCount.value(0).toInt() - 1;
    showCount();

    show1stClass();
}

void MainWindow::showCount()
{// 展示存放的密码数量
    ui->info->setText("一共记录了" + QString::number(count) + "条密码");
}

void MainWindow::terminate()
{// 终止程序，出现问题或者密码输错3次调用
    //qDebug() << "exiting...";
    this->close();
    this->~MainWindow();
    exit(0);
}

void MainWindow::warningBox(QString text)
{
    QMessageBox::critical(this, "警告", text);
}

void MainWindow::infoBox(QString text)
{
    QMessageBox::information(this, "提示", text);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::generatePassword()
{//生成密码的函数
    int length = ui->length->value();
    QString password = "";
    while(length > 0)
    {
        password.append(randomChar());
        length--;
    }
    ui->urpwd->setText(password);
    restartTimer();
}

char MainWindow::randomChar()
{//生成一个随机字符
    int index = QRandomGenerator::global()->bounded(32, 126);
    bool needLetter = ui->letter->checkState(); //字母 ASCII 65-90 97-122
    bool needNumber = ui->number->checkState(); //数字 ASCII 48-57
    bool needCharac = ui->character->checkState(); //其他可显示字符 ASCII 32-47 58-64 91-96 123-126

    //不需要字母但生成的是字母
    bool flag0 = (needLetter == false) && ((index >= 64 && index <= 90) || (index >= 97 && index <= 122));
    //不需要数字但生成的是数字
    bool flag1 = (needNumber == false) && (index >= 48 && index <= 57);
    //不需要字符但生成的是字符
    bool flag2 = (needCharac == false)
            && ((index >= 32 && index <= 47) || (index >= 58 && index <= 64)
                || (index >= 91 && index <= 96) || (index >= 123 && index <= 126));
    while(flag0 || flag1 || flag2)
    {
        index = QRandomGenerator::global()->bounded(32, 126);
        flag0 = (needLetter == false) && ((index >= 64 && index <= 90) || (index >= 97 && index <= 122));
        flag1 = (needNumber == false) && (index >= 48 && index <= 57);
        flag2 = (needCharac == false)
                && ((index >= 32 && index <= 47) || (index >= 58 && index <= 64)
                    || (index >= 91 && index <= 96) || (index >= 123 && index <= 126));
    }

    return (char)index;
}

void MainWindow::on_length_valueChanged(int value)
{//slidebar的槽函数
    ui->slidebarLabel->setText("长度: " + QString::number(value));
    generatePassword(); //这里打开的话可以实现拖动slidebar时更新密码
    //restartTimer();
}

void MainWindow::on_letter_stateChanged(int)
{//checkbox的槽函数
    if((ui->number->checkState() == false) && (ui->character->checkState() == false))
    {
        ui->letter->setCheckState(Qt::Checked);
        restartTimer();
        return;
    }
    generatePassword();
}

void MainWindow::on_number_stateChanged(int)
{//checkbox的槽函数
    if((ui->letter->checkState() == false) && (ui->character->checkState() == false))
    {
        ui->number->setCheckState(Qt::Checked);
        restartTimer();
        return;
    }
    generatePassword();
}

void MainWindow::on_character_stateChanged(int)
{//checkbox的槽函数
    if((ui->letter->checkState() == false) && (ui->number->checkState() == false))
    {
        ui->character->setCheckState(Qt::Checked);
        restartTimer();
        return;
    }
    generatePassword();
}

void MainWindow::on_copy_clicked()
{//复制按钮的槽函数
    QClipboard *clipBoard = QGuiApplication::clipboard();
    clipBoard->setText(ui->urpwd->text());
    restartTimer();
}

void MainWindow::restartTimer()
{//重置定时器
    timer->stop();
    timer->start(time * 1000);
}

void MainWindow::on_pwd4wt_textEdited(const QString &arg1)
{
    restartTimer();
}

void MainWindow::on_how_textEdited(const QString &arg1)
{
    restartTimer();
}

void MainWindow::on_urpwd_textEdited(const QString &arg1)
{
    restartTimer();
}

void MainWindow::on_cbpwd4w_currentIndexChanged(const QString &arg1)
{//这里,正常查询时要让cbhow一起变动，但新增时又不需要
    for(int i = 0; i < ui->cbpwd4w->count(); i++)
    {
        if(arg1.compare(ui->cbpwd4w->itemText(i)) == 0)
        {//comboBox的内容不是新增
            //在这里查询次选项, 这里的arg1就是comboBox的内容，若是int则是对应index
            ui->cbhow->clear();
            QByteArray cl0 = encrypt(arg1, hashKey);
            QString cmd = "select distinct " + colunm1 + " from " + table  + " where "
                    + colunm0 + " = ? and "
                    + colunm1 + " <> 'column1 for your master password'";

            QSqlQuery query;
            query.prepare(cmd);
            query.bindValue(0, cl0);
            query.exec();
            while(query.next())
            {
                QString item = decrypt(query.value(colunm1).toByteArray(), hashKey);
                ui->cbhow->addItem(item);
            }
            ui->urpwd->setText("^_^");

            restartTimer();
            return;
        }
    }
}

void MainWindow::on_deleteBtn_clicked()
{//这里删除选择的密码，删除前确认
    QString c0 = ui->cbpwd4w->currentText();
    QString c1 = ui->cbhow->currentText();//这里要先把删除条件记下来，因为下面解锁会更新下拉框的值
    QString msg = "真的不要" + c0 + "/" + c1 + "的密码了嘛>_<";
    if(confirm(msg))
    {
        unlock();
        QByteArray pwd4wt = encrypt(c0, hashKey);
        QByteArray how = encrypt(c1, hashKey);

        QSqlQuery query;
        query.prepare(sqlDelete);
        query.bindValue(0, pwd4wt);
        query.bindValue(1, how);
        if(count > 0 && query.exec())
        {
            ui->urpwd->setText("^_^");
            infoBox("那个密码已经删掉了~");
            count--;
            showCount();
            show1stClass();
            restartTimer();
        }
        else
        {
            infoBox("emmm删除好像出了什么问题");
        }
    }
}

void MainWindow::show1stClass()
{
    //这里显示密码大类, 也是有更新时更新
    ui->cbpwd4w->clear();
    QSqlQuery query;
    query.exec("select distinct " + colunm0 + " from " + table  + " where "
               + colunm0 + " <> 'column0 for your master password'");
    while(query.next())
    {
        QString item = decrypt(query.value(colunm0).toByteArray(), hashKey);
        ui->cbpwd4w->addItem(item);
    }
    on_cbpwd4w_currentIndexChanged(ui->cbpwd4w->currentText());
}
