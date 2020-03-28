#include "clientwnd.h"
#include "ui_clientwnd.h"

ClientWnd::ClientWnd(const QString& strHost, int nPort, QWidget *parent):
    MesClient(strHost, nPort, parent),
    ui(new Ui::ClientWnd)
{
    ui->setupUi(this);
    connect(this, SIGNAL(showSysMsg()), SLOT(slotShowSysMsg()) );
    connect(ui->action_Connect, SIGNAL( triggered() ), this, SLOT( slotReconnect() ) );
    connect(ui->action_Login, SIGNAL(triggered()), this, SLOT(login()));

    slotShowSysMsg();
    setFriendModel();

    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectContact(QListWidgetItem*)));
}

ClientWnd::~ClientWnd(){
    delete ui;
}


void ClientWnd::setFriendModel(){
    if (not friendLst.isEmpty()){
        QMap<QString, QString>::iterator iter = friendLst.begin();
        QStringList inModel;
        while (iter != friendLst.end()){
            inModel << iter.value();
            ++iter;
        }
        ui->listWidget->addItems(inModel);
    }
}

void ClientWnd::checkUserStatus(const QString & id){
    slotSendToServer("2 " + id);
}

//slots
void ClientWnd::slotShowSysMsg(){
    ui->statusbar->showMessage(sysMsg);
    QStringList msg;
    msg = sysMsg.trimmed().split(' ');
    if (msg.length()>1){
        switch (msg[1].toInt()) {
        case 1:
            if (msg[2] == '1')
                sysMsg = "Login succeeded.";
            else
                sysMsg = "Login unsucceeded!";
            slotShowSysMsg();
            break;
        case 2:
            if (msg[3] == "NULL"){
                ui->listWidget->currentItem()->setBackground(Qt::white);
                ui->listWidget->currentItem()->setForeground(Qt::black);
            }
            else{
                ui->listWidget->currentItem()->setBackground(Qt::darkGreen);
                ui->listWidget->currentItem()->setForeground(Qt::white);
            }
            break;
        default:

            break;
        }

    }
}

void ClientWnd::login(){
    logForm = new QDialog(this);
    QVBoxLayout mainLayout;
    mainLayout.addWidget(new QLabel("Login", logForm));
    loginLE = new QLineEdit(logForm);
    mainLayout.addWidget(loginLE);
    mainLayout.addWidget(new QLabel("Password", logForm));
    passwordLE = new QLineEdit(logForm);
    passwordLE->setEchoMode(QLineEdit::Password);
    mainLayout.addWidget(passwordLE);
    QPushButton* okBtn = new QPushButton("&Ok", logForm);
    mainLayout.addWidget(okBtn);
    connect(okBtn, SIGNAL(clicked()), this, SLOT( enterLogData() ) );

    logForm->setLayout(&mainLayout);
    logForm->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    logForm->setWindowModality(Qt::WindowModal);

    logForm->hide();
    logForm->show();
}

void ClientWnd::enterLogData(){
    slotSendToServer("1 "+loginLE->text()+" "+passwordLE->text());
    logForm->close();
    delete logForm;
}

void ClientWnd::selectContact(QListWidgetItem* item){
    QString contactName = item->data(Qt::DisplayRole).toString();
    QString id = friendLst.key(contactName);
    checkUserStatus(id);
}
