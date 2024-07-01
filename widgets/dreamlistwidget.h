#ifndef DREAMLISTWIDGET_H
#define DREAMLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "QListWidget"

class DreamListWidget : public QListWidget
{
    Q_OBJECT

public:
    static DreamListWidget *self();
    void updateDreams();
    void updateSize();

signals:
    void dreamClicked(const QString &dreamId);

protected:
    bool event(QEvent *e) override;

private:
    explicit DreamListWidget(QWidget *parent = nullptr);
    ~DreamListWidget();

    static DreamListWidget *singleton;
};

#endif // DREAMLISTWIDGET_H
