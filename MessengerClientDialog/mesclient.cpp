#include "mesclient.h"

MesClient::MesClient(const QString& strHost, int nPort, QWidget *parent):
    QMainWindow(parent),
    nextBlockSize(0)
{
    //Соединение с сервером и соединение слотов
    pTcpSocket = new QTcpSocket(this);
    this->strHost = strHost;
    this->nPort = nPort;

    pTcpSocket->connectToHost(strHost, nPort);
    sysMsg = QString(pTcpSocket->state());
    connect(pTcpSocket, SIGNAL( connected() ), this, SLOT( slotConnected() ) );
    connect(pTcpSocket, SIGNAL( readyRead() ), this, SLOT( slotReadReady() ) );
    connect(pTcpSocket, SIGNAL( error(QAbstractSocket::SocketError) ),
            this, SLOT( slotError(QAbstractSocket::SocketError) )
            );
    connect(this, SIGNAL(signalSendToServer(QString)), SLOT(slotSendToServer(QString)));

    frLstFile = new QFile("./data/frLst.data");
    if (frLstFile->exists()){
        readFriendList();
    }
    delete frLstFile;
}

MesClient::~MesClient(){
    if (not frLstFile.isNull()){
        if (frLstFile->isOpen())
            frLstFile->close();
        delete frLstFile;
    }
}

bool MesClient::readFriendList(){
    frLstFile->open(QFile::ReadOnly);
    char buff[1024];
    QStringList buffLst;
    while (frLstFile->readLine(buff, sizeof(buff)) > 0) {
        //friendLst.append(QString(buff).trimmed().split(' '));
        buffLst = QString(buff).trimmed().split(' ');
        friendLst.insert(buffLst[0], buffLst[1]);
    }

    return true;
}

//SLOTS
void MesClient::slotReadReady(){
    QDataStream in(pTcpSocket);
    in.setVersion(QDataStream::Qt_5_3);
    for (;;) {
        if (!nextBlockSize) {
            if (pTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
        in >> nextBlockSize;
        }
        if (pTcpSocket->bytesAvailable() < nextBlockSize){
            break;
        }
        QTime time;
        QString str;
        in >> time >> str;

        sysMsg = time.toString() +" "+ str;
        emit showSysMsg();
        nextBlockSize = 0;
        //надо будет обработать sysMsg...
    }
}

void MesClient::slotError(QAbstractSocket::SocketError err){
    QString strError = "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                                            "The host was not found." :
                                            err == QAbstractSocket::RemoteHostClosedError ?
                                            "The remote host is closed." :
                                            err == QAbstractSocket::ConnectionRefusedError ?
                                            "The connection was refused." :
                                            QString(pTcpSocket->errorString())
                                            );
    sysMsg = strError;
    emit showSysMsg();
}

void MesClient::slotConnected(){
    //connected
    sysMsg = "connected";
    emit showSysMsg();
}

void MesClient::slotReconnect(){
    //reconnecting
    emit showSysMsg();
    pTcpSocket->connectToHost(strHost, nPort);
}

void MesClient::slotSendToServer(QString txt){
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << QTime::currentTime() << txt;

    out.device()->seek(0);
    out << quint16(arrBlock.size()) - sizeof(quint16);

    pTcpSocket->write(arrBlock);
}
