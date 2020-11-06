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

    connect(ui->pushButton, &QPushButton::clicked, [&] {
        close();
    });
}

FilterConfigDlg::~FilterConfigDlg() {
    delete ui;
}
