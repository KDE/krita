#ifndef __GRADIENTVIEW_H__
#define __GRADIENTVIEW_H__

#include <qptrlist.h>
#include <qwidget.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qsize.h>

//#warning "TODO: convert QColor -> KoColor"
// Why? The Gradients are RGB only, so use QColor.
struct GradientItem
{
	QColor leftColor;
	QColor rightColor;

	double middle;
	double right;
};

class GradientView : public QWidget
{
	Q_OBJECT

public:

	GradientView( QWidget *_parent = 0 , const char *_name = 0 );
	~GradientView();

	void addItem( QColor, QColor, float, float );

	virtual QSize sizeHint() const;

protected:

	virtual void paintEvent( QPaintEvent *_event );

private:

	void updatePixmap();
	void readGIMPGradientFile( const QString& _file );

	QPtrList<GradientItem> m_lstGradientItems;

	QPixmap m_pixmap;
};

#endif


