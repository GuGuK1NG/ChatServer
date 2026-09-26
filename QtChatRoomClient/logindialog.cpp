#include "logindialog.h"
#include "qchatclient.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    initSignalsAndSlots();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::initSignalsAndSlots()
{
    connect(ui->btnQuit,     &QPushButton::clicked, this, &LoginDialog::reject);
    connect(ui->lineEditPwd, &QLineEdit::returnPressed, this, &LoginDialog::on_btnLogin_clicked);
    QChatClient *c =QChatClient::instance();
    connect(c, &QChatClient::loginResult,  this, &LoginDialog::onLoginResult);//*这两个信号由,QChatClient发出
    connect(c,&QChatClient::regResult,this,&LoginDialog::onRegResult);        //这两个槽函数只用于确认状态
    connect(c, &QChatClient::linkStateChanged, this, &LoginDialog::onLinkStateChanged);

    c->connectToServer("192.168.31.219",6000);
}

void LoginDialog::on_btnLogin_clicked()
{

    const QString user = ui->lineEditUser->text().trimmed();
    const QString pwd = ui->lineEditPwd->text();
    if(user.isEmpty()||pwd.isEmpty()){
        QMessageBox::warning(this,"警告","用户名或密码不能为空");
        return;
    }
    QChatClient *c = QChatClient::instance();
    c->sendlogin(user,pwd);

    ui->btnLogin->setEnabled(false);
    ui->btnLogin->setText("登录中...");

}


void LoginDialog::on_btnReg_clicked()
{
    const QString user = ui->lineEditUser->text().trimmed();
    const QString pwd = ui->lineEditPwd->text();
    if(user.isEmpty()||pwd.isEmpty()){
        QMessageBox::warning(this,"警告","用户名或密码不能为空");
        return;
    }
    QChatClient *c = QChatClient::instance();
    c->sendreg(user,pwd);
    ui->btnReg->setEnabled(false);
    ui->btnReg->setText("注册中...");

}

void LoginDialog::on_btnQuit_clicked()
{
    reject();
}

void LoginDialog::onLoginResult(int errno_, const QString &errmsg)
{
    if(errno_==0){
        accept();
    }
    else{
        ui->btnLogin->setEnabled(true);
        ui->btnLogin->setText("登录");
        QMessageBox::warning(this,"登录失败",errmsg);
    }
}

void LoginDialog::onRegResult(int errno_, const QString &errmsg)
{
    if(errno_==0){
        ui->btnReg->setEnabled(true);
        ui->btnReg->setText("注册");
        QMessageBox::information(this,"注册成功","注册成功,请登录");
    }
    else{
        ui->btnReg->setEnabled(true);
        ui->btnReg->setText("注册");
        QMessageBox::warning(this,"注册失败",errmsg);
        qDebug()<<errmsg;
    }
}

void LoginDialog::onLinkStateChanged(bool connected, const QString &text)
{
    setConnStatus(connected, text);
}

void LoginDialog::setConnStatus(bool connected, const QString &text)
{
    ui->labelConnStatus->setText(text);
    ui->labelConnStatus->setStyleSheet(
        connected ? "color: green;" : "color: red;");
}






