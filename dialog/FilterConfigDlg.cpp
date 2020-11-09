#include "FilterConfigDlg.h"

#include "ui_filterconfig.h"

FilterConfigDlg::FilterConfigDlg(QWidget* parent)
    : ShadowWidget(parent)
{
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowModality(Qt::ApplicationModal);
    setBackgroundColor(0xff252526);
    setBorderRadius(4);
    setDraggableTitleHeight(164);

    ui = new Ui::FilterConfig;
    setupUi(*ui);

    connect(ui->btn_cancel, &QPushButton::clicked, [&] {
        close();
    });

    connect(ui->btn_confim, &QPushButton::clicked, [&] {
        ConfigData data;
        data.configName = ui->name->text();
        if (data.configName.isEmpty()) {
            return;
        }
        data.tagThreadName = ui->thread->text();
        data.tag = ui->tag->text();
        data.regEnable = ui->reg->isChecked();
        data.logLevel = ui->level->currentIndex();
        newConfig(data);
        close();
    });

    ui->level->addItems(QStringList() << "debug" << "warning" << "error");
}

FilterConfigDlg::~FilterConfigDlg() {
    delete ui;
}
