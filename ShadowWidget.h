#pragma once

#include <qwidget.h>
#include <qboxlayout.h>
#include <qgraphicseffect.h>
#include <qpropertyanimation.h>

#define RADIUS_DEFAULT_SINGLE	6	//单色最小半径
#define RADIUS_DEFAULT_IMAGE	11	//背景图最小半径

#define SAD_COLOR_DEFAULT_SINGLE	0xff7d7d7d
#define SAD_COLOR_DEFAULT_IMAGE		0xff000000

class ShadowWidget : public QWidget {
public:
	ShadowWidget(QWidget* parent);
	~ShadowWidget();

	void show();

protected:
	void closeEvent(QCloseEvent*) override;
	void showEvent(QShowEvent*) override;
	bool event(QEvent*) override;

	QPropertyAnimation* transAnimation(bool show = true);
	QPropertyAnimation * posAnimation(bool show = true);

	struct BodyProperty {
		uint shadowColor;//阴影初始颜色
		uint shadowWidth;//阴影宽度
		uint borderRadius;//边框四个角半径
		uint rgb;//背景色
		QString bg;//背景图片
	} bodyProperty = { SAD_COLOR_DEFAULT_SINGLE , 10, RADIUS_DEFAULT_SINGLE, 0xffffffff, "" };

	inline void setShadowColor(uint color) { bodyProperty.shadowColor = color; }
	inline void setShadowWidth(uint width) { bodyProperty.shadowWidth = width; titleHeight = width; }
	inline void setBorderRadius(uint radius) { bodyProperty.borderRadius = radius; }
	inline void setBackgroundColor(uint rgb) { bodyProperty.rgb = rgb; }
	inline void setBackgroundImage(const QString& bg) { bodyProperty.bg = bg; }
	//将body参数转化为stylesheet字符串
	inline auto getPropertyStyleSheet(const BodyProperty& bodyProperty) {
		QString style = "#ShadowWidget_body{border-radius:%1px;border:none;background:%2;}";
		return style.arg(QString::number(bodyProperty.borderRadius), [bodyProperty] {
			if (bodyProperty.bg.isEmpty()) {
				return QColor(QRgba64::fromArgb32(bodyProperty.rgb)).name(QColor::HexArgb);
			}
			return "url(" + bodyProperty.bg + ')';//背景图片不为空，以设置图片为主忽略背景颜色
		}());
	}

	template<class T>
	void setupUi(T& ui) {
		//嵌套一层widget，设置背景和圆角效果，指定objectName避免影响子布局
		auto vbox = new QVBoxLayout(this);
		vbox->setContentsMargins(0, 0, 0, 0);
		auto wnd = new QWidget(this);
		wnd->setObjectName("ShadowWidget_body");
		wnd->setStyleSheet(getPropertyStyleSheet(bodyProperty));
		vbox->addWidget(wnd);

		//再嵌套一层widget，用于加载子布局
		auto vbox2 = new QVBoxLayout(wnd);
		vbox2->setContentsMargins(0, 0, 0, 0);
		auto body = new QWidget(this);
		vbox2->addWidget(body);

		ui.setupUi(body);//在第二层body中加载布局而不是在第一层wnd，避免ui布局文件中顶层widget设置stylesheet后wnd的样式无效
		resize(body->size());//将窗口大小设置为body相同大小
		//为wnd设置阴影
		auto shadowEffect = new QGraphicsDropShadowEffect(this);
		shadowEffect->setOffset(0, 0);
		shadowEffect->setColor(QColor(bodyProperty.shadowColor));
		shadowEffect->setBlurRadius(bodyProperty.shadowWidth);
		wnd->setGraphicsEffect(shadowEffect);
		//添加内边距显示阴影
		setContentsMargins(bodyProperty.shadowWidth, bodyProperty.shadowWidth, bodyProperty.shadowWidth, bodyProperty.shadowWidth);
	}
	//设置显示消失动画时间，ms
	inline void setAnimDuration(int duration) { animDuration = duration; }
	//设置可拖拽标题栏高度，如果设置了阴影宽度，必须在setShadowWidth之后设置
	inline void setDraggableTitleHeight(int height) { titleHeight = bodyProperty.shadowWidth + height; }

private:
	QRect parentRect;//父窗口位置
	int animDuration;//显示和关闭动画时间
	int titleHeight;//可拖拽标题栏高度
};

