class Transformation : QObject
{
%TypeHeaderCode
#include "Transformation.h"
%End
    Transformation(const Transformation & __0);
public:
    Transformation(QObject*  parent /TransferThis/ = 0);
    virtual ~Transformation();
    QString definition() const;
    void setDefinition(QString value);
public Q_SLOTS:
Q_SIGNALS:
private:
};
