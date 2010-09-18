#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QApplication>
#include <QPainter>

#define CROP_BORDER_LINE        10
#define CROP_GRID_SIZE           3

class THGraphicsCropItem : public QGraphicsItem {
    public:
	    THGraphicsCropItem(QGraphicsItem *parent) 
	        : QGraphicsItem(parent)
	    {
            _mousePress = false;
            _cropRect = QRectF(20, 20, 100, 70);
            grabMouse();
	    }

        QRectF cropRect (void) const {
            return(_cropRect);
        }

    public:
	    QRectF boundingRect (void) const {
	        return(parentItem()->boundingRect());
	    }

	    void paint (QPainter *painter, 
		            const QStyleOptionGraphicsItem *option,
		            QWidget *widget = 0)
	    {
            Q_UNUSED(widget)

	        painter->save();

            QPainterPath cropPath;
            cropPath.addRect(_cropRect);

            QPainterPath windowPath;
            windowPath.addRect(option->rect);
            windowPath -= cropPath;

	        // Draw Alpha-Black Background.
	        painter->fillPath(windowPath, QColor(0x33, 0x33, 0x33, 0xcc));

            // Draw Crop Rect
            painter->setPen(QPen(QColor(0xdd, 0xdd, 0xdd), 1));
            painter->drawPath(cropPath);            

            int topRightX = _cropRect.x() + _cropRect.width();
            int bottomY = _cropRect.y() + _cropRect.height();

            // Draw Grid if mouse is Pressed
            if (_mousePress) {
                qreal f = 1.0f / CROP_GRID_SIZE;
                qreal hsize = _cropRect.height() * f;
                qreal wsize = _cropRect.width() * f;

                QPainterPath gridPath;
                for (uint i = 1; i < CROP_GRID_SIZE; ++i) {
                    qreal y = _cropRect.y() + i * hsize;
                    gridPath.moveTo(_cropRect.x(), y);
                    gridPath.lineTo(topRightX, y);

                    for (uint j = 1; j < CROP_GRID_SIZE; ++j) {
                        qreal x = _cropRect.x() + j * wsize;
                        gridPath.moveTo(x, _cropRect.y());
                        gridPath.lineTo(x, bottomY);
                    }
                }

                // Draw Grid Path
                painter->setPen(QPen(QColor(0x99, 0x99, 0x99, 0x80), 1));
                painter->drawPath(gridPath);
            }

            QPainterPath borderPath;
            // Top-Left Corner
            borderPath.moveTo(_cropRect.x(), _cropRect.y());
            borderPath.lineTo(_cropRect.x() + CROP_BORDER_LINE, _cropRect.y());
            borderPath.moveTo(_cropRect.x(), _cropRect.y());
            borderPath.lineTo(_cropRect.x(), _cropRect.y() + CROP_BORDER_LINE);

            // Top-Right Corner
            borderPath.moveTo(topRightX - CROP_BORDER_LINE, _cropRect.y());
            borderPath.lineTo(topRightX, _cropRect.y());
            borderPath.moveTo(topRightX, _cropRect.y());
            borderPath.lineTo(topRightX, _cropRect.y() + CROP_BORDER_LINE);

            // Bottom-Left Corner
            borderPath.moveTo(_cropRect.x(), bottomY);
            borderPath.lineTo(_cropRect.x() + CROP_BORDER_LINE, bottomY);
            borderPath.moveTo(_cropRect.x(), bottomY - CROP_BORDER_LINE);
            borderPath.lineTo(_cropRect.x(), bottomY);

            // Bottom-Left Corner
            borderPath.moveTo(topRightX, bottomY);
            borderPath.lineTo(topRightX - CROP_BORDER_LINE, bottomY);
            borderPath.moveTo(topRightX, bottomY - CROP_BORDER_LINE);
            borderPath.lineTo(topRightX, bottomY);

            // Draw Border Path
            painter->setPen(QPen(QColor(0xee, 0xee, 0xee), 3));
            painter->drawPath(borderPath);

	        painter->restore();
	    }

    protected:
        void mousePressEvent (QGraphicsSceneMouseEvent *event) {
            QGraphicsItem::mousePressEvent(event);

            _mousePress = true;
            _cropResize = CropItemResizeNone;
            if (event->buttons() & Qt::LeftButton) {
                int lthreshold = (CROP_BORDER_LINE >> 1);
                int rthreshold = CROP_BORDER_LINE;
                int size = CROP_BORDER_LINE << 1;

                int rightX = _cropRect.x() + _cropRect.width() - rthreshold;
                int leftX = _cropRect.x() - lthreshold;

                int bottomY = _cropRect.y() + _cropRect.height() - rthreshold;
                int topY = _cropRect.y() - lthreshold;

                QRectF bottomRightCorner(rightX, bottomY, size, size);
                QRectF bottomLeftCorner(leftX, bottomY, size, size);
                QRectF topRightCorner(rightX, topY, size, size);
                QRectF topLeftCorner(leftX, topY, size, size);

                if (bottomRightCorner.contains(event->pos()))
                    _cropResize = CropItemResizeBottomRight;
                else if (bottomLeftCorner.contains(event->pos()))
                    _cropResize = CropItemResizeBottomLeft;
                else if (topRightCorner.contains(event->pos()))
                    _cropResize = CropItemResizeTopRight;
                else if (topLeftCorner.contains(event->pos()))
                    _cropResize = CropItemResizeTopLeft;
            }

            update();
        }

        void mouseReleaseEvent (QGraphicsSceneMouseEvent *event) {
            QGraphicsItem::mouseReleaseEvent(event);

            _cropResize = CropItemResizeNone;
            _mousePress = false;

            update();
        }

        void mouseMoveEvent (QGraphicsSceneMouseEvent *event) {
            QGraphicsItem::mouseMoveEvent(event);

            qreal minSize = 4 + (CROP_BORDER_LINE << 1);

            QPointF delta = event->pos() - event->lastPos();
            switch (_cropResize) {
                case CropItemResizeNone:
                    if (!_cropRect.contains(event->pos()))
                        return;

                    if (!(event->buttons() & Qt::LeftButton))
                        return;

                    _cropRect.translate(delta);
                    break;
                case CropItemResizeTopLeft:
                    delta.setY(qMin(_cropRect.height() - minSize, delta.y()));
                    delta.setX(qMin(_cropRect.width() - minSize, delta.x()));
                    _cropRect.setHeight(_cropRect.height() - delta.y());
                    _cropRect.setWidth(_cropRect.width() - delta.x()); 
                    _cropRect.translate(delta);
                    break;
                case CropItemResizeTopRight:
                    delta.setY(qMin(_cropRect.height() - minSize, delta.y()));
                    _cropRect.setWidth(_cropRect.width() + delta.x());
                    _cropRect.setHeight(_cropRect.height() - delta.y());
                    _cropRect.translate(0, delta.y());
                    break;
                case CropItemResizeBottomLeft:
                    delta.setX(qMin(_cropRect.width() - minSize, delta.x()));
                    _cropRect.setHeight(_cropRect.height() + delta.y());
                    _cropRect.setWidth(_cropRect.width() - delta.x());
                    _cropRect.translate(delta.x(), 0);
                    break;
                case CropItemResizeBottomRight:
                    _cropRect.setWidth(_cropRect.width() + delta.x()); 
                    _cropRect.setHeight(_cropRect.height() + delta.y());
                    break;
            }

            if (_cropRect.width() < minSize) _cropRect.setWidth(minSize);
            if (_cropRect.height() < minSize) _cropRect.setHeight(minSize);

            update();
        }

    private:
        enum CropItemResize {
            CropItemResizeNone,
            CropItemResizeTopLeft,
            CropItemResizeTopRight,
            CropItemResizeBottomLeft,
            CropItemResizeBottomRight,
        };

    private:
        CropItemResize _cropResize;
        QRectF _cropRect;
        bool _mousePress;
};

int main (int argc, char **argv) {
    QApplication app(argc, argv);

    QGraphicsScene scene;
    QGraphicsPixmapItem *item = scene.addPixmap(QPixmap("image.jpg"));

    new THGraphicsCropItem(item);

    QGraphicsView view(&scene);
    view.setWindowTitle("Image Crop");
    view.show();

    return(app.exec());
}

