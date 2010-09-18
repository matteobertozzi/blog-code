#include "test.h"

#define NITEMS      (5)
#define TIMERS      (1000)
#define TIMER0      (550)
#define TIMER1      (500)
#define ITEMW       (140)

void Animator::animate (void) {
    QPropertyAnimation *anim;
    int msec = TIMERS;
    QPointF lastPos;
    int duration;
    Item *item;
    qreal dx;

    // Calculate Item X Shift and store the last item position
    dx = (_items.size() > 1) ? (_items[1]->x() - _items[0]->x()) : 0.0;
    lastPos = _items.last()->pos();

    // Pop first Item, Fade Out and re-enqueue
    item = _items.dequeue();
    anim = new QPropertyAnimation(item, "opacity");
    anim->setDuration(TIMER0);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    _items.enqueue(item);
    duration = msec + TIMER0;
    pushAnimation(msec, anim);

    // Shift Items
    for (int i = 0, n = _items.size() - 1; i < n; ++i) {
        item = _items[i];
        anim = new QPropertyAnimation(item, "pos");
        anim->setDuration(TIMER1);
        anim->setStartValue(item->pos());
        anim->setEndValue(item->pos() - QPointF(dx, 0.0));
        anim->setEasingCurve(QEasingCurve::OutBack);

        msec += TIMER1 / 2;
        duration += TIMER1;
        pushAnimation(msec, anim);
    }

    // Now it's time for a parallel animation...
    // Fade In and enter from the right for the last item.
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    item = _items.last();

    // Old First Item comes in from the right
    anim = new QPropertyAnimation(item, "pos");
    anim->setDuration(TIMER0);
    anim->setStartValue(lastPos + QPointF(dx / 3.0, 0.0));
    anim->setEndValue(lastPos);
    anim->setEasingCurve(QEasingCurve::OutBack);
    group->addAnimation(anim);

    // Fade In old First Item in the last position
    anim = new QPropertyAnimation(item, "opacity");
    anim->setDuration(TIMER0);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    group->addAnimation(anim);

    msec += TIMER0;
    duration += TIMER0;
    pushAnimation(msec, group);

    // Do Loop! Call again the animate()
    msec += TIMER0;
    QTimer::singleShot(msec, this, SLOT(animate()));
}

void Animator::pushAnimation (int msec, QAbstractAnimation *animation) {
    QTimer::singleShot(msec, animation, SLOT(start()));
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
}

int main (int argc, char **argv) {
    Animator animator;

    QApplication app(argc, argv);

    QGraphicsScene scene;
    scene.setSceneRect(0, 0, 800, 250);

    QGraphicsView view(&scene);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Add Items to the scene
    for (int i = 0; i < NITEMS; ++i) {
        Item *item = new Item(i);
        item->setPos((i * (ITEMW + 10)) + 25, 25);
        scene.addItem(item);
        animator.addItem(item);
    }

    // Run Animation Loop!
    QTimer::singleShot(1000, &animator, SLOT(animate()));

    view.setWindowTitle("Qt Animation Framework");
    view.show();

    return(app.exec());
}

