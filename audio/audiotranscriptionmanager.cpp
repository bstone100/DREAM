#include "audiotranscriptionmanager.h"
#include <QString>
#include "QPermission"
#include <QMessageBox>
#include <QApplication>
#include "../mainwindow.h"
#include "../widgets/waveformwidget.h"
#include "../widgets/audiorecorderwidget.h"

// android and others will have equivalent file
#include "whisperinterface.h"

AudioTranscriptionManager *AudioTranscriptionManager::singleton = NULL;

AudioTranscriptionManager::AudioTranscriptionManager() {
    if (!singleton) {
        singleton = this;
    }

    init();

    setupAudioCapture();

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

void AudioTranscriptionManager::init()
{
#if QT_CONFIG(permissions)
    QMicrophonePermission microphonePermission;
    switch (qApp->checkPermission(microphonePermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(microphonePermission, this, &AudioTranscriptionManager::init);
        return;
    case Qt::PermissionStatus::Denied:
        QMessageBox::warning(NULL, "Permission Error", "Microphone permission is not granted!");
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }
#endif
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
    startAudioCapture();
    updateLevelTimer.start();
    AudioRecorderWidget::self()->handleRecordingStarted();
}

void AudioTranscriptionManager::stop() {
    MainWindow::self()->sendChat();
    AudioRecorderWidget::self()->handleRecordingFinished();
    updateLevelTimer.stop();
    stopAudioCapture();
}

bool AudioTranscriptionManager::isRecording()
{
    return isCapturing();
}

void AudioTranscriptionManager::updateLevel()
{
    float level = getCurrentLevel();
    AudioRecorderWidget::self()->getWaveformWidget()->addLevel(level);
}










