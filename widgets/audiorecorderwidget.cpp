#include "audiorecorderwidget.h"
#include <QVBoxLayout>

#include "QtCore/qregularexpression.h"
#include "waveformwidget.h"
#include "resizingtextedit.h"
#include "recordbutton.h"
#include "../audio/audiotranscriptionmanager.h"
#include "../mainwindow.h"

AudioRecorderWidget* AudioRecorderWidget::singleton = nullptr;

AudioRecorderWidget::AudioRecorderWidget(QWidget *parent)
    : QGroupBox(parent)
{
    // Initialize components
    recordButton = new RecordButton(this);
    recordButton->setFixedSize(60, 60);

    transcriptTextEdit = new ResizingTextEdit(this);
    transcriptTextEdit->setAcceptRichText(false);
    transcriptTextEdit->setReadOnly(true);
    transcriptTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    transcriptTextEdit->setMinHeight(60);
    transcriptTextEdit->setMaxHeight(200);

    waveformWidget = new WaveformWidget(this);
    waveformWidget->setFixedSize(300, 70);

    transcriptTextEdit->hide(); // Initially hidden
    waveformWidget->hide();

    connect(recordButton, &RecordButton::clicked, AudioTranscriptionManager::self(), &AudioTranscriptionManager::toggleStart);
    connect(AudioTranscriptionManager::self(), &AudioTranscriptionManager::transcriptionUpdated, this, &AudioRecorderWidget::handleTranscriptionUpdated);
    connect(AudioTranscriptionManager::self(), &AudioTranscriptionManager::silenceDetected, this, &AudioRecorderWidget::handleSilenceDetected);
    connect(AudioTranscriptionManager::self(), &AudioTranscriptionManager::timeLimitReached, this, &AudioRecorderWidget::handleTimeLimitReached);

    // Layout management
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(waveformWidget, 0, Qt::AlignHCenter);
    mainLayout->addWidget(transcriptTextEdit, 0, Qt::AlignHCenter);
    mainLayout->addWidget(recordButton, 0, Qt::AlignHCenter | Qt::AlignBottom);


    setLayout(mainLayout);

    silenceTimer.setSingleShot(true);
    silenceTimer.setInterval(2000);
}

QVBoxLayout *AudioRecorderWidget::getMainLayout() const
{
    return mainLayout;
}

AudioRecorderWidget* AudioRecorderWidget::self() {
    if (singleton == nullptr) {
        singleton = new AudioRecorderWidget();
    }
    return singleton;
}

AudioRecorderWidget::~AudioRecorderWidget() {
    delete recordButton;
    delete transcriptTextEdit;
    delete waveformWidget;
}

WaveformWidget *AudioRecorderWidget::getWaveformWidget() const
{
    return waveformWidget;
}

RecordButton *AudioRecorderWidget::getRecordButton() const
{
    return recordButton;
}

ResizingTextEdit *AudioRecorderWidget::getTranscriptTextEdit() const
{
    return transcriptTextEdit;
}

// called repeatedly during recording with updated transcription
void AudioRecorderWidget::handleTranscriptionUpdated(QString text)
{
//    if (!AudioTranscriptionManager::self()->isRecording()) return;

    text = text.trimmed();

    if (!text.isEmpty()) {
        text[0] = text[0].toUpper();
    }

    if (transcriptionCurrent == text) {
        return;
    }

    transcriptionCurrent = text;

    transcriptTextEdit->setTextBetter(transcriptionBeginning + transcriptionCurrent);
}

void AudioRecorderWidget::handleSilenceDetected()
{
    if (transcriptionCurrent.isEmpty()) return;

    transcriptionBeginning += transcriptionCurrent;
    transcriptionCurrent.clear();

    transcriptionBeginning = transcriptionBeginning.trimmed();

    QChar lastChar = transcriptionBeginning[transcriptionBeginning.length() - 1];
    lastChar.isPunct() ? transcriptionBeginning += " " : transcriptionBeginning += ". ";
}

// for now just stop recording
// time gets reset with silence so this is unlikely to be reached
void AudioRecorderWidget::handleTimeLimitReached()
{
    recordButton->click();
}

void AudioRecorderWidget::handleRecordingStarted()
{
    recordButton->handleRecordingStateChanged(true);

    auto anim = animateHeightChange(expandedHeight, 300);

    connect(anim, &QPropertyAnimation::stateChanged, this, [=](QAbstractAnimation::State newState, QAbstractAnimation::State oldState){
        if (newState == QAbstractAnimation::Stopped) {
            MainWindow::self()->fadeInWidgets({waveformWidget, transcriptTextEdit}, 200);
        }
    });
}

void AudioRecorderWidget::handleRecordingFinished()
{
    transcriptionBeginning.clear();
    transcriptionCurrent.clear();

    recordButton->handleRecordingStateChanged(false);

    // Set the minimum height to the current height to prevent layout shrinkage
    setMinimumHeight(this->height());

    auto anim = MainWindow::self()->fadeOutWidgets({waveformWidget, transcriptTextEdit}, 200);

    connect(anim, &QPropertyAnimation::stateChanged, this, [=](QAbstractAnimation::State newState, QAbstractAnimation::State oldState){
        if (newState == QAbstractAnimation::Stopped) {
            transcriptTextEdit->clear();
            waveformWidget->clearLevels();

            waveformWidget->hide();
            transcriptTextEdit->hide();

            // Animate the height change after hiding the widgets
            auto heightAnim = animateHeightChange(collapsedHeight, 300);

            // Reset the minimum height after the animation
            connect(heightAnim, &QPropertyAnimation::finished, this, [=]{
                setMinimumHeight(0);
            });
        }
    });
}

int AudioRecorderWidget::getCurrentTranscriptionWordCount()
{
    // Split the text by any sequence of non-word characters
    static QRegularExpression regex("\\W+");
    QStringList words = transcriptTextEdit->toPlainText().split(regex, Qt::SkipEmptyParts);
    return words.count();
}

QString AudioRecorderWidget::getFirstWord(QString text)
{
    // Split the text by any sequence of non-word characters
    static QRegularExpression regex("\\W+");
    QStringList words = text.split(regex, Qt::SkipEmptyParts);
    if (words.size() > 0) {
        return words.at(0);
    } else {
        return "";
    }
}

QPropertyAnimation *AudioRecorderWidget::animateHeightChange(int newHeight, int duration, QEasingCurve easingCurve)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "minimumHeight");
    animation->setDuration(duration);
    animation->setEasingCurve(easingCurve);
    animation->setStartValue(this->height());
    animation->setEndValue(newHeight);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    return animation;
}
















