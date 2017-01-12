class Selection : QObject
{
%TypeHeaderCode
#include "Selection.h"
%End
    Selection(const Selection & __0);
public:
    Selection(QObject*  parent /TransferThis/ = 0);
    virtual ~Selection();
public Q_SLOTS:
    int width() const;
    int height() const;
    int x() const;
    int y() const;
    void move(int x, int y);
    void clear();
    void contract(int value);
    void cut(Node *node);
    void paste(Node *source, Node*destination);
    void deselect();
    void expand(int value);
    void feather(int value);
    void grow(int xradius, int yradius);
    void invert();
    void resize(int w, int h);
    void select(int x, int y, int w, int h, int value);
    void selectAll(Node*  node, int value);
    void replace(Selection *selection);
    void add(Selection *selection);
    void subtract(Selection *selection);
    void intersect(Selection *selection);
    QByteArray pixelData(int x, int y, int w, int h) const;
    void setPixelData(QByteArray value, int x, int y, int w, int h);
private:
};
