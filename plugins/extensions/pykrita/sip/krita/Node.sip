class Node : QObject
{
%TypeHeaderCode
#include "Node.h"
%End
    Node(const Node & __0);
public:
    virtual ~Node();
public Q_SLOTS:
    bool alphaLocked() const;
    void setAlphaLocked(bool value);
    QString blendingMode() const;
    void setBlendingMode(QString value);
    QList<Channel *> channels() const;
    QList<Node *> childNodes() const;
    void setChildNodes(QList<Node *> value);
    QString colorDepth() const;
    QString colorLabel() const;
    void setColorLabel(QString value);
    QString colorModel() const;
    QString colorProfile() const;
    void setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);
    bool inheritAlpha() const;
    void setInheritAlpha(bool value);
    bool locked() const;
    void setLocked(bool value);
    QString name() const;
    void setName(QString value);
    int opacity() const;
    void setOpacity(int value);
    Node * parentNode() const;
    void setParentNode(Node*  value);
    QString type() const;
    void setType(QString value);
    bool visible() const;
    void setVisible(bool value);
    InfoObject * metaDataInfo() const;
    void setMetaDataInfo(InfoObject*  value);
    Generator * generator() const;
    void setGenerator(Generator*  value);
    Filter * filter() const;
    void setFilter(Filter*  value);
    Transformation * transformation() const;
    void setTransformation(Transformation*  value);
    QString fileName() const;
    void setFileName(QString value);
    QByteArray pixelData(int x, int y, int w, int h) const;
    QByteArray projectionPixelData(int x, int y, int w, int h) const;
    void setPixelData(QByteArray value, int x, int y, int w, int h);
    QRect bounds() const;
    void move(int x, int y);
    void moveToParent(Node*  parent);
    void remove();
    Node *duplicate();
    void save(const QString &filename, double xRes, double yRes);
Q_SIGNALS:
private:
};
