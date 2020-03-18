class Resource : QObject
{
%TypeHeaderCode
#include "Resource.h"
%End
    Resource(const Resource & __0);
public:    
    bool operator==(const Resource &other) const;
    bool operator!=(const Resource &other) const;
public:
    virtual ~Resource();
public Q_SLOTS:
    QString type() const;
    QString name() const;
    void setName(QString value);
    QString filename() const;
    QImage image() const;
    void setImage(QImage image);
public Q_SLOTS:
Q_SIGNALS:
private:
};
