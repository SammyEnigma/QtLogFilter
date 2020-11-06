#include "ShadowWidget.h"
#include <QCloseEvent>
#include <qparallelanimationgroup.h>
#include <qdebug.h>

ShadowWidget::ShadowWidget(QWidget* parent) {
	parentRect = parent->geometry();
	setAttribute(Qt::WA_DeleteOnClose);//关闭后自动删除
	setAttribute(Qt::WA_TranslucentBackground);//主窗口透明
	setWindowFlags(windowFlags() | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint);//去除popup模式下的自带阴影、去除标题栏

	animDuration = 600;
	titleHeight = 10;
}


ShadowWidget::~ShadowWidget() {
}

void ShadowWidget::show() {
	QWidget::show();
	if (windowFlags() & Qt::Popup) {//popup类型获取键盘焦点以支持中文输入
		activateWindow();
	}
}

void ShadowWidget::closeEvent(QCloseEvent *e) {
	static bool closeAccept = false;
	if (closeAccept) {
		e->accept();
		closeAccept = false;
		return;
	}
	e->ignore();
	auto group = new QParallelAnimationGroup;
	group->addAnimation(transAnimation(false));
	group->addAnimation(posAnimation(false));
	group->start(QAbstractAnimation::DeleteWhenStopped);
	closeAccept = true;
	connect(group, &QParallelAnimationGroup::finished, this, &QWidget::close);
}

void ShadowWidget::showEvent(QShowEvent *) {
	move(parentRect.x() + (parentRect.width() - width()) / 2, parentRect.y() + (parentRect.height() - height()) / 2);//移动到父窗口中间
	//设置显示过渡动画
	auto group = new QParallelAnimationGroup;
	group->addAnimation(transAnimation());
	group->addAnimation(posAnimation());
	group->start(QAbstractAnimation::DeleteWhenStopped);
}

QPropertyAnimation* ShadowWidget::transAnimation(bool show) {
	auto animation = new QPropertyAnimation(this, "windowOpacity");
	animation->setDuration(animDuration);
	animation->setEasingCurve(QEasingCurve::OutQuint);
	int startV = 0;
	int endV = 1;
	if (!show) {
		startV = 1;
		endV = 0;
	}
	animation->setStartValue(startV);
	animation->setEndValue(endV);
	return animation;
}

QPropertyAnimation* ShadowWidget::posAnimation(bool show) {
	auto animation = new QPropertyAnimation(this, "pos");
	animation->setDuration(animDuration);
	animation->setEasingCurve(QEasingCurve::OutQuint);
	auto geometry = this->geometry();
	int topOffset = 10;
	auto startV = QPoint(geometry.left(), geometry.top() - topOffset);
	auto endV = QPoint(geometry.left(), geometry.top());
	if (!show) {
		startV = QPoint(geometry.left(), geometry.top());
		endV = QPoint(geometry.left(), geometry.top() + topOffset);
	}
	animation->setStartValue(startV);
	animation->setEndValue(endV);
	return animation;
}

bool ShadowWidget::event(QEvent *e) {
	auto type = e->type();
	if (type == QEvent::MouseButtonPress || type == QEvent::MouseMove || type == QEvent::MouseButtonRelease) {
		static QPoint pos;//鼠标点击时窗口左上角相对屏幕偏移量
		static bool mouseValid;//拖拽有效
		auto event = dynamic_cast<QMouseEvent*>(e);
		if (event->button() == Qt::LeftButton && type == QEvent::MouseButtonPress) {
			auto y = event->y();
			if (y > bodyProperty.shadowWidth && y <= titleHeight) {
				pos = event->globalPos() - this->pos();
				mouseValid = true;
			} else {
				mouseValid = false;
			}
		} else if ((event->buttons() & Qt::LeftButton) && mouseValid && type == QEvent::MouseMove) {
			move(event->globalPos() - pos);
		} else if (type == QEvent::MouseButtonRelease) {
			mouseValid = false;
		}
	}

	return QWidget::event(e);
}
