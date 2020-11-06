#pragma once

#include <qwidget.h>
#include <qboxlayout.h>
#include <qgraphicseffect.h>
#include <qpropertyanimation.h>

#define RADIUS_DEFAULT_SINGLE	6	//��ɫ��С�뾶
#define RADIUS_DEFAULT_IMAGE	11	//����ͼ��С�뾶

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
		uint shadowColor;//��Ӱ��ʼ��ɫ
		uint shadowWidth;//��Ӱ���
		uint borderRadius;//�߿��ĸ��ǰ뾶
		uint rgb;//����ɫ
		QString bg;//����ͼƬ
	} bodyProperty = { SAD_COLOR_DEFAULT_SINGLE , 10, RADIUS_DEFAULT_SINGLE, 0xffffffff, "" };

	inline void setShadowColor(uint color) { bodyProperty.shadowColor = color; }
	inline void setShadowWidth(uint width) { bodyProperty.shadowWidth = width; titleHeight = width; }
	inline void setBorderRadius(uint radius) { bodyProperty.borderRadius = radius; }
	inline void setBackgroundColor(uint rgb) { bodyProperty.rgb = rgb; }
	inline void setBackgroundImage(const QString& bg) { bodyProperty.bg = bg; }
	//��body����ת��Ϊstylesheet�ַ���
	inline auto getPropertyStyleSheet(const BodyProperty& bodyProperty) {
		QString style = "#ShadowWidget_body{border-radius:%1px;border:none;background:%2;}";
		return style.arg(QString::number(bodyProperty.borderRadius), [bodyProperty] {
			if (bodyProperty.bg.isEmpty()) {
				return QColor(QRgba64::fromArgb32(bodyProperty.rgb)).name(QColor::HexArgb);
			}
			return "url(" + bodyProperty.bg + ')';//����ͼƬ��Ϊ�գ�������ͼƬΪ�����Ա�����ɫ
		}());
	}

	template<class T>
	void setupUi(T& ui) {
		//Ƕ��һ��widget�����ñ�����Բ��Ч����ָ��objectName����Ӱ���Ӳ���
		auto vbox = new QVBoxLayout(this);
		vbox->setContentsMargins(0, 0, 0, 0);
		auto wnd = new QWidget(this);
		wnd->setObjectName("ShadowWidget_body");
		wnd->setStyleSheet(getPropertyStyleSheet(bodyProperty));
		vbox->addWidget(wnd);

		//��Ƕ��һ��widget�����ڼ����Ӳ���
		auto vbox2 = new QVBoxLayout(wnd);
		vbox2->setContentsMargins(0, 0, 0, 0);
		auto body = new QWidget(this);
		vbox2->addWidget(body);

		ui.setupUi(body);//�ڵڶ���body�м��ز��ֶ������ڵ�һ��wnd������ui�����ļ��ж���widget����stylesheet��wnd����ʽ��Ч
		resize(body->size());//�����ڴ�С����Ϊbody��ͬ��С
		//Ϊwnd������Ӱ
		auto shadowEffect = new QGraphicsDropShadowEffect(this);
		shadowEffect->setOffset(0, 0);
		shadowEffect->setColor(QColor(bodyProperty.shadowColor));
		shadowEffect->setBlurRadius(bodyProperty.shadowWidth);
		wnd->setGraphicsEffect(shadowEffect);
		//����ڱ߾���ʾ��Ӱ
		setContentsMargins(bodyProperty.shadowWidth, bodyProperty.shadowWidth, bodyProperty.shadowWidth, bodyProperty.shadowWidth);
	}
	//������ʾ��ʧ����ʱ�䣬ms
	inline void setAnimDuration(int duration) { animDuration = duration; }
	//���ÿ���ק�������߶ȣ������������Ӱ��ȣ�������setShadowWidth֮������
	inline void setDraggableTitleHeight(int height) { titleHeight = bodyProperty.shadowWidth + height; }

private:
	QRect parentRect;//������λ��
	int animDuration;//��ʾ�͹رն���ʱ��
	int titleHeight;//����ק�������߶�
};

