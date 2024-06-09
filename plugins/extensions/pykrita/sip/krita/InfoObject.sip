class InfoObject : QObject
{
%TypeHeaderCode
#include "InfoObject.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a InfoObject passed to a Signal is provided as a QObject instead of
  // a InfoObject object
  // (not sure why, but explicit conversion here fix the problem)
  InfoObject* mc = qobject_cast<InfoObject*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_InfoObject;
  }
%End

    InfoObject(const InfoObject & __0);
public:    
    bool operator==(const InfoObject &other) const;
    bool operator!=(const InfoObject &other) const;
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
