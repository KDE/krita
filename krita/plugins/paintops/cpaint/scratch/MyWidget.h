#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>

class QLabel;
class QImage;

using namespace std;

class MyWidget : public QWidget
{
	Q_OBJECT

public:
	MyWidget(QWidget *parent = 0);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

public:
	QLabel *label;
	QLabel *widthLabel;
	QImage *image;
	int lines;
	QPixmap pixmap;
	QPoint p0, p1;
	int x0, y0, x1 ,y1;
	int width;

	bool drag;

	QString *dda;
	QString *wu;
	QString *gs;

	void setWidth(int value);
};

#endif

