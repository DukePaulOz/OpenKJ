#ifndef AUDIOFADER_H
#define AUDIOFADER_H

#include <QObject>
#include <QTimer>
#include <gst/gst.h>

class AudioFader : public QObject
{
    Q_OBJECT
private:
    GstElement *volumeElement;
    QTimer *timer;
    bool fading;
    double preFadeVol;
    double targetVol;
    void setVolume(double volume);
    double volume();
    bool paused;

public:
    explicit AudioFader(QObject *parent = 0);
    void setVolumeElement(GstElement *volumeElement);
    void setPaused(bool paused);
signals:
    void volumeChanged(double volume);
    void volumeChanged(int volume);
    void fadeComplete();

public slots:
    void fadeOut(bool block = false);
    void fadeIn(bool block = false);


private slots:
    void timerTimeout();
};

#endif // AUDIOFADER_H
