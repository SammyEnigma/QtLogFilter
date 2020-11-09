#include "SettingDlg.h"

#include "ui_setting.h"

#include <qregexp.h>
#include <qregularexpression.h>

SettingDlg::SettingDlg(QWidget* parent)
    : ShadowWidget(parent)
{
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowModality(Qt::ApplicationModal);
    setBackgroundColor(0xff252526);
    setBorderRadius(4);
    setDraggableTitleHeight(126);

    ui = new Ui::Setting;
    setupUi(*ui);

    ui->comboBox->addItems(getLocalIps());
    ui->comboBox->addItem("0.0.0.0");
    ui->comboBox->addItem("127.0.0.1");

    connect(ui->pushButton, &QPushButton::clicked, [&] {
        int port = ui->lineEdit->text().toInt();
        if (port > 65535) {
            return;
        }
        QHostAddress select = QHostAddress(ui->comboBox->currentText());
        if (select != oldAddr || port != oldPort) {
            configChanged(select, oldPort);
        }
        close();
    });

    ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("\\d{0,5}")));
}

SettingDlg::~SettingDlg()
{
    delete ui;
}

void SettingDlg::setConfig(const QHostAddress& address, int port) {
    oldAddr = address;
    oldPort = port;
    ui->comboBox->setCurrentText(address.toString());
    ui->lineEdit->setText(QString::number(port));
}

QStringList SettingDlg::getLocalIps() {
    QStringList ips;
    auto info = QHostInfo::fromName(QHostInfo::localHostName());
    for (const auto& address : info.addresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            ips << address.toString();
        }
    }
    return ips;
}