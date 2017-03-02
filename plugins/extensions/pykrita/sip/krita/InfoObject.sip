class InfoObject : QObject
{
%TypeHeaderCode
#include "InfoObject.h"
%End
    InfoObject(const InfoObject & __0);
public:
    InfoObject(QObject*  parent /TransferThis/ = 0);
    virtual ~InfoObject();
    QMap<QString, QVariant> properties() const;
    void setProperties(QMap<QString, QVariant> value);
public Q_SLOTS:
    void setProperty(const QString & key, QVariant value);
    QVariant property(const QString & key);
Q_SIGNALS:
private:
};
