#ifndef KIS_FILTER_PREVIEW_WIDGET
#define KIS_FILTER_PREVIEW_WIDGET

#include <QWidget>

class QWheelEvent;
class QPaintEvent;

class KisFilterPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    KisFilterPreviewWidget(QWidget * parent);
    ~KisFilterPreviewWidget();

    virtual QSize sizeHint() const;
    virtual void paintEvent(QPaintEvent* event);

    void setImage(const QImage &img);

protected:





private:
    QPixmap m_pixmap;
    QBrush m_checkBrush;

};

#endif
