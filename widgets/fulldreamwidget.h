#ifndef FULLDREAMWIDGET_H
#define FULLDREAMWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

class FullDreamWidget : public QWidget {
    Q_OBJECT

public:
    explicit FullDreamWidget(QWidget *parent = nullptr);

    void setDream(const QString &dreamId);
    void updateWidget();

signals:
    void backButtonClicked();

private:
    QString currentDreamID;

    QPushButton *backButton;
    QLabel *titleLabel;
    QLabel *dateTimeLabel;
    QLabel *locationLabel;
    QCheckBox *nightmareCheckBox;
    QCheckBox *lucidCheckBox;
    QCheckBox *favoritedCheckBox;
    QLabel *transcriptLabel;

    QVBoxLayout *mainLayout;
};

#endif // FULLDREAMWIDGET_H
