#ifndef FULLDREAMWIDGET_H
#define FULLDREAMWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

class SvgButton;

class FullDreamWidget : public QWidget {
    Q_OBJECT

public:
    explicit FullDreamWidget(QWidget *parent = nullptr);

    void setDream(const QString &dreamId);
    void updateWidget();

    void handleWidgetInteraction();

signals:
    void backButtonClicked();

private:
    QString currentDreamID;

    SvgButton *backButton;
    SvgButton *heartButton;
    SvgButton *trashButton;

    QLabel *titleLabel;
    QLabel *dateTimeLocationLabel;

    QCheckBox *nightmareCheckBox;
    QCheckBox *lucidCheckBox;

    QLabel *transcriptLabel;

    QVBoxLayout *mainLayout;

    void handleTrashClicked();
    void updateHeart();
};

#endif // FULLDREAMWIDGET_H
