class Shape : QObject
{
%TypeHeaderCode
#include "Shape.h"
%End
    Shape(const Shape & __0);
    virtual ~Shape();
public Q_SLOTS:
    QString type() const;
    QString name() const;
    void setName(const QString &name);
    int zIndex() const;
    void setZIndex(int zindex);
    bool visible() const;
    void setVisible(bool visible);
    bool selectable() const;
    void setSelectable(bool selectable);
    bool geometryProtected() const;
    void setGeometryProtected(bool protect);
    QRectF boundingBox() const;
    QPointF position() const;
    void setPosition(QPointF point);
    QTransform transformation() const;
    void setTransformation(QTransform matrix);
    bool remove();
    void update();
    void updateAbsolute(QRectF box);
    QString toSvg(bool prependStyles = false, bool stripTextMode = true);
Q_SIGNALS:
private:
};
