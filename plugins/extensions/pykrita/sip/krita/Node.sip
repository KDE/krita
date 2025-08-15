class Node : QObject
{
%TypeHeaderCode
#include "Node.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Node passed to a Signal is provided as a QObject instead of
  // a Node object
  // (not sure why, but explicit conversion here fix the problem)
  Node* mc = qobject_cast<Node*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Node;
  }
%End

    Node(const Node & __0);
public:
    virtual ~Node();
    bool operator==(const Node &other) const;
    bool operator!=(const Node &other) const;
public Q_SLOTS:
    //QList<Shape *> shapes() const /Factory/;
    Node *clone() const /Factory/;
    bool alphaLocked() const;
    void setAlphaLocked(bool value);
    QString blendingMode() const;
    void setBlendingMode(QString value);
    QList<Channel *> channels() const /Factory/;
    QList<Node *> childNodes() const /Factory/;
    QList<Node *> findChildNodes(const QString &name = QString(), bool recursive = false, bool partialMatch = false, const QString &type = QString(), int colorLabelIndex = 0) const /Factory/;
    bool addChildNode(Node *child, Node *above);
    bool removeChildNode(Node *child);
    void setChildNodes(QList<Node *> nodes);
    QString colorDepth() const;
    bool animated() const;
    void enableAnimation() const;
    //void setPinnedToTimeline(bool pinned) const;
    //bool isPinnedToTimeline() const;
    void setCollapsed(bool collapsed);
    bool collapsed() const;
    int colorLabel() const;
    void setColorLabel(int value);
    QString colorModel() const;
    QString colorProfile() const;
    bool setColorProfile(const QString &colorProfile);
    bool setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);
    bool inheritAlpha() const;
    void setInheritAlpha(bool value);
    bool locked() const;
    void setLocked(bool value);
    bool hasExtents();
    QString name() const;
    void setName(QString value);
    int opacity() const;
    void setOpacity(int value);
    Node * parentNode() const  /Factory/;
    QString type() const;
    QIcon icon() const;
    bool visible() const;
    bool hasKeyframeAtTime(int frameNumber);
    void setVisible(bool value);
    QByteArray pixelData(int x, int y, int w, int h) const;
    QByteArray pixelDataAtTime(int x, int y, int w, int h, int time) const;
    QByteArray projectionPixelData(int x, int y, int w, int h) const;
    bool setPixelData(QByteArray value, int x, int y, int w, int h);
    QRect bounds() const;
    void move(int x, int y);
    QPoint position() const;
    bool remove();
    Node *duplicate() /Factory/;
    void save(const QString &filename, double xRes, double yRes, const InfoObject & exportConfiguration, const QRect &exportRect = QRect());
    Node *mergeDown() /Factory/;
    void scaleNode(QPointF origin, int width, int height, QString strategy);
    void rotateNode(double radians);
    void cropNode(int x, int y, int w, int h);
    void shearNode(double angleX, double angleY);
    QImage thumbnail(int w, int h);
    QString layerStyleToAsl();
    bool setLayerStyleFromAsl(const QString &asl);
    int index() const;
    QUuid uniqueId() const;

    void paintLine(const QPoint pointOne, const QPoint pointTwo, double pressureOne = 1.0, double pressureTwo = 1.0, const QString strokeStyle = "ForegroundColor");
    void paintRectangle(const QRectF &rect, const QString strokeStyle = "ForegroundColor", const QString fillStyle = "None");
    void paintPolygon(const QList<QPointF> points, const QString strokeStyle = "ForegroundColor", const QString fillStyle = "None");
    void paintEllipse(const QRectF &rect, const QString strokeStyle = "ForegroundColor", const QString fillStyle = "None");
    void paintPath(const QPainterPath &path, const QString strokeStyle = "ForegroundColor", const QString fillStyle = "None");
    QString paintAbility();
Q_SIGNALS:
private:
};
