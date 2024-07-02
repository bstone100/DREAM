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
    // Set widget background and shape
//    setStyleSheet("background-color: #142539; border-radius: 10px;");
//    setAttribute(Qt::WA_StyledBackground);

//    setFixedWidth(300);

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
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(waveformWidget);
    layout->addWidget(transcriptTextEdit);

//    auto hLayout = new QHBoxLayout;
////    hLayout->addStretch();
//    QSpacerItem *s1 = new QSpacerItem();
//    hLayout->addWidget(recordButton);
//    hLayout->addStretch();
    layout->addWidget(recordButton);
//    layout->addLayout(hLayout);

    layout->setAlignment(waveformWidget, Qt::AlignHCenter);
    layout->setAlignment(transcriptTextEdit, Qt::AlignHCenter);
    layout->setAlignment(recordButton, Qt::AlignHCenter | Qt::AlignBottom);

    setLayout(layout);
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
    if (text == "you" || text == "." || text == "You" || text == "♪♪") {
        text.clear(); // avoid showing common hallucinations of silence
    }

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

    MainWindow::self()->fadeInWidgets({waveformWidget, transcriptTextEdit}, 500);
}

void AudioRecorderWidget::handleRecordingFinished()
{
    transcriptionBeginning.clear();
    transcriptionCurrent.clear();

    recordButton->handleRecordingStateChanged(false);

    // fade out and hide once faded
    auto anim = MainWindow::self()->fadeOutWidget(waveformWidget, 500);
    MainWindow::self()->fadeOutWidget(transcriptTextEdit, 500);
    connect(anim, &QPropertyAnimation::finished, this, [&]{
        transcriptTextEdit->clear();
        waveformWidget->clearLevels();

        waveformWidget->hide();
        transcriptTextEdit->hide();
    });
}

int AudioRecorderWidget::getCurrentTranscriptionWordCount()
{
    // Split the text by any sequence of non-word characters
    static QRegularExpression regex("\\W+");
    QStringList words = transcriptTextEdit->toPlainText().split(regex, Qt::SkipEmptyParts);
    return words.count();
}





