#include <unistd.h>
#include <signal.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include "udpaudio.h"

#define AUDIO_UDP_PORT    55455

UdpAudioReceiver::UdpAudioReceiver (const QAudioDeviceInfo& audioDevice,
                                    const QAudioFormat& format,
                                    QObject *parent)
    : QIODevice(parent), m_audio(audioDevice, format)
{
#ifdef UDP_AUDIO_RECEIVER_DEBUG
    connect(&m_audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChanged(QAudio::State)));
#endif

    m_udpSocket = new QUdpSocket();
    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(processDatagrams()));

    open(QIODevice::ReadOnly);
}

UdpAudioReceiver::~UdpAudioReceiver() {
    qDebug() << "DESTROY";
    m_audio.stop();
    close();
    delete m_udpSocket;
}

bool UdpAudioReceiver::bind (const QHostAddress& address, quint16 port) {
#ifdef UDP_AUDIO_RECEIVER_DEBUG
    qDebug() << "UdpAudioReceiver::bind()" << address << port;
#endif

    if (!m_udpSocket->bind(address, port)) {
        setErrorString(m_udpSocket->errorString());
        return(false);
    }

    m_audio.start(this);
    return(true);
}

qint64 UdpAudioReceiver::bytesAvailable() const {
#ifdef UDP_AUDIO_RECEIVER_DEBUG
    qDebug() << "UdpAudioReceiver.bytesAvailable()" << m_buffer.size();
#endif
    return m_buffer.size();
}

qint64 UdpAudioReceiver::readData(char *data, qint64 maxlen) {
    maxlen = qMin((quint64)m_buffer.size(), (quint64)maxlen);

    if (maxlen == 0) {
        // HACK: To dalay audio.suspend() we sleep a bit.
        usleep(1000000);
        processDatagrams();
        maxlen = qMin((quint64)m_buffer.size(), (quint64)maxlen);
    }

#ifdef UDP_AUDIO_RECEIVER_DEBUG
    qDebug() << "UdpAudioReceiver.readData()" << maxlen << m_buffer.size();
#endif

    if (maxlen > 0) {
        memcpy(data, m_buffer.constData(), maxlen);
        m_buffer.remove(0, maxlen);
        return maxlen;
    }

    m_audio.suspend();
    return 0;
}

#ifdef UDP_AUDIO_RECEIVER_DEBUG
void UdpAudioReceiver::stateChanged (QAudio::State state) {
    qDebug() << "UdpAudioReceiver.stateChanged()" << state;
}
#endif

qint64 UdpAudioReceiver::writeData(const char *data, qint64 len) {
    Q_UNUSED(data)
    Q_UNUSED(len)
    return(0);
}

void UdpAudioReceiver::processDatagrams (void)
{
    QByteArray datagram;

    while (m_udpSocket->hasPendingDatagrams()) {
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());

        m_buffer.append(datagram);
    }

    if (m_audio.state() == QAudio::SuspendedState)
        m_audio.resume();

    emit readyRead();
}

static void __sig_trap (int signum) {
    QCoreApplication::instance()->exit();
}

#define USE_UDP_STREAM                  (1)
int main (int argc, char **argv) {
    QCoreApplication app(argc, argv);

    signal(SIGKILL, __sig_trap);
    signal(SIGINT, __sig_trap);

    QAudioFormat format;
    format.setFrequency(10100); // Qt Bug with 8khz
    format.setChannels(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend.";
        return(1);
    }

#if USE_UDP_STREAM
    UdpAudioReceiver stream(info, format, &app);
    if (!stream.bind(QHostAddress("192.168.3.1"), AUDIO_UDP_PORT)) {
        qWarning() << "Udp Bind Error" << stream.errorString();
        return(1);
    }
#else
    QFile stream("stream.raw");
    if (!stream.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to Open" << stream.errorString();
        return 1;
    }

    // Run Audio Output using specified format and stream.
    QAudioOutput audio(info, format);
    audio.start(&stream);
#endif

    return(app.exec());
}

