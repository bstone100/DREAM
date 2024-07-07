#ifndef AUDIOTRANSCRIPTIONMANAGER_H
#define AUDIOTRANSCRIPTIONMANAGER_H

#include <QObject>
#include "QTimer"

class AudioTranscriptionManager : public QObject {
    Q_OBJECT

public:
    AudioTranscriptionManager();
    static AudioTranscriptionManager *self();

    void toggleStart();
    void start();
    void stop();

    int getRecordingTime() const;

signals:
    void transcriptionUpdated(const QString &transcription);
    void silenceDetected();
    void timeLimitReached();

private:
    static AudioTranscriptionManager *singleton;

    void updateLevel();
    QTimer updateLevelTimer;
    int recordingTime;
};

#endif // AUDIOTRANSCRIPTIONMANAGER_H
