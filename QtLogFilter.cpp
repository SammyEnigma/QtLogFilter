#include "QtLogFilter.h"

#include <qfile.h>

#include "SmoothScrollbar.h"

#include "dialog/SettingDlg.h"
#include "dialog/FilterConfigDlg.h"

QtLogFilter::QtLogFilter(QWidget *parent)
    : ShadowWidget(parent)
{
	setWindowFlag(Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_DeleteOnClose, false);
	setBackgroundColor(0xff252526);
	setBorderRadius(4);
	setDraggableTitleHeight(40);
    setupUi(ui);

    //ui.comboBox->addItems(getLocalIps());

    connect(ui.btn_close, &QPushButton::clicked, this, &QWidget::close);
    connect(ui.btn_setting, &QPushButton::clicked, [&] {
        auto dlg = new SettingDlg(this);
        dlg->show();
    });
    connect(ui.btn_add, &QPushButton::clicked, [&] {
        auto dlg = new FilterConfigDlg(this);
        dlg->show();
    });

    ui.textBrowser->setVerticalScrollBar(new SmoothScrollbar(this));

    QFile file("test.txt");
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();
    ui.textBrowser->setHtml(data);
}

void QtLogFilter::showEvent(QShowEvent*) {
	move(QCursor::pos());
}

