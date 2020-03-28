#include "serv.h"

serv::serv(int nPort,QObject *parent) :
    QObject(parent),
    nextBlockSize(0)
{
    ptcpServer = new QTcpServer(this);
    if ( !ptcpServer->listen(QHostAddress::Any, nPort) ){
        qDebug() << "Server error. Unable to start the server: " + ptcpServer->errorString();
        ptcpServer->close();
        return;
    }

    connect( ptcpServer, SIGNAL(newConnection()),
            this, SLOT(slotNewConnection()) );
    connect( this, SIGNAL( signalLogin(QString*, QString*, QTcpSocket*) ),
             this, SLOT( slotLogin(QString*, QString*, QTcpSocket*) ) );
    connect( this, SIGNAL( signalChekUserStatus(int, QTcpSocket*) ),
             this, SLOT( slotGetUserIp(int, QTcpSocket*) ) );

    if ( connectDB() ){
        qDebug() << "Connected to db.";
    }
    else
    {
        qDebug() << "Can not establish the connection.";
    }
    //testing code below. need to be deleted later;
    //its begin

    //its end
}

//---------------------------------------------------------------
//METHODS
void serv::sendToClient(QTcpSocket *pSocket, const QString &str){
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

bool serv::connectDB(){
    static QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setDatabaseName("messenger");
    db.setUserName("pasha");
    db.setHostName("localhost");
    db.setPassword("0000");
    if (!db.open()){
        qDebug() << "Can not open database:" << db.lastError().text();
        return false;
    }
    return true;
}

int serv::userAuth(QString* login, QString* password){
    int id = 0;
    QSqlQuery query;
    QString strArg = "SELECT user_id FROM users "
                    "WHERE user_log = '%1' "
                    "AND user_pass = sha2('%2',256);";
    QString str;
    str = strArg.arg(*login)
                .arg(*password);
    if (!query.exec(str)){
        qDebug() << "Unable to do authentitacion. " + query.lastError().text();
        return id;
    }
    QSqlRecord rec = query.record();
    while(query.next()){
        id = query.value(rec.indexOf("user_id")).toInt();
        qDebug() << "user_id = " + QString::number(id);
    }
    return id;
}

QString serv::checkUserStatus(int user_id){
    QSqlQuery query;
    QString strArg = "SELECT user_ip FROM users_online "
                        "WHERE user_id = %1;";
    QString str;
    str = strArg.arg(QString::number(user_id));

    QString ip = "NULL";

    if(!query.exec(str)){
        qDebug() << "Unable to check user\'s status. " + query.lastError().text();
        return ip;
    }

    QSqlRecord rec = query.record();
    while (query.next()) {
        ip = query.value(rec.indexOf("user_ip")).toString();
    }
    return ip;
}

bool serv::setUserIp(int user_id, QString* user_ip){
    QSqlQuery query;
    QString strArg = "UPDATE users_online SET user_ip = '%1' "
                        "WHERE user_id = %2;";
    QString str;
    str = strArg.arg(*user_ip)
            .arg(QString::number(user_id));

    qDebug() << str;

    if(!query.exec(str)){
        qDebug() << "Unable to set user\'s ip. " + query.lastError().text();
        return false;
    }

    return true;
}

bool serv::isUserAuthorized(QString *user_ip){
    QSqlQuery query;
    QString strArg = "SELECT EXISTS (SELECT user_ip "
                     "FROM users_online WHERE user_ip = '%1') isExist;";
    QString str = strArg.arg(*user_ip);

    if(!query.exec(str)){
        qDebug() << "Unable to check user\'s status. " + query.lastError().text();
        return false;
    }

    QSqlRecord rec = query.record();
    while (query.next()) {
        qDebug() << query.value( rec.indexOf("isExist") ).toInt();
        if( query.value( rec.indexOf("isExist") ).toInt() )
            return true;
    }
    return false;
}
//---------------------------------------------------------------
//SLOTS
void serv::slotNewConnection()
{
    QTcpSocket* pClientSocket = ptcpServer->nextPendingConnection();
    connect(this, SIGNAL( signalDeleteLater() ),
            pClientSocket, SLOT(deleteLater())
            );
    connect(pClientSocket, SIGNAL(disconnected()),
            this, SLOT( slotDisconnected() )
            );
    connect(pClientSocket, SIGNAL(readyRead()),
            this, SLOT(slotReadClient())
            );
    sendToClient(pClientSocket, "Server response: Connected!");
}

void serv::slotReadClient(){
    QTcpSocket* pClientSocket = static_cast<QTcpSocket*>(sender());
    //запишем здесь Ip клиента
    //в этом методе и должны вызываться всякие сигналы
    //для аутентификации, и любой обработки общения сервера с клиентом
    //возиожно стоит все методы обработки общения выделить
    //в отдельный класс... Либо сделать user_ip статическим...
    //Вообще лучше подробнее почитать о написании сетевых
    //приложений на qt

    //Придумать, что должен писать клиент, чтобы отличить его запрос
    //в соответствии с этим написать switch case
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_3);
    for (;;){
        if (!nextBlockSize){
            if (pClientSocket->bytesAvailable() < sizeof(quint16)){
                break;
            }
            in >> nextBlockSize;
        }


        if (pClientSocket->bytesAvailable() < nextBlockSize){
            break;
        }
        QTime time;
        QString str;
        in >> time >> str;
        QStringList strData;
        strData = str.split(" ");
        switch( strData[0].toInt() ){
        case 1 :
            //login (smb:123;smb2:32)
            if (strData.length() > 2){
                emit signalLogin(&strData[1], &strData[2], pClientSocket);
            }
            else{
                sendToClient(pClientSocket, "Server did not get user\'s login or password.");
            }
            break;
        case 2:
            //getip to connect other client
            if (strData.length() > 1){
                emit signalChekUserStatus(strData[1].toInt(), pClientSocket);
            }
            else{
                sendToClient(pClientSocket, "Server did not get user\'s ip.");
            }
            break;
        default:
            qDebug() << "Wrong message code.";
            return;
            break;
        }

        QString strMessage = time.toString() + " Client has sent - " + str;
        qDebug() << strMessage;
        nextBlockSize = 0;

        //sendToClient(pClientSocket, "Server Response : Resieved " + str);
    }
}

void serv::slotLogin(QString* login, QString* password, QTcpSocket* pClientSocket){
    int user_id = userAuth(login, password);
    QString user_ip = pClientSocket->peerAddress().toString();
    qDebug() << pClientSocket->peerAddress().toIPv4Address();
    if ( user_id ){
        if ( setUserIp(user_id, &user_ip ) ){
            sendToClient(pClientSocket, QString("1 ") + "1");
            return;
        }
        else{
            sendToClient(pClientSocket, QString("1 ") + "0");
            qDebug() << "Did not manage to set status "
                        "\"online\" to user.";
        }
    }
}

void serv::slotGetUserIp(int id, QTcpSocket* pClientSocket){
    QString user_ip = pClientSocket->peerAddress().toString();
    if ( isUserAuthorized(&user_ip) ){
        QString request_ip = checkUserStatus(id);
        QString str = "2 " + QString::number(id) + " ";
        str += request_ip;
        qDebug() << str ;
        sendToClient(pClientSocket, str);
    }
}

void serv::slotDisconnected(){
    QSqlQuery query;
    QTcpSocket* pClientSocket = static_cast<QTcpSocket*>( sender() );
    QString user_ip = pClientSocket->peerAddress().toString();

    QString strArg = "UPDATE users_online SET user_ip = \"NULL\" "
                     "WHERE user_ip = '%1'";
    QString str = strArg.arg(user_ip);
    if (!query.exec(str)){
        qDebug() << "Error updating database!" + query.lastError().text();
    }
    emit signalDeleteLater();
}



