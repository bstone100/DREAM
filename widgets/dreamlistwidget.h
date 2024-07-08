#ifndef DREAMLISTWIDGET_H
#define DREAMLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "QListWidget"
#include "QScroller"

class DreamListWidgetItem;

class DreamListWidget : public QListWidget
{
    Q_OBJECT

public:
    static DreamListWidget *self();
    void updateDreams();
    void uncheckItemWidgets();

    DreamListWidgetItem *getDreamItem(const QString &dreamID);

signals:
    void dreamClicked(const QString &dreamId);

protected:
    bool event(QEvent *e) override;

private:
    explicit DreamListWidget(QWidget *parent = nullptr);
    ~DreamListWidget();

    static DreamListWidget *singleton;

    QMap<QString, DreamListWidgetItem *> idToItemMap;

    QScroller *scroller;
};

#endif // DREAMLISTWIDGET_H
