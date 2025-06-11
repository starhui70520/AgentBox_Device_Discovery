#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <windows.h>
#include <dwmapi.h>
#include <QUdpSocket>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>

class AgentoxDiscoverer : public QWidget {
    Q_OBJECT

public:
    AgentoxDiscoverer(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        setupSocket();
    }

private slots:
    void readPendingDatagrams() {
        while (udpSocket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(udpSocket->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;

            udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

            QString data = QString::fromUtf8(datagram);
            processAgentoxMessage(data, sender.toString());
        }
    }

    void onItemClicked(QListWidgetItem* item) {
        QString url = item->data(Qt::UserRole).toString();
        qInfo() << "Original URL:" << url;

        // 创建选择对话框
        QDialog dialog;
        dialog.setWindowTitle("选择服务");
        QVBoxLayout layout(&dialog);

        // 提示文本
        QLabel label("请选择要使用的服务:", &dialog);
        layout.addWidget(&label);

        // 使用 QList 存储有序的服务列表（端口号与显示名称的映射）
        QList<QPair<QString, QString>> serviceList = {
            {"8082", "主页"},
            {"8080", "主页(加密证书)"},
            {"3000", "企管端"},
            {"3001", "Dify"}
        };

        // 存储按钮和端口的映射关系
        QHash<QPushButton*, QString> buttonToPortMap;

        // 为每个服务创建一个按钮（按定义的顺序）
        for (const auto &service : serviceList) {
            const QString &port = service.first;
            const QString &serviceName = service.second;

            QPushButton *serviceButton = new QPushButton(serviceName, &dialog);
            layout.addWidget(serviceButton);
            buttonToPortMap[serviceButton] = port;

            QObject::connect(serviceButton, &QPushButton::clicked, [&dialog, port, url]() {
                QString finalUrl;
                if (port == "8080") {
                    finalUrl = "https://192.168.2.236:8080/";  // 固定主页URL
                } else {
                    finalUrl = url + ":" + port;
                }
                qInfo() << "Opening URL:" << finalUrl;
                QDesktopServices::openUrl(QUrl(finalUrl));
                dialog.accept(); // 关闭对话框
            });
        }

        // 添加一个取消按钮
        QPushButton *cancelButton = new QPushButton("取消", &dialog);
        layout.addWidget(cancelButton);
        QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        // 显示对话框
        dialog.exec();
    }

private:
    QUdpSocket *udpSocket;
    QListWidget *deviceList;

    void setupUI() {
        this->setAttribute(Qt::WA_TranslucentBackground);

        this->setWindowTitle("AgentBox设备查找");
        this->setWindowIcon(QIcon(":/logo_lzwc.png"));

        HWND hwnd = (HWND)this->winId();
        MARGINS magins = {-1, 50, 50, 50};

        const auto type = DWMSBT_TRANSIENTWINDOW;
        DwmExtendFrameIntoClientArea(hwnd, &magins);
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof(type));
        QVBoxLayout *layout = new QVBoxLayout(this);

        deviceList = new QListWidget(this);
        deviceList->setStyleSheet(
            "QListWidget {"
            "   font-size: 14px;"
            "   background-color: rgba(255, 255, 255, 60%);"  // 60% 透明度（白色背景）
            "   border-radius: 5px;"                          // 5px 圆角
            "}"
            );
        connect(deviceList, &QListWidget::itemClicked, this, &AgentoxDiscoverer::onItemClicked);

        QLabel *deviceList_title = new QLabel("设备列表：");

        layout->addWidget(deviceList_title);
        layout->addWidget(deviceList);
        resize(300, 500);
    }

    void setupSocket() {
        udpSocket = new QUdpSocket(this);

        if (!udpSocket->bind(QHostAddress::AnyIPv4, 1900, QUdpSocket::ShareAddress)) {
            QMessageBox::critical(this, "错误", "无法绑定到UDP端口1900");
            return;
        }

        if (!udpSocket->joinMulticastGroup(QHostAddress("239.255.255.250"))) {
            QMessageBox::warning(this, "警告", "无法加入多播组");
        }

        connect(udpSocket, &QUdpSocket::readyRead, this, &AgentoxDiscoverer::readPendingDatagrams);
    }

    void processAgentoxMessage(const QString &message, const QString &senderIp) {
        QStringList lines = message.split("\r\n", Qt::SkipEmptyParts);

        QString location;
        QString server;
        QString nt;

        for (const QString &line : lines) {
            if (line.startsWith("LOCATION:", Qt::CaseInsensitive)) {
                location = line.mid(9).trimmed();
            } else if (line.startsWith("SERVER:", Qt::CaseInsensitive)) {
                server = line.mid(7).trimmed();
            } else if (line.startsWith("NT:", Qt::CaseInsensitive)) {
                nt = line.mid(3).trimmed();
            }
        }

        // 只处理Agentox设备
        if (nt == "agentbox:device" && !location.isEmpty()) {
            bool exists = false;
            for (int i = 0; i < deviceList->count(); ++i) {
                if (deviceList->item(i)->data(Qt::UserRole).toString() == location) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setText(QString("AgentBox设备\nIP: %1\n服务: %2").arg(senderIp).arg(server));
                item->setData(Qt::UserRole, location);
                deviceList->addItem(item);
                qDebug() << "发现Agentbox设备:" << senderIp << location;
            }
        }
    }
};


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AgentoxDiscoverer discoverer;
    discoverer.show();

    return a.exec();
}

#include "main.moc"
