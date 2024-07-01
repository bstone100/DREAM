#ifndef MOONWIDGET_H
#define MOONWIDGET_H

#include <QWidget>

class MoonWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MoonWidget(QWidget *parent = nullptr);

    void setLevel(qreal level);

    void resizeImage();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_level = 0.0;

    QPixmap moonImage;
    QPixmap currentPixmap;
};

#endif // MOONWIDGET_H
