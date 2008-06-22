#include <QtGui>
#include "MyWidget.h"
#include <iostream>
#include "stroke.h"

using namespace std;

MyWidget::MyWidget(QWidget *parent)
	: QWidget(parent)
{
	setBackgroundRole(QPalette::Dark);
	setAutoFillBackground(true);
	setFocusPolicy(Qt::StrongFocus);
	setWindowTitle("Press F1, F2 or F3 to change line algorithm");


	dda = new QString("DDA Line");
	wu = new QString("Wu anti-aliased line");
	gs = new QString("Gupta-Sproul line");

	lines = 0;
	width = 1;
	drag = false;
	
	int w = 640, h = 480;
	this->resize(w,h);
	label = new QLabel();
	widthLabel = new QLabel( QString::number(width) );
	
	image = new QImage(w,h,QImage::Format_ARGB32);
	label->setPixmap(QPixmap::fromImage(*image));
	//QObject::connect(quit, SIGNAL(clicked())),this, SLOT(drawLine(int,int,int,int,int,const QColor &) );

	QPushButton *quit = new QPushButton("Quit", this);
    quit->setGeometry(62, 40, 75, 30);

	QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(label);
	topLeftLayout->addWidget(widthLabel);
	topLeftLayout->addWidget(quit);

	QObject::connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()));

    setLayout(topLeftLayout);

}

void MyWidget::paintEvent(QPaintEvent * /* event */)
{
	QStylePainter painter(this);
	pixmap = QPixmap::fromImage(*image);
	painter.drawPixmap(0,0, pixmap);
}

void MyWidget::mousePressEvent(QMouseEvent *event)
{
	drag = true;
    p0 = event->pos();
	setCursor(Qt::CrossCursor);
}

void MyWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (drag){
		p1 = event->pos();
	}
}

void MyWidget::mouseReleaseEvent(QMouseEvent * /*event*/)
{
	drag = false;
	//cout << p0.x() << p0.y() << p1.x() << p1.y() << endl;
	Stroke stroke;
	if (windowTitle() == dda){
		stroke.drawDDALine(image,p0.x(),p0.y(),p1.x(),p1.y(),Qt::black);
	} else
	if (windowTitle() == wu){
		stroke.drawWuLine(image,p0.x(),p0.y(),p1.x(),p1.y(),width,Qt::black);
	} else
	if (windowTitle() == gs){
		stroke.drawGSLine(image,p0.x(),p0.y(),p1.x(),p1.y(),width,width,Qt::black);
	}else
		setWindowTitle("Error");

	unsetCursor();
	update();
}

void MyWidget::keyPressEvent(QKeyEvent *event)
{
	switch ( event->key() ){
		case Qt::Key_F1:
			setWindowTitle(*dda);
			break;
		case Qt::Key_F2:
			setWindowTitle(*wu);
			break;
		case Qt::Key_F3:
			setWindowTitle(*gs);
			break;
		case Qt::Key_Up:
			setWidth(width+1);
			break;
		case Qt::Key_Down:
			setWidth(width-1);;
			break;

	}

}

void MyWidget::setWidth(int value)
{
	width = value;
	widthLabel->setText( QString::number(width) );
}


