class Resource : QObject
{
%TypeHeaderCode
#include "Resource.h"
%End
    Resource(const Resource & __0);
public:
    Resource(QObject*  parent /TransferThis/ = 0);
    virtual ~Resource();
    QString type() const;
    void setType(QString value);
    QString name() const;
    void setName(QString value);
    QString filename() const;
    void setFilename(QString value);
public Q_SLOTS:
Q_SIGNALS:
private:
};
