#include "pipeclient.h"
#include "ui_pipeclient.h"

pipeClient::pipeClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pipeClient)
{
    ui->setupUi(this);
    ui->btnCompute->setEnabled(0);
    ui->btnBreak->setEnabled(0);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    connect(ui->btnConnect, &QPushButton::clicked, this, &pipeClient::Connect );
    connect(ui->btnCompute, &QPushButton::clicked, this, &pipeClient::Compute );
    connect(ui->btnBreak, &QPushButton::clicked, this, &pipeClient::Break );
    connect(ui->btnExit, &QPushButton::clicked, this, &pipeClient::Exit );
}

pipeClient::~pipeClient()
{
    delete ui;
}

void pipeClient::Connect(){
    localSocket->connectToServer("\\\\.\\pipe\\ICS0025");
    if(localSocket->state() == QLocalSocket::ConnectedState){
           ui->status->append("Connected");
           ui->btnConnect->setEnabled(0);
           ui->btnCompute->setEnabled(1);
           ui->btnBreak->setEnabled(1);
    }
    else{
           ui->status->append(localSocket->errorString());
           ui->btnConnect->setEnabled(1);
           ui->btnCompute->setEnabled(0);
           ui->btnBreak->setEnabled(0);
    }
}

void pipeClient::Compute(){
    if(localSocket->state() == QLocalSocket::ConnectedState){
           QByteArray reply, packet = forgePacket();
           if(packet.isEmpty())return;
           //ui->status->append(packet.toHex());
           localSocket->write(packet);

           if(localSocket->waitForReadyRead(1000)){
               reply = localSocket->readAll();
               //ui->status->append(reply.toHex());
               visualize(reply);
           }
           else{
                   ui->status->append(localSocket->errorString());
                   ui->btnConnect->setEnabled(1);
                   ui->btnCompute->setEnabled(0);
                   ui->btnBreak->setEnabled(0);
                   return;
           }

    }
    else{
           ui->status->append(localSocket->errorString());
           ui->btnConnect->setEnabled(1);
           ui->btnCompute->setEnabled(0);
           ui->btnBreak->setEnabled(0);
    }
}

void pipeClient::Break(){
    if(localSocket->state() == QLocalSocket::ConnectedState){
        QByteArray packet;
        QDataStream dataStream(&packet, QIODevice::WriteOnly | QIODevice::Append);
        wchar_t* func_name = new wchar_t[5]{'S', 't', 'o', 'p'};
        dataStream << qToBigEndian(14);
        dataStream.writeRawData(reinterpret_cast<char*>(func_name), 10);
        localSocket->write(packet);
        ui->status->append("Disconnected");
        ui->btnConnect->setEnabled(1);
        ui->btnCompute->setEnabled(0);
        ui->btnBreak->setEnabled(0);
    }
    else{
           ui->status->append(localSocket->errorString());
           ui->btnConnect->setEnabled(1);
           ui->btnCompute->setEnabled(0);
           ui->btnBreak->setEnabled(0);
    }
}

void pipeClient::Exit()
{
    localSocket->disconnectFromServer();
    close();
}

QByteArray pipeClient::forgePacket(){
    bool casted;
    QString Function = ui->inpFunction->currentText();
    double X0 = ui->inpX0->text().toDouble(&casted);
    if(!casted){
        QMessageBox::warning(this, "Warning", "X0 supports only double numbers");
        return NULL;
    }
    double Xn = ui->inpXn->text().toDouble(&casted);
    if(!casted){
        QMessageBox::warning(this, "Warning", "Xn supports only double numbers");
        return NULL;
    }
    int nPoints = ui->inpnPoints->text().toInt(&casted);
    if(!casted){
        QMessageBox::warning(this, "Warning", "nPoints supports only integers");
        return NULL;
    }
    int Order = ui->inpOrder->text().toInt(&casted);
    if(Function == "Bessel function" && !casted){
        QMessageBox::warning(this, "Warning", "Order supports only integers");
        return NULL;
    }
    if(X0 > Xn){
        QMessageBox::warning(this, "Warning", "X0 can't be bigger than Xn");
        return NULL;
    }

    QByteArray tmp, packet;
    QDataStream dataStream(&tmp, QIODevice::WriteOnly | QIODevice::Append);
    QDataStream dataStream2(&packet, QIODevice::WriteOnly | QIODevice::Append);
    wchar_t* func_name = new wchar_t[Function.length()];
    Function.toWCharArray(func_name);
    dataStream.writeRawData(reinterpret_cast<char*>(func_name), Function.length()*2);
    char zero = '\0';
    dataStream.writeRawData(&zero, 1);
    dataStream.writeRawData(&zero, 1);
    dataStream << qToBigEndian(X0) << qToBigEndian(Xn) << qToBigEndian(nPoints);
    if(Function == "Bessel function") dataStream << qToBigEndian(Order);
    dataStream2 << qToBigEndian(static_cast<int>(tmp.size()+4));
    packet.append(tmp);
    return packet;
}

void pipeClient::visualize(QByteArray reply){
    QDataStream dataStream(&reply, QIODevice::ReadOnly);
    int length, tmp;
    QByteArray status;
    dataStream >> tmp;
    length = qToBigEndian(tmp);
    status.resize(12);
    dataStream.readRawData(status.data(), 12);
    if(!status.startsWith(0x43)){
        QMessageBox::warning(this, "Warning", "Server replied with an error");
        return;
    }
    QVector<double> Xs, Ys;
    while(!dataStream.atEnd()){
        dataStream.setByteOrder(QDataStream::LittleEndian);
        double x, y;
        dataStream >> x >> y;
        Xs.push_back(x);
        Ys.push_back(y);
    }
    double minimum_y = Ys.at(0);
    double maximum_y = Ys.at(0);
    std::for_each(Ys.begin(), Ys.end(),[&minimum_y](double coordinate){if(minimum_y > coordinate ) minimum_y = coordinate;});
    std::for_each(Ys.begin(), Ys.end(),[&maximum_y](double coordinate){if(maximum_y < coordinate ) maximum_y = coordinate;});
    ui->plot->clearGraphs();
    ui->plot->addGraph();
    ui->plot->graph(0)->setData(Xs, Ys);
    ui->plot->xAxis->setRange(ui->inpX0->text().toInt(),ui->inpXn->text().toInt());
    ui->plot->yAxis->setRange(minimum_y, maximum_y);
    ui->plot->replot();
    ui->status->append("Plotted Successfully");
    ui->inpX0->setText("");
    ui->inpXn->setText("");
    ui->inpnPoints->setText("");
    ui->inpOrder->setText("");
}
