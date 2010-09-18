#ifndef _ANIMATION_TEST_H_
#define _ANIMATION_TEST_H_

#include <QtGui>

class Item : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

    public:
        Item (int index, QGraphicsItem *parent = 0)
            : QObject(0), QGraphicsPixmapItem(parent)
        {
            setPixmap(QPixmap(QString("p%1.jpg").arg(index)));

            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
            effect->setBlurRadius(25.0);
            effect->setOffset(5.0, 5.0);
            setGraphicsEffect(effect);
        }
};

class Animator : public QObject {
    Q_OBJECT

    public:
        Animator (QObject *parent = 0) : QObject(parent) {}

        void addItem (Item *item) { _items.enqueue(item); }

    public Q_SLOTS:
        void animate (void);

    private:
        void pushAnimation (int msec, QAbstractAnimation *animation);

    private:
        QQueue<Item *> _items;
};

#endif /* !_ANIMATION_TEST_H_ */

