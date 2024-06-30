#include "recordbutton.h"
#include "../audio/audiotranscriptionmanager.h"

RecordButton::RecordButton(QWidget *parent)
    : QWidget(parent)
{

}

void RecordButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // paint outer white circle
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::transparent);
    painter.drawEllipse(1, 1, width() - 2, height() - 2);

    // paint inner red shape
    painter.setPen(QPen(Qt::transparent, 2));
    painter.setBrush(QColor(0xff453a)); // red

    bool isRecording = AudioTranscriptionManager::self()->isRecording();

    if (isRecording) {
        // Draw smaller red square
        int padding = 14;
        QRect rect(padding, padding, width() - 2 * padding, height() - 2 * padding);

        int cornerRadius = 2;
        painter.drawRoundedRect(rect, cornerRadius, cornerRadius);
    } else {
        // Draw red circle
        int padding = 4;
        QRect rect(padding, padding, width() - 2 * padding, height() - 2 * padding);

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




