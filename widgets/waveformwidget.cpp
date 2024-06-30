#include "waveformwidget.h"
#include <QPainter>

WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget(parent)
{

}

void WaveformWidget::addLevel(float level) {
    if (levels.size() > width()) {
        levels.removeFirst();
    }
    level = qBound(0.0, level, 1.0);
    levels.append(level);
    update();
}

void WaveformWidget::clearLevels()
{
    levels.clear();
    update();
}

void WaveformWidget::expand()
{
    // idk yet
}

void WaveformWidget::collapse()
{
    // show the levels shrinking to min height then hide the widget
}

void WaveformWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(Qt::red); // Set the color of the waveform

    int x = width();
    for (int i = levels.size() - 1; i >= 0; --i) {
        float lineHeight = levels[i] * height();
        lineHeight = qBound(minHeight, lineHeight, (float)height());
        painter.drawLine(x, height() / 2 - lineHeight / 2, x, height() / 2 + lineHeight / 2);
        x -= 4;
    }
}







