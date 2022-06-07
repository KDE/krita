
%ModuleHeaderCode
#include "ColorizeMask.h"
%End

class ColorizeMask : Node
{
%TypeHeaderCode
#include "ColorizeMask.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<ColorizeMask*>(sipCpp))
        sipType = sipType_ColorizeMask;
    else
        sipType = nullptr;
%End

    ColorizeMask(const ColorizeMask & __0);
public:
    virtual ~ColorizeMask();
    virtual QString type() const;

    QList<ManagedColor*> keyStrokesColors() const /Factory/;
    void initializeKeyStrokeColors(QList<ManagedColor*> colors, int transparentIndex = -1);
    void removeKeyStroke(ManagedColor* color);
    int transparencyIndex() const;
    QByteArray keyStrokePixelData(ManagedColor* color, int x, int y, int w, int h) const;
    bool setKeyStrokePixelData(QByteArray value, ManagedColor* color, int x, int y, int w, int h);
    void setUseEdgeDetection(bool value);
    bool useEdgeDetection() const;
    void setEdgeDetectionSize(qreal value);
    qreal edgeDetectionSize() const;
    void setCleanUpAmount(qreal value);
    qreal cleanUpAmount() const;
    void setLimitToDeviceBounds(bool value);
    bool limitToDeviceBounds() const;
    void updateMask(bool force = false);
    void resetCache();
    bool showOutput() const;
    void setShowOutput(bool enabled);
    bool editKeyStrokes() const;
    void setEditKeyStrokes(bool enabled);

Q_SIGNALS:
private:
};

