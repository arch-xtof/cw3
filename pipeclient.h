#ifndef PIPECLIENT_H
#define PIPECLIENT_H

#include <QMainWindow>
#include <QLocalSocket>
#include <QtDebug>
#include <QVector>
#include <QDataStream>
#include <QByteArray>
#include <QIODevice>
#include <QMessageBox>
#include <QString>
#include <QtEndian>

QT_BEGIN_NAMESPACE
namespace Ui { class pipeClient; }
QT_END_NAMESPACE

class pipeClient : public QMainWindow
{
    Q_OBJECT

public:
    pipeClient(QWidget *parent = nullptr);
    ~pipeClient();

private slots:
    void Connect();
    void Compute();
    void Break();
    void Exit();

private:
    Ui::pipeClient *ui;
    QLocalSocket* localSocket = new QLocalSocket(this);
    QByteArray forgePacket();
    void visualize(QByteArray);
};
#endif // PIPECLIENT_H
