#include "recordbutton.h"
#include "../audio/audiotranscriptionmanager.h"

RecordButton::RecordButton(QWidget *parent)
    : QWidget(parent)
{
    size = 50;
}

void RecordButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // paint outer white circle
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::transparent);
    painter.drawEllipse(1, 1, size - 2, size - 2);

    // paint inner red shape
    painter.setPen(QPen(Qt::transparent, 2));
    painter.setBrush(QColor(0xff453a)); // red

    bool isRecording = AudioTranscriptionManager::self()->isRecording();

    if (isRecording) {
        // Draw smaller red square
        int padding = 14;
        QRect rect(padding, padding, size - 2 * padding, size - 2 * padding);

        int cornerRadius = 2;
        painter.drawRoundedRect(rect, cornerRadius, cornerRadius);
    } else {
        // Draw red circle
        int padding = 4;
        QRect rect(padding, padding, size - 2 * padding, size - 2 * padding);

        painter.drawEllipse(rect);
    }
}

void RecordButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPressed = true;
    }
}

void RecordButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isPressed) {
        isPressed = false;
        emit clicked();  // Emit the clicked signal
    }
}

void RecordButton::click()
{
    emit clicked();
}




