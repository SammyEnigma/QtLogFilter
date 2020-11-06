#include "SettingDlg.h"

#include "ui_setting.h"

#include <qhostinfo.h>

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
    ui->comboBox->addItem("any");

    connect(ui->pushButton, &QPushButton::clicked, [&] {
        close();
    });
}

SettingDlg::~SettingDlg()
{
    delete ui;
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