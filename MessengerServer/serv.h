#ifndef SERV_H
#define SERV_H

#include <QObject>
#include <QtCore>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QtSql>

class serv : public QObject
{
    Q_OBJECT
private:
    QTcpServer * ptcpServer;
    quint16 nextBlockSize;

public:
    serv(int nPort = 2323, QObject *parent = 0);
    void sendToClient(QTcpSocket* pSocket, const QString& str);

private:
    static bool connectDB();
    int userAuth(QString* login, QString* password);
    QString checkUserStatus(int user_id);
    bool setUserIp(int user_id, QString* user_ip);
    bool isUserAuthorized(QString* user_ip);

signals:
    void signalLogin(QString* login, QString* password, QTcpSocket* pSocket);
    void signalChekUserStatus(int id, QTcpSocket* pSocket);
    void signalDeleteLater();

public slots:
    void slotNewConnection();
    void slotReadClient();

private slots:
    void slotLogin(QString* login, QString* password, QTcpSocket* pSocket);
    void slotGetUserIp(int id, QTcpSocket* pSocket);
    void slotDisconnected();
};


#endif // SERV_H
