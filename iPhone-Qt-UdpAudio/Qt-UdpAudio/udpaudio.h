#ifndef _UDP_AUDIO_H_
#define _UDP_AUDIO_H_

#include <QAudioOutput>
#include <QUdpSocket>
#include <QIODevice>

//#define UDP_AUDIO_RECEIVER_DEBUG

class UdpAudioReceiver : public QIODevice {
    Q_OBJECT

    public:
        UdpAudioReceiver (const QAudioDeviceInfo& audioDevice,
                          const QAudioFormat& format,
                          QObject *parent = 0);
        ~UdpAudioReceiver();

        bool bind (const QHostAddress& address, quint16 port);

        qint64 bytesAvailable() const;

        bool atEnd() const { return false; }
        bool isSequential() const { return true; }

#ifdef UDP_AUDIO_RECEIVER_DEBUG
    public Q_SLOTS:
        void stateChanged (QAudio::State state);
#endif

    protected:
        qint64 readData(char *data, qint64 maxlen);
        qint64 writeData(const char *data, qint64 len);

    private Q_SLOTS:
        void processDatagrams (void);

    private:
        QUdpSocket *m_udpSocket;
        QByteArray m_buffer;
        QAudioOutput m_audio;
};

#endif /* !_UDP_AUDIO_H_ */

