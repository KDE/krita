class Selection : QObject
{
%TypeHeaderCode
#include "Selection.h"
%End
    Selection(const Selection & __0);
public:
    Selection(QObject*  parent /TransferThis/ = 0);
    virtual ~Selection();
    bool operator==(const Selection &other) const;
    bool operator!=(const Selection &other) const;
public Q_SLOTS:
    Selection *duplicate() /Factory/;
    int width() const;
    int height() const;
    int x() const;
    int y() const;
    void move(int x, int y);
    void clear();
    void contract(int value);
    void copy(Node *node);
    void cut(Node *node);
    void paste(Node *destination, int x, int y);
    void erode();
    void dilate();
    void border(int xRadius, int yRadius);
    void feather(int radius);
    void grow(int xradius, int yradius);
    void shrink(int xRadius, int yRadius, bool edgeLock);
    void smooth();
    void invert();
    void resize(int w, int h);
    void select(int x, int y, int w, int h, int value);
    void selectAll(Node*  node, int value);
    void replace(Selection *selection);
    void add(Selection *selection);
    void subtract(Selection *selection);
    void intersect(Selection *selection);
    void symmetricdifference(Selection *selection);
    QByteArray pixelData(int x, int y, int w, int h) const;
    void setPixelData(QByteArray value, int x, int y, int w, int h);
private:
};
