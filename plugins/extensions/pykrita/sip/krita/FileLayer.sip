%ModuleHeaderCode
#include "FileLayer.h"
%End

class FileLayer : Node
{
%TypeHeaderCode
#include "FileLayer.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<FileLayer*>(sipCpp))
        sipType = sipType_FileLayer;
    else
        sipType = nullptr;
%End

    FileLayer(const FileLayer & __0);
public:
    virtual ~FileLayer();
    virtual QString type() const;
    void setProperties(QString FileName, QString ScalingMethod);
    void resetCache();
    QString scalingMethod() const;
    QString path() const;
Q_SIGNALS:
private:
};
