#ifndef AUDIORECORDERWIDGET_H
#define AUDIORECORDERWIDGET_H

#include "QtWidgets/qgroupbox.h"
#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QPainter>
#include <QTimer>

class ResizingTextEdit;
class RecordButton;
class WaveformWidget;

class AudioRecorderWidget : public QGroupBox
{
    Q_OBJECT

public:
    static AudioRecorderWidget* self();
    virtual ~AudioRecorderWidget();

    ResizingTextEdit *getTranscriptTextEdit() const;
    RecordButton *getRecordButton() const;
    WaveformWidget *getWaveformWidget() const;

    void handleTranscriptionUpdated(QString text);
    void handleSilenceDetected();
    void handleTimeLimitReached();

    void handleRecordingStarted();
    void handleRecordingFinished();

    int getCurrentTranscriptionWordCount();

protected:
    explicit AudioRecorderWidget(QWidget *parent = nullptr);

private:
    static AudioRecorderWidget* singleton;

    RecordButton *recordButton = NULL;
    ResizingTextEdit *transcriptTextEdit = NULL;
    WaveformWidget *waveformWidget = NULL;

    QString transcriptionBeginning;
    QString transcriptionCurrent;

};

#endif // AUDIORECORDERWIDGET_H
