#ifndef KHAUDIOBACKENDQTMULTIMEDIA_H
#define KHAUDIOBACKENDQTMULTIMEDIA_H

#include <QObject>
#include <khabstractaudiobackend.h>
#include <QAudioDecoder>
#include <QAudioOutput>
#include <QBuffer>
#include <QTimer>
#include <QThread>

class FaderQtMultimedia : public QThread
{
    Q_OBJECT

private:
    float m_targetVolume;
    float m_preOutVolume;
    QAudioOutput *audioOutput;
    bool fading;

public:
    explicit FaderQtMultimedia(QAudioOutput *audioOutput, QObject *parent = 0);
    void run();
    void fadeIn();
    void fadeOut();
    bool isFading();
    void restoreVolume();

signals:
    void volumeChanged(int volume);

public slots:
    void setBaseVolume(int volume);

};


class KhAudioBackendQtMultimedia : public KhAbstractAudioBackend
{
    Q_OBJECT
private:
    QAudioDecoder *audioDecoder;
    QAudioOutput *audioOutput;
    QIODevice *audioBuffer;
    qint64 m_duration;
    QBuffer *buffer;
    QByteArray *m_array;
    QTimer *silenceDetectTimer;
    bool m_silent;
    bool m_detectSilence;
    FaderQtMultimedia *fader;
    bool m_fade;

public:
    explicit KhAudioBackendQtMultimedia(QObject *parent = 0);
    int volume();
    qint64 position();
    bool isMuted();
    qint64 duration();
    KhAbstractAudioBackend::State state();
    bool canPitchShift();
    int pitchShift();
    bool canFade();
    QString backendName();
    bool canDetectSilence();
    bool isSilent();
    bool canDownmix();
    bool downmixChangeRequiresRestart();
    void setOutputDevice(int deviceIndex);
    bool stopping();

public slots:
    void play();
    void pause();
    void setMedia(QString filename);
    void setMuted(bool muted);
    void setPosition(qint64 position);
    void setVolume(int volume);
    void stop(bool skipFade);
    void setPitchShift(int pitchShift);
    void fadeOut();
    void fadeIn();
    void setUseFader(bool fade);
    void setUseSilenceDetection(bool enabled);
    void setDownmix(bool enabled);

private slots:
    void processAudio();
    void durationReceived(qint64 duration);
    void audioOutputNotify();
    void audioOutputStateChanged(QAudio::State state);
    void silenceDetectTimerTimeout();

};

#endif // KHAUDIOBACKENDQTMULTIMEDIA_H