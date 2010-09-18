#!/usr/bin/env python
#-*- coding: utf-8 -*-

from PyQt4.Qt import *
import math

class Oscillator(object):
    """
    http://en.wikipedia.org/wiki/Sine_wave
    http://en.wikipedia.org/wiki/Square_wave
    http://en.wikipedia.org/wiki/Triangle_wave
    http://en.wikipedia.org/wiki/Sawtooth_wave
    """
    WAVE_SINE = 0
    WAVE_SQUARE = 1
    WAVE_TRIANGLE = 2
    WAVE_SAW_TOOTH = 3
    WAVE_REVERSE_SAW_TOOTH = 4

    def __init__(self, sampleRate, frequency, waveType):
        self.sampleRate = sampleRate
        self.frequency = frequency
        self.waveType = waveType

        self._nsample = 0

    def setFrequency(self, frequency):
        self.frequency = frequency

    def setWaveType(self, waveType):
        self.waveType = waveType

    def value(self):
        periodSamples = self.sampleRate // self.frequency
        x = float(self._nsample) / periodSamples
        self._nsample = (self._nsample + 1) % periodSamples

        if self.waveType == self.WAVE_SINE:
            return math.sin(2.0 * 3.14 * x)
        if self.waveType == self.WAVE_SQUARE:
            return 1.0 if (self._nsample < periodSamples // 2) else -1.0
        if self.waveType == self.WAVE_TRIANGLE:
            return 2.0 * abs(2.0 * x - 2.0 * math.floor(x) - 1.0) - 1.0
        if self.waveType == self.WAVE_SAW_TOOTH:
            return 2.0 * (x - math.floor(x) - 0.5)
        if self.waveType == self.WAVE_REVERSE_SAW_TOOTH:
            return 2.0 * (math.floor(x) - x + 0.5)

        raise Exception('Invalid Wave Type')

class WaveViewer(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self._values = []

    def addValue(self, value):
        # Clean up a bit... has someone resolution of 4096px? :)
        if len(self._values) > 8192:
            self._values = self._values[4096:]

        self._values.append(value)
        self.update()

    def paintEvent(self, event):
        QWidget.paintEvent(self, event)
        geom = self.geometry()

        width = geom.width() - 1
        height = geom.height() - 1
        step = height * 0.5

        # Calculate x, y position of specified value at index
        def calcPoint(index):
            value = self._values[-(index + 1)]
            return QPointF(width - index, height - (value * height))

        p = QPainter(self)

        # Build path only if there's more than one value.
        # Take the newest values, based on window width.
        n = min(len(self._values), geom.width()) - 1
        path = QPainterPath()
        if n > 0:
            path.moveTo(calcPoint(1))
            for i in xrange(n - 2):
                path.lineTo(calcPoint(i + 1))

        # Draw ref lines
        p.setPen(Qt.lightGray)
        p.drawLine(0, 0, width, 0)
        p.drawLine(0, step, width, step)
        p.drawLine(0, height, width, height)

        # Draw Line Captions
        p.setPen(Qt.gray)
        p.drawText(0, 1, 50, 25, Qt.AlignLeft | Qt.AlignTop, '+1.0')
        p.drawText(0, step + 1, 50, 25, Qt.AlignLeft | Qt.AlignTop, ' 0.0')
        p.drawText(0, height - 25, 50, 25, Qt.AlignLeft | Qt.AlignBottom, '-1.0')

        # Draw Wave Path
        p.setPen(Qt.black)
        p.drawPath(path)

        p.end()

class OscillatorViewer(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)

        self._oscillator = Oscillator(44100, 250, Oscillator.WAVE_SINE)

        layout = QVBoxLayout()
        self.setLayout(layout)

        self._viewer = WaveViewer()
        layout.addWidget(self._viewer)

        hlayout = QHBoxLayout()
        layout.addLayout(hlayout)

        wave_combo = QComboBox()
        wave_combo.addItem('Sine', Oscillator.WAVE_SINE)
        wave_combo.addItem('Square', Oscillator.WAVE_SQUARE)
        wave_combo.addItem('Triangle', Oscillator.WAVE_TRIANGLE)
        wave_combo.addItem('Saw Tooth', Oscillator.WAVE_SAW_TOOTH)
        wave_combo.addItem('Reversed Saw Tooth', Oscillator.WAVE_REVERSE_SAW_TOOTH)
        QObject.connect(wave_combo, SIGNAL('currentIndexChanged(int)'),
                        lambda x: self._oscillator.setWaveType(wave_combo.itemData(x).toInt()[0]))

        freq_spinbox = QSpinBox()
        freq_spinbox.setSuffix("Hz")
        freq_spinbox.setRange(1, 2500)
        freq_spinbox.setValue(self._oscillator.frequency)
        QObject.connect(freq_spinbox, SIGNAL('valueChanged(int)'),
                        lambda x: self._oscillator.setFrequency(x))

        hlayout.addWidget(QLabel("Waves:"))
        hlayout.addWidget(wave_combo)
        hlayout.addWidget(QLabel("Frequency:"))
        hlayout.addWidget(freq_spinbox)
        hlayout.addStretch(1)

        self._timer = self.startTimer(10)

    def timerEvent(self, event):
        if event.timerId() == self._timer:
            # Request next Wave Sample, and convert to 0.0-1.0 space.
            # Returned Wave Samples are between -1.0 and 1.0.
            self._viewer.addValue((self._oscillator.value() + 1.0) * 0.5)
        QWidget.timerEvent(self, event)

if __name__ == '__main__':
    app = QApplication([])

    w = QMainWindow()
    w.setCentralWidget(OscillatorViewer())
    w.setWindowTitle('PyQt4 Oscillator')
    w.resize(560, 320)
    w.show()

    app.exec_()
