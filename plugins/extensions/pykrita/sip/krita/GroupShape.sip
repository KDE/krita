class GroupShape : Shape
{
%TypeHeaderCode
#include "vector/GroupShape.h"
%End
    GroupShape(const GroupShape & __0);
public:
    virtual ~GroupShape();
    QList <Shape*> children();
Q_SIGNALS:
private:
};
