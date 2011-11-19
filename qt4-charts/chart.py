from itertools import cycle
from PyQt4.Qt import *

DEFAULT_COLORS = [0x3366cc, 0xdc3912, 0xff9900, 0x109618, 0x990099,
                  0x0099c6, 0xdd4477, 0x66aa00, 0xb82e2e, 0x316395,
                  0x994499, 0x22aa99, 0xaaaa11, 0x6633cc, 0x16d620]

class Chart(object):
    def __init__(self, data, colors=None):
        self.data = data
        self.colors = colors

        self._ref_col = 0
        self._ref_isv = True

    def setVerticalAxisColumn(self, column):
        self._ref_col = column
        self._ref_isv = True

    def setHorizontalAxisColumn(self, column):
        self._ref_col = column
        self._ref_isv = False

    def save(self, filename, chart_size, legend_width=None):
        image_size = chart_size
        if legend_width is not None:
            image_size = image_size + QSize(legend_width, 0)

        image = QImage(image_size, QImage.Format_ARGB32_Premultiplied)
        painter = QPainter(image)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.fillRect(image.rect(), Qt.white)
        self.draw(painter, QRect(QPoint(0, 0), chart_size))
        if legend_width is not None:
            self.drawLegend(painter, QRect(QPoint(chart_size.width(), 10), QSize(legend_width, chart_size.height())))
        painter.end()
        return image.save(filename)

    def draw(self, painter, rectangle):
        raise NotImplementedError

    def drawLegend(self, painter, rectangle):
        SPACE = 2

        font_metrics = painter.fontMetrics()
        size = font_metrics.xHeight() * 2

        y = SPACE
        x0 = SPACE
        x1 = x0 + size + SPACE * 3

        w = rectangle.width() - size - SPACE
        tw = w - x1

        painter.save()
        painter.translate(rectangle.x(), rectangle.y())

        color = self._icolors()
        for i, column in enumerate(self._fetchLegendData()):
            if (y + size + SPACE * 2) >= (rectangle.y() + rectangle.height()) and i < (len(self.data.columns) - 1):
                painter.drawText(x1, y, tw, size, Qt.AlignLeft | Qt.AlignVCenter, "...")
                y += size + SPACE
                break

            text = font_metrics.elidedText(column, Qt.ElideRight, tw)
            painter.fillRect(x0, y, size, size, QColor(next(color)))
            painter.drawText(x1, y, tw, size, Qt.AlignLeft | Qt.AlignVCenter, text)
            y += size + SPACE

        painter.setPen(Qt.lightGray)
        painter.drawRect(0, 0, w, y)
        painter.restore()

    def _fetchLegendData(self):
        for i, column in enumerate(self.data.columns):
            if i != self._ref_col:
                yield column

    def _icolors(self):
        if self.colors is None:
            return cycle(DEFAULT_COLORS)
        return cycle(self.colors)

class PieChart(Chart):
    def draw(self, painter, rectangle):
        painter.save()
        painter.translate(rectangle.x(), rectangle.y())

        # Calculate Values
        vtotal = float(sum(row[not self._ref_col] for row in self.data.rows))
        values = [row[not self._ref_col] / vtotal for row in self.data.rows]

        # Draw Char
        start_angle = 90 * 16
        for color, v in zip(self._icolors(), values):
            span_angle = v * -360.0 * 16
            painter.setPen(Qt.white)
            painter.setBrush(QColor(color))
            painter.drawPie(rectangle, start_angle, span_angle)
            start_angle += span_angle

        painter.restore()

    def _fetchLegendData(self):
        for row in self.data.rows:
            yield row[self._ref_col]

class ScatterChart(Chart):
    SPAN = 10

    def __init__(self, data, **kwargs):
        super(ScatterChart, self).__init__(data, **kwargs)

        self.haxis_title = None
        self.haxis_vmin = None
        self.haxis_vmax = None
        self.haxis_step = None
        self.haxis_grid = True

        self.vaxis_title = None
        self.vaxis_vmin = None
        self.vaxis_vmax = None
        self.vaxis_step = None
        self.vaxis_grid = True

    def draw(self, painter, rectangle):
        self._setupDefaultValues()

        font_metrics = painter.fontMetrics()
        h = font_metrics.xHeight() * 2
        x = font_metrics.width(self._vToString(self.vaxis_vmax))

        # Calculate X steps
        nxstep = int(round((self.haxis_vmax - self.haxis_vmin) / self.haxis_step))
        nystep = int(round((self.vaxis_vmax - self.vaxis_vmin) / self.vaxis_step))

        # Calculate chart space
        xmin = h + self.SPAN + x
        xstep = (rectangle.width() - xmin - (h + self.SPAN)) / nxstep
        xmax = xmin + xstep * nxstep

        # Calculate Y steps
        ymin = h + self.SPAN
        ystep = (rectangle.height() - ymin - (h * 2 + self.SPAN)) / nystep
        ymax = ymin + ystep * nystep

        painter.save()
        painter.translate(rectangle.x(), rectangle.y())

        # Draw Axis Titles
        painter.save()
        self._drawAxisTitles(painter, xmin, xmax, ymin, ymax)
        painter.restore()

        # Draw Axis Labels
        painter.save()
        self._drawAxisLabels(painter, xmin, xmax, ymin, ymax, xstep, nxstep, ystep, nystep)
        painter.restore()

        # Draw Data
        painter.save()
        painter.setClipRect(xmin + 1, ymin, xmax - xmin - 2, ymax - ymin - 1)
        self._drawData(painter, xmin, xmax, ymin, ymax)
        painter.restore()

        # Draw Border
        painter.setPen(Qt.black)
        painter.drawLine(xmin, ymin, xmin, ymax)
        painter.drawLine(xmin, ymax, xmax, ymax)

        painter.restore()

    def _drawAxisTitles(self, painter, xmin, xmax, ymin, ymax):
        font_metrics = painter.fontMetrics()
        h = font_metrics.xHeight() * 2
        hspan = self.SPAN / 2

        font = painter.font()
        font.setItalic(True)
        painter.setFont(font)

        if self.haxis_title is not None:
            painter.drawText(xmin + ((xmax - xmin) / 2 - font_metrics.width(self.haxis_title) / 2), ymax + h * 2 + hspan, self.haxis_title)

        if self.vaxis_title is not None:
            painter.rotate(90)
            painter.drawText(ymin + (ymax - ymin) / 2 - font_metrics.width(self.vaxis_title) / 2, -hspan, self.vaxis_title)

    def _drawAxisLabels(self, painter, xmin, xmax, ymin, ymax, xstep, nxstep, ystep, nystep):
        font_metrics = painter.fontMetrics()
        h = font_metrics.xHeight() * 2

        # Draw Internal Grid
        painter.setPen(Qt.lightGray)
        x = xmin + xstep
        ys = ymin if self.haxis_grid else ymax - 4
        for _ in xrange(nxstep):
            painter.drawLine(x, ys, x, ymax)
            x += xstep

        y = ymin
        xe = xmax if self.vaxis_grid else xmin + 4
        for _ in xrange(nystep):
            painter.drawLine(xmin, y, xe, y)
            y += ystep

        # Draw Axis Labels
        painter.setPen(Qt.black)
        for i in xrange(1 + nxstep):
            x = xmin + (i * xstep)
            v = self._hToString(self.haxis_vmin + i * self.haxis_step)
            painter.drawText(x - font_metrics.width(v) / 2, 2 + ymax + h, v)

        for i in xrange(1 + nystep):
            y = ymin + (i * ystep)
            v = self._vToString(self.vaxis_vmin + (nystep - i) * self.vaxis_step)
            painter.drawText(xmin - font_metrics.width(v) - 2, y, v)

    def _drawData(self, painter, xmin, xmax, ymin, ymax):
        c = 0
        color = self._icolors()
        while c < len(self.data.columns):
            if c != self._ref_col:
                if self._ref_isv:
                    a, b = c, self._ref_col
                else:
                    a, b = self._ref_col, c
                self._drawColumnData(painter, next(color), a, b, xmin, xmax, ymin, ymax)
            c += 1

    def _drawColumnData(self, painter, color, xcol, ycol, xmin, xmax, ymin, ymax):
        painter.setPen(QPen(QColor(color), 7, Qt.SolidLine, Qt.RoundCap))
        for row in self.data.rows:
            x, y = self._xyFromData(row[xcol], row[ycol], xmin, xmax, ymin, ymax)
            painter.drawPoint(x, y)

    def _xyFromData(self, xdata, ydata, xmin, xmax, ymin, ymax):
        x = xmin + (float(xdata - self.haxis_vmin) / (self.haxis_vmax - self.haxis_vmin)) * (xmax - xmin)
        y = ymin + (1.0 - (float(ydata - self.vaxis_vmin) / (self.vaxis_vmax - self.vaxis_vmin))) * (ymax - ymin)
        return x, y

    def _vToString(self, value):
        if isinstance(self.vaxis_step, float):
            return '%.2f' % value
        if isinstance(self.vaxis_step, int):
            return '%d' % value
        return '%s' % value

    def _hToString(self, value):
        if isinstance(self.haxis_step, float):
            return '%.2f' % value
        if isinstance(self.haxis_step, int):
            return '%d' % value
        return '%s' % value

    def _setupDefaultValues(self):
        def _minMaxDelta(col):
            vmin = None
            vmax = None
            vdelta = 0
            ndelta = 1

            last_value = self.data.rows[0][col]
            vmin = vmax = last_value
            for row in self.data.rows[1:]:
                vdelta += abs(row[col] - last_value)
                ndelta += 1
                if row[col] > vmax:
                    vmax = row[col]
                elif row[col] < vmin:
                    vmin = row[col]

            return vmin, vmax, vdelta / ndelta

        ref_min, ref_max, ref_step = _minMaxDelta(self._ref_col)
        oth_min = oth_max = oth_step = None
        for col in xrange(len(self.data.columns)):
            if col == self._ref_col:
                continue

            cmin, cmax, cstep = _minMaxDelta(col)
            oth_min = cmin if oth_min is None else min(cmin, oth_min)
            oth_max = cmax if oth_max is None else max(cmax, oth_max)
            oth_step = cstep if oth_step is None else (oth_step + cstep) / 2

        if self._ref_isv:
            if self.vaxis_vmin is None: self.vaxis_vmin = ref_min
            if self.vaxis_vmax is None: self.vaxis_vmax = ref_max
            if self.vaxis_step is None: self.vaxis_step = ref_step
            if self.haxis_vmin is None: self.haxis_vmin = oth_min
            if self.haxis_vmax is None: self.haxis_vmax = oth_max
            if self.haxis_step is None: self.haxis_step = oth_step
        else:
            if self.haxis_vmin is None: self.haxis_vmin = ref_min
            if self.haxis_vmax is None: self.haxis_vmax = ref_max
            if self.haxis_step is None: self.haxis_step = ref_step
            if self.vaxis_vmin is None: self.vaxis_vmin = oth_min
            if self.vaxis_vmax is None: self.vaxis_vmax = oth_max
            if self.vaxis_step is None: self.vaxis_step = oth_step

class LineChart(ScatterChart):
    def _drawColumnData(self, painter, color, xcol, ycol, xmin, xmax, ymin, ymax):
        painter.setPen(QPen(QColor(color), 2, Qt.SolidLine, Qt.RoundCap))

        path = QPainterPath()

        row = self.data.rows[0]
        x, y = self._xyFromData(row[xcol], row[ycol], xmin, xmax, ymin, ymax)
        path.moveTo(x, y)

        for row in self.data.rows[1:]:
            x, y = self._xyFromData(row[xcol], row[ycol], xmin, xmax, ymin, ymax)
            path.lineTo(x, y)
            painter.drawPath(path)

class AreaChart(ScatterChart):
    def _drawColumnData(self, painter, color, xcol, ycol, xmin, xmax, ymin, ymax):
        painter.setPen(QPen(QColor(color), 2, Qt.SolidLine, Qt.RoundCap))

        color = QColor(color)
        color.setAlpha(40)
        painter.setBrush(color)

        path = QPainterPath()
        path.moveTo(xmin, ymax)
        for row in self.data.rows:
            x, y = self._xyFromData(row[xcol], row[ycol], xmin, xmax, ymin, ymax)
            path.lineTo(x, y)
        path.lineTo(xmax, ymax)
        path.moveTo(xmin, ymax)
        painter.drawPath(path)

class Viewer(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.graph = None

    def setGraph(self, func):
        self.graph = func
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        if self.graph is not None:
            self.graph.draw(painter, QRect(0, 0, event.rect().width() - 120, event.rect().height()))
            self.graph.drawLegend(painter, QRect(event.rect().width() - 120, 20, 120, event.rect().height() - 20))

        painter.end()

class DialogViewer(QDialog):
   def __init__(self):
        QDialog.__init__(self)
        self.viewer = Viewer()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.layout().addWidget(self.viewer)

   def setGraph(self, func):
        self.viewer.setGraph(func)

class DataTable(object):
    def __init__(self):
        self.columns = []
        self.rows = []

    def addColumn(self, label):
        self.columns.append(label)

    def addRow(self, row):
        assert len(row) == len(self.columns)
        self.rows.append(row)

def _pieChartDemo():
    table = DataTable()
    table.addColumn('Lang')
    table.addColumn('Rating')
    table.addRow(['Java', 17.874])
    table.addRow(['C', 17.322])
    table.addRow(['C++', 8.084])
    table.addRow(['C#', 7.319])
    table.addRow(['PHP', 6.096])

    chart = PieChart(table)
    #chart.save('pie.png', QSize(240, 240), 100)

    view = DialogViewer()
    view.setGraph(chart)
    view.resize(360, 240)
    view.exec_()

def _scatterChartDemo():
    table = DataTable()
    table.addColumn('Quality')
    table.addColumn('Test 1')
    table.addColumn('Test 2')
    table.addRow([ 92,  4.9,  8.0])
    table.addRow([ 94,  2.0,  2.5])
    table.addRow([ 96,  7.2,  6.9])
    table.addRow([ 98,  3.5,  1.2])
    table.addRow([100,  8.0,  5.3])
    table.addRow([102, 15.0, 14.2])

    chart = ScatterChart(table)
    chart.haxis_title = 'Process input'
    chart.haxis_vmin = 0
    chart.haxis_vmax = 16
    chart.haxis_step = 2
    chart.vaxis_title = 'Quality'
    chart.vaxis_vmin = 90
    chart.vaxis_vmax = 104
    chart.vaxis_step = 1

    #chart.save('scatter.png', QSize(400, 240), 100)

    view = DialogViewer()
    view.setGraph(chart)
    view.resize(400, 240)
    view.exec_()

def _lineChartDemo():
    table = DataTable()
    table.addColumn('Time')
    table.addColumn('Site 1')
    table.addColumn('Site 2')
    table.addColumn('Site 3')
    table.addRow([ 4.00, 120,   80,  400])
    table.addRow([ 6.00, 270,  850,  320])
    table.addRow([ 8.30,  50, 1200,  280])
    table.addRow([10.15, 320, 1520,  510])
    table.addRow([12.00, 150,  930, 1100])
    table.addRow([18.20,  62, 1100,  240])

    chart = LineChart(table)
    chart.setHorizontalAxisColumn(0)
    chart.haxis_title = 'Time'
    chart.haxis_vmin = 0.0
    chart.haxis_vmax = 20.0
    chart.haxis_step = 2

    #chart.save('line.png', QSize(400, 240), 100)

    view = DialogViewer()
    view.setGraph(chart)
    view.resize(400, 240)
    view.exec_()

def _areaChartDemo():
    table = DataTable()
    table.addColumn('Time')
    table.addColumn('Site 1')
    table.addColumn('Site 2')
    table.addRow([ 4.00, 120,   500])
    table.addRow([ 6.00, 270,   460])
    table.addRow([ 8.30, 1260, 1120])
    table.addRow([10.15, 2030,  540])
    table.addRow([12.00,  520,  890])
    table.addRow([18.20, 1862, 1500])

    chart = AreaChart(table)
    chart.setHorizontalAxisColumn(0)
    chart.haxis_title = 'Time'
    chart.haxis_vmin = 0.0
    chart.haxis_vmax = 20.0
    chart.haxis_step = 5

    #chart.save('area.png', QSize(400, 240), 100)

    view = DialogViewer()
    view.setGraph(chart)
    view.resize(400, 240)
    view.exec_()

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)

    _pieChartDemo()
    _scatterChartDemo()
    _lineChartDemo()
    _areaChartDemo()
