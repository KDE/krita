class Scratchpad : public QWidget /NoDefaultCtors/
{
%TypeHeaderCode
#include "Scratchpad.h"
%End


public:
        Scratchpad(View* view , const QColor & defaultColor, QWidget*  parent /TransferThis/ = 0);
        virtual ~Scratchpad();

public Q_SLOTS:
    void clear();
    void setModeManually(bool value);
    void linkCanvasZoom(bool value);
    bool canvasZoomLink();
    void setCanvasZoomLink(bool value);
    void setMode(QString modeName);
    void setFillColor(QColor color);
    void loadScratchpadImage(QImage image);

    void setScale(qreal scale);
    void scaleToFit();
    void scaleReset();
    qreal scale();
    void panTo(qint32 x, qint32 y);
    void panCenter();

    QRect viewportBounds() const;
    QRect contentBounds() const;

    void fillDefault();
    void fillBackground();
    void fillForeground();
    void fillTransparent();
    void fillGradient(const QPoint &gradientVectorStart = QPoint(),
                      const QPoint &gradientVectorEnd = QPoint(),
                      const QString &gradientShape = "linear",
                      const QString &gradientRepeat = "none",
                      bool reverseGradient = false,
                      bool dither = false);
    void fillLayer(bool fullContent = true);
    void fillDocument(bool fullContent = true);
    void fillPattern(QTransform transform = QTransform());

    QImage copyScratchpadImageData();
Q_SIGNALS:
    void scaleChanged(qreal scale);
    void contentChanged();
    void viewportChanged(const QRect rect);
};
