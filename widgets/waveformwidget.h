#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>

class WaveformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget *parent = nullptr);
    void addLevel(float level); // Adds a new level to the waveform
    void clearLevels();

    void expand();
    void collapse();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<float> levels; // Store the audio levels
    float minHeight = 1.0;

};

#endif // WAVEFORMWIDGET_H
