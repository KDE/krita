class Document : QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "Document.h"
%End
    Document(const Document & __0);
public Q_SLOTS:
    Node * activeNode() const  /Factory/;
    void setActiveNode(Node*  value);
    QList<Node*> toplevelNodes() const  /Factory/;
    Node *nodeByName(const QString &node) const  /Factory/;
    bool batchmode() const;
    void setBatchmode(bool value);
    QString colorDepth() const;
    QString colorModel() const;
    QString colorProfile() const;
    bool setColorProfile(const QString &colorProfile);
    void setColorSpace(const QString &value, const QString &colorDepth, const QString &colorProfile);
    QString documentInfo() const;
    void setDocumentInfo(const QString &document);
    QString fileName() const;
    void setFileName(QString value);
    int height() const;
    void setHeight(int value);
    QString name() const;
    void setName(QString value);
    int resolution() const;
    void setResolution(int value);
    Node * rootNode() const  /Factory/;
    Selection * selection() const  /Factory/;
    void setSelection(Selection*  value);
    int width() const;
    void setWidth(int value);
    double xRes() const;
    void setXRes(double xRes) const;

    double yRes() const;
    void setyRes(double yRes) const;
    QByteArray pixelData(int x, int y, int w, int h) const;
    bool close();
    void crop(int x, int y, int w, int h);
    bool exportImage(const QString &filename, const InfoObject & exportConfiguration);
    void flatten();
    void resizeImage(int w, int h);
    bool save();
    bool saveAs(const QString & filename);
    Node *createNode(const QString & name, const QString & nodeType)  /Factory/;

Q_SIGNALS:
    void nodeCreated(Node *node);
private:

};
