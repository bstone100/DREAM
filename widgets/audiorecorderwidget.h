#ifndef AUDIORECORDERWIDGET_H
#define AUDIORECORDERWIDGET_H

#include "QtCore/qeasingcurve.h"
#include "QtCore/qpropertyanimation.h"
#include "QtWidgets/qgroupbox.h"
#include "qboxlayout.h"
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
    QString getFirstWord(QString text);

    QVBoxLayout *getMainLayout() const;

protected:
    explicit AudioRecorderWidget(QWidget *parent = nullptr);

private:
    static AudioRecorderWidget* singleton;

    QVBoxLayout *mainLayout;

    RecordButton *recordButton = NULL;
    ResizingTextEdit *transcriptTextEdit = NULL;
    WaveformWidget *waveformWidget = NULL;

    QTimer silenceTimer;

    QString transcriptionBeginning;
    QString transcriptionCurrent;

    int collapsedHeight = 84;
    int expandedHeight = 234;
    QPropertyAnimation *animateHeightChange(int newHeight, int duration, QEasingCurve easingCurve = QEasingCurve::Linear);
};

#endif // AUDIORECORDERWIDGET_H
