#include "moonwidget.h"

#include <QPainter>
#include "QPainterPath"

MoonWidget::MoonWidget(QWidget *parent) : QWidget(parent)
{
    moonImage = QPixmap(":/images/dream.png");
}

void MoonWidget::setLevel(qreal level)
{
    if (m_level != level) {
        m_level = level;

        resizeImage();
        update();
    }
}

void MoonWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    // Adjust the drawing position based on the device pixel ratio
    qreal ratio = devicePixelRatioF();
    painter.drawPixmap(QRect((width() - currentPixmap.width() / ratio) / 2,
                             (height() - currentPixmap.height() / ratio) / 2,
                             currentPixmap.width() / ratio,
                             currentPixmap.height() / ratio), currentPixmap);
}

void MoonWidget::resizeImage() {
    int minSize = width() - 20;
    int maxSize = width();
    int size = static_cast<int>(minSize + (maxSize - minSize) * m_level);

    qreal ratio = devicePixelRatioF();
    // Calculate the size considering the device pixel ratio
    currentPixmap = moonImage.scaled(size * ratio, size * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    currentPixmap.setDevicePixelRatio(ratio);  // Set the device pixel ratio for pixmap
}




