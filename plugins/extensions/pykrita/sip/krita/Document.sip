class Document : QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "Document.h"
%End
    Document(const Document & __0);
public:
    Node * activeNode() const;
    void setActiveNode(Node*  value);
    bool batchmode() const;
    void setBatchmode(bool value);
    ColorDepth * colorDepth() const;
    void setColorDepth(ColorDepth*  value);
    ColorManager * colorManager() const;
    void setColorManager(ColorManager*  value);
    ColorModel * colorModel() const;
    void setColorModel(ColorModel*  value);
    ColorProfile * colorProfile() const;
    void setColorProfile(ColorProfile*  value);
    InfoObject * documentInfo() const;
    void setDocumentInfo(InfoObject*  value);
    QString fileName() const;
    void setFileName(QString value);
    int height() const;
    void setHeight(int value);
    InfoObject * metaData() const;
    void setMetaData(InfoObject*  value);
    QString name() const;
    void setName(QString value);
    int resolution() const;
    void setResolution(int value);
    Node * rootNode() const;
    Selection * selection() const;
    void setSelection(Selection*  value);
    int width() const;
    void setWidth(int value);
    QByteArray pixelData() const;
public Q_SLOTS:
    bool close();
    bool convert(const QString & colorModel, const ColorProfile*  profile);
    void crop(int x, int y, int w, int h);
    bool exportImage(const QString &filename, const InfoObject & exportConfiguration);
    void flatten();
    void resizeImage(int w, int h);
    bool save();
    bool saveAs(const QString & filename);
    void openView();
    Node * createNode(const QString & name, const QString & nodeType);
private:

};
