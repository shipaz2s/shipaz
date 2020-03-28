#ifndef CLIENTWND_H
#define CLIENTWND_H

#include <QObject>
#include <QMainWindow>
#include "mesclient.h"
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QBrush>


namespace Ui {
    class ClientWnd;
}

class ClientWnd : public MesClient
{
    Q_OBJECT
public:
    ClientWnd(const QString& strHost, int nPort, QWidget *parent = nullptr);
    ~ClientWnd();

private:
    Ui::ClientWnd * ui;
    QStringListModel friendModel;

    QDialog* logForm;
    QLineEdit* loginLE;
    QLineEdit* passwordLE;

    void setFriendModel();
    void checkUserStatus(const QString&);//true = online


private slots:
    void slotShowSysMsg();
    void login();
    void enterLogData();
    void selectContact(QListWidgetItem*);
};

#endif // CLIENTWND_H
