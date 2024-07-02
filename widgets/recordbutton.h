#ifndef RECORDBUTTON_H
#define RECORDBUTTON_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>

class RecordButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal shapeProgress READ shapeProgress WRITE setShapeProgress NOTIFY shapeProgressChanged)

public:
    explicit RecordButton(QWidget *parent = nullptr);

    void click();
    void handleRecordingStateChanged(bool recording);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void clicked();
    void shapeProgressChanged();

private:
    qreal shapeProgress() const;
    void setShapeProgress(qreal progress);

    bool isPressed;
    bool isRecording;
    qreal m_shapeProgress;
};

#endif // RECORDBUTTON_H
