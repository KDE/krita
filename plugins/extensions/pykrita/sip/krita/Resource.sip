class Resource : QObject
{
%TypeHeaderCode
#include "Resource.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Resource passed to a Signal is provided as a QObject instead of
  // a Resource object
  // (not sure why, but explicit conversion here fix the problem)
  Resource* mc = qobject_cast<Resource*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Resource;
  }
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
