class Selection : QObject
{
%TypeHeaderCode
#include "Selection.h"
%End
    Selection(const Selection & __0);
public:
    Selection(QObject*  parent /TransferThis/ = 0);
    virtual ~Selection();
    int width() const;
    void setWidth(int value);
    int height() const;
    void setHeight(int value);
    int x() const;
    void setX(int value);
    int y() const;
    void setY(int value);
    QString type() const;
    void setType(QString value);
public Q_SLOTS:
    void clear();
    void contract(int value);
    Selection * copy(int x, int y, int w, int h);
    void cut(Node*  node);
    void deselect();
    void expand(int value);
    void feather(int value);
    void fill(Node*  node);
    void grow(int value);
    void invert();
    void resize(int w, int h);
    void rotate(int degrees);
    void select(int x, int y, int w, int h, int value);
    void selectAll(Node*  node);
Q_SIGNALS:
private:
};
