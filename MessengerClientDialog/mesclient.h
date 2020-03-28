#ifndef MESCLIENT_H
#define MESCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QtCore>
#include <QPointer>

class MesClient : public QMainWindow
{
    Q_OBJECT
protected:
    QTcpSocket* pTcpSocket;
    quint16 nextBlockSize;
    QString strHost;
    int nPort;

    QPointer<QFile> frLstFile;
    QFile* logData;//в классе виджета обработать
    QMap<QString,QString> friendLst;

    QString sysMsg;

public:
    MesClient(const QString& strHost, int nPort, QWidget *parent = 0);
    ~MesClient();

protected:
    bool readFriendList();

signals:
    void signalSendToServer(QString);
    void showSysMsg();//определить для него слот в наследуемом классе виджета

protected slots:
    void slotReadReady();
    void slotError(QAbstractSocket::SocketError);
    void slotSendToServer(QString);
    void slotConnected();
    void slotReconnect();
    //void slotConnectToUser();
};

#endif // MESCLIENT_H
