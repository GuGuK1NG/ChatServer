#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

#include <QAbstractSocket>
#include <QApplication>
#include <QMessageBox>
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_btnLogin_clicked();

    void on_btnReg_clicked();

    void on_btnQuit_clicked();

    void onLoginResult(int errno_, const QString &errmsg);
    void onRegResult(int errno_,const QString &errmsg);
    void onLinkStateChanged(bool connected, const QString &text);

    void setConnStatus(bool connected,const QString& text);

private:
    void initSignalsAndSlots();
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
