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
    bool visible();
    void setVisible(bool visible);
    QRectF boundingBox();
    QPointF position();
    void setPosition(QPointF point);
    QString toSvg();
Q_SIGNALS:
private:
};
