class Shape : QObject
{
%TypeHeaderCode
#include "vector/Shape.h"
%End
    Shape(const Shape & __0);
public:
    virtual ~Shape();
    QString type() const;
    QString name() const;
    void setName(const QString &name);
    bool visible();
    void setVisible(bool visible);
Q_SIGNALS:
private:
};
