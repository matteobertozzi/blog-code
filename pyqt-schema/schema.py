#!/usr/bin/env python
# ----------------------------------------------------------------------------
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# ----------------------------------------------------------------------------
# Author: Matteo Bertozzi <theo.bertozzi@gmail.com>
# Site: http://th30z.netsons.org
# ----------------------------------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtXml import *

import sys

def loadXmlDocument(filename):
    """Load Xml DomDocument from file"""
    fobject = QFile(filename)
    if not fobject.open(QIODevice.ReadOnly):
        return None

    xml = QDomDocument()
    if not xml.setContent(fobject):
        xml = None
    
    fobject.close()
    return xml

def nodeAttributesToDict(node):
    """Convert Node Attributes to Dict; Rembember that keys are QString!"""
    attributes = {}
    node_attr = node.attributes()
    for i in xrange(node_attr.count()):
        attr = node_attr.item(i)
        attributes[attr.nodeName()] = attr.nodeValue()
    return attributes

def _elementById(self, element_id):
    """
    QDomDocument.elementById is not implemented!
    This is a stupid implementation!
    """
    n = self.documentElement().firstChild()
    while not n.isNull():
        element = n.toElement()
        node = element.attributeNode('id')
        if node.nodeValue() == element_id:
            return element
        n = n.nextSibling()
    return None
QDomDocument.elementById = _elementById

class SchemaDrawer(object):
    def __init__(self, xml):
        self.xml = xml

    def draw(self):
        doc_attr = nodeAttributesToDict(self.xml.documentElement())

        width, _ = doc_attr[QString('width')].toInt()
        height, _ = doc_attr[QString('height')].toInt()

        image = QImage(QSize(width, height), QImage.Format_ARGB32_Premultiplied)
        painter = QPainter(image)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setRenderHint(QPainter.TextAntialiasing)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)

        # Draw in order Item, Linee, Text...
        node_types = [('item', self._drawItem),                      
                      ('line', self._drawLine),
                      ('text', self._drawText)]

        for node_type, node_drawer in node_types:
            item_list = xml.elementsByTagName(node_type)
            for i in xrange(item_list.count()):
                node_drawer(painter, item_list.at(i).toElement())

        painter.end()   
        return image

    def _drawItem(self, painter, node):
        node_attr = nodeAttributesToDict(node)
        node_type = self._getStringAttr(node_attr, 'type', None)

        x, y, width, height = self._getObjectRetangle(node_attr)

        border_size, border_color = node_attr[QString('border')].split('/')
        border_color = self._parseColorString(border_color)
        border_size, _ = border_size.toInt()

        colors = node_attr[QString('color')].split('/')
        gradient = QLinearGradient(x, y, x, y + height)
        for i in xrange(colors.count()):
            gradient.setColorAt(i, self._parseColorString(colors[i]))

        painter.setBrush(gradient)

        if border_size > 0:
            painter.setPen(QPen(border_color, border_size))
        else:
            painter.setPen(Qt.NoPen)

        if node_type == 'rounded-rectangle':
            painter.drawRoundedRect(x, y, width, height, 10, 10)
        else:
            painter.drawRect(x, y, width, height)

    def _drawText(self, painter, node):
        node_attr = nodeAttributesToDict(node)

        rect = QRect(*self._getObjectRetangle(node_attr))
        color = self._getColorAttr(node_attr, 'color', '0x000000')
        text = node.text().trimmed()

        font = QFont(painter.font())
        font.setFamily(self._getStringAttr(node_attr, 'font', font.family()))
        font.setPixelSize(self._getIntAttr(node_attr, 'size', 10))
        font.setItalic(self._getBoolAttr(node_attr, 'italic', False))
        font.setBold(self._getBoolAttr(node_attr, 'bold', False))

        painter.setFont(font)
        painter.setPen(QPen(color, 1))
        painter.drawText(rect, Qt.AlignCenter | Qt.TextWordWrap, text)

    def _drawLine(self, painter, node):
        node_attr = nodeAttributesToDict(node)
        color = self._getColorAttr(node_attr, 'color', '0x000000')
        border = self._getStringAttr(node_attr, 'border', 0)

        line_path = self._getStringAttr(node_attr, 'path', None).split(';')       

        path = QPainterPath()
        path.moveTo(self._parsePointString(line_path[0]))
        for p in line_path:
            path.lineTo(self._parsePointString(p))

        painter.setPen(QPen(color, border))
        painter.setBrush(Qt.transparent)
        painter.drawPath(path)

    def _getObjectRetangle(self, node_attr):
        height = self._getIntAttr(node_attr, 'height', 0)
        width = self._getIntAttr(node_attr, 'width', 0)
        x = self._getIntAttr(node_attr, 'x', 0)
        y = self._getIntAttr(node_attr, 'y', 0)
        return x, y, width, height

    def _parseColorString(self, color):
        r, _ = color[2:4].toInt(16)
        g, _ = color[4:6].toInt(16)
        b, _ = color[6:8].toInt(16)
        a = 0xff
        if len(color) > 8:
            a, _ = color[8:10].toInt(16)
        return QColor(r, g, b, a)

    def _parsePointString(self, point):
        x, y = point.split(',')
        x, _ = x.toInt()
        y, _ = y.toInt()
        return QPointF(x, y)

    def _getIntAttr(self, node, attr, default):
        value, _ = node.get(QString(attr), QString(str(default))).toInt()
        return value

    def _getBoolAttr(self, node, attr, default):
        value = node.get(QString(attr), QString(str(default)))        
        return value.trimmed().toLower() == 'true'

    def _getStringAttr(self, node, attr, default=None):
        return node.get(QString(attr), default)

    def _getColorAttr(self, node, attr, default):
        return self._parseColorString(node.get(QString(attr), QString(default)))

if __name__ == '__main__':
    if len(sys.argv) < 3 or '--help' in sys.argv:
        print 'Usage:'
        print '     schema.py <input> <output>'
        print '     schema.py <input> --display'
        sys.exit(1)

    app = QApplication(sys.argv)

    xml = loadXmlDocument(sys.argv[-2])
    if not xml:
        print 'Failed to open', sys.argv[-2]
        sys.exit(1)

    drawer = SchemaDrawer(xml)
    image = drawer.draw()

    if sys.argv[-1] == '--display':
        name = QFileInfo(sys.argv[-2]).fileName()

        w = QLabel()    
        w.setWindowTitle(QString('Schema View - %1').arg(name))
        w.setPixmap(QPixmap.fromImage(image))
        w.show()

        app.exec_()
    else:
        print 'Saving output on', sys.argv[-1]
        image.save(sys.argv[-1])

