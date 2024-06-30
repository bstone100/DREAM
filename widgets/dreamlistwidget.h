#ifndef DREAMLISTWIDGET_H
#define DREAMLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

class DreamListWidget : public QWidget
{
    Q_OBJECT

public:
    static DreamListWidget *self();
    void updateDreams();

signals:
    void dreamClicked(const QString &dreamId);

private:
    explicit DreamListWidget(QWidget *parent = nullptr);
    ~DreamListWidget();

    QVBoxLayout *listLayout;
    QScrollArea *scrollArea;
    QWidget *container;

    static DreamListWidget *singleton;
};

#endif // DREAMLISTWIDGET_H
