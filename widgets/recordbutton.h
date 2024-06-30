#ifndef RECORDBUTTON_H
#define RECORDBUTTON_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class RecordButton : public QWidget
{
    Q_OBJECT

public:
    explicit RecordButton(QWidget *parent = nullptr);

    void click();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void clicked();

private:
    bool isPressed;
};

#endif // RECORDBUTTON_H
