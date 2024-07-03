#ifndef DREAMLISTWIDGETITEM_H
#define DREAMLISTWIDGETITEM_H

#include "QtWidgets/qpushbutton.h"
#include <QWidget>
#include <QLabel>

class DreamListWidgetItem : public QPushButton {
    Q_OBJECT

public:
    explicit DreamListWidgetItem(const QString &dreamID, QWidget *parent = nullptr);

    QString getDreamID() const;

private:
    QString dreamID;

    QLabel *titleLabel;
    QLabel *dateLabel;
    QLabel *lengthLabel;
};

#endif // DREAMLISTWIDGETITEM_H
