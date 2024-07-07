#include "audiotranscriptionmanager.h"
#include <QString>
#include "QPermission"
#include <QMessageBox>
#include <QApplication>
#include "../mainwindow.h"
#include "../widgets/waveformwidget.h"
#include "../widgets/audiorecorderwidget.h"
#include "../libraries/easing.h"

// android and others will have equivalent file
#include "speechrecognizer.h"

AudioTranscriptionManager *AudioTranscriptionManager::singleton = NULL;

AudioTranscriptionManager::AudioTranscriptionManager() {
    if (!singleton) {
        singleton = this;
    }

    updateLevelTimer.setSingleShot(false);
    updateLevelTimer.setInterval(30);

    connect(&updateLevelTimer, &QTimer::timeout, this, &AudioTranscriptionManager::updateLevel);
}

AudioTranscriptionManager *AudioTranscriptionManager::self()
{
    if (!singleton) {
        singleton = new AudioTranscriptionManager();
    }
    return singleton;
}

void AudioTranscriptionManager::toggleStart()
{
    if (!updateLevelTimer.isActive()) {
        start();
    } else {
        stop();
    }
}

void AudioTranscriptionManager::start() {
#if QT_CONFIG(permissions)
    QMicrophonePermission microphonePermission;
    switch (qApp->checkPermission(microphonePermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(microphonePermission, this, &AudioTranscriptionManager::start);
        return;
    case Qt::PermissionStatus::Denied:
        QMessageBox::warning(NULL, "Permission Error", "Microphone permission is not granted!");
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }
#endif

    startRecording();
    recordingTime = 0;
    updateLevelTimer.start();
    AudioRecorderWidget::self()->handleRecordingStarted();
}

void AudioTranscriptionManager::stop() {
    MainWindow::self()->sendChat();
    AudioRecorderWidget::self()->handleRecordingFinished();
    updateLevelTimer.stop();
    stopRecording();
}

void AudioTranscriptionManager::updateLevel()
{
    if (updateLevelTimer.isActive()) {
        recordingTime += updateLevelTimer.interval();
    }
    float level = getCurrentLevel();
    // stretch audio
    level *= 3;
    level = getEasedProgress(EaseInOutSine, level);
    AudioRecorderWidget::self()->getWaveformWidget()->addLevel(level);
}

int AudioTranscriptionManager::getRecordingTime() const
{
    return recordingTime;
}










