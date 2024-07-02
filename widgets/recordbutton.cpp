#include "recordbutton.h"
#include "../mainwindow.h"
#include <QPropertyAnimation>
#include <QPainter>

RecordButton::RecordButton(QWidget *parent)
    : QWidget(parent),
    isRecording(false),
    m_shapeProgress(0.0)
{

}

void RecordButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // paint outer white circle
    painter.setPen(QPen(MainWindow::lightColor, 3));
    painter.setBrush(Qt::transparent);
    int padding = 2;
    painter.drawEllipse(padding, padding, width() - 2 * padding, height() - 2 * padding);

    // paint inner red shape
    painter.setPen(QPen(Qt::transparent, 3));
    painter.setBrush(QColor(0xff453a)); // red

    // make padding and corner radius depend on shapeProgress
    // circle can be drawn as rounded rect
    int innerPadding = MainWindow::interpolate(6, width() / 3, m_shapeProgress);
    float cornerRadiusRelativeValue = MainWindow::interpolate(100, 50, m_shapeProgress);
    QRect rect(innerPadding, innerPadding, width() - 2 * innerPadding, height() - 2 * innerPadding);
    painter.drawRoundedRect(rect, cornerRadiusRelativeValue, cornerRadiusRelativeValue, Qt::RelativeSize);
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

// shapeProgress: 0 (not recording) -> 1 (recording) -> 0 (not recording)
void RecordButton::handleRecordingStateChanged(bool recording)
{
    isRecording = recording;

    QPropertyAnimation *animation = new QPropertyAnimation(this, "shapeProgress");
    animation->setDuration(300); // Animation duration in milliseconds
    animation->setStartValue(isRecording ? 0.0 : 1.0);
    animation->setEndValue(isRecording ? 1.0 : 0.0);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(animation, &QPropertyAnimation::finished, animation, &QObject::deleteLater);
    animation->start();
}

qreal RecordButton::shapeProgress() const
{
    return m_shapeProgress;
}

void RecordButton::setShapeProgress(qreal progress)
{
    if (m_shapeProgress != progress) {
        m_shapeProgress = progress;
        update();
    }
}







