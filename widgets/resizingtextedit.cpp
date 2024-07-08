#include "resizingtextedit.h"
#include <QFontMetrics>
#include "QAbstractTextDocumentLayout"
#include "QScroller"
#include "QtCore/qtimer.h"
#include "QtWidgets/qscrollbar.h"

ResizingTextEdit::ResizingTextEdit(QWidget *parent) : QTextEdit(parent) {
    connect(this, &ResizingTextEdit::textChanged, this, &ResizingTextEdit::updateHeight);

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    QScroller* scroller = QScroller::scroller(this);

    QScrollerProperties properties = scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0);

    scroller->setScrollerProperties(properties);
    scroller->grabGesture(this, QScroller::TouchGesture);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
#endif

    minHeight = 40;
    maxHeight = 100;

    updateHeight();
}

void ResizingTextEdit::updateHeight() {
    int docHeight = this->document()->size().height(); // Get the document height
    int margins = this->contentsMargins().top() + this->contentsMargins().bottom(); // Calculate the total vertical margins

    int height = qBound(minHeight, docHeight + margins, maxHeight);

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
    setVerticalScrollBarPolicy(height < maxHeight ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
#endif

    setFixedHeight(height);
}

int ResizingTextEdit::getMaxHeight() const
{
    return maxHeight;
}

void ResizingTextEdit::setMaxHeight(int newMaxHeight)
{
    maxHeight = newMaxHeight;
    updateHeight();
}

int ResizingTextEdit::getMinHeight() const
{
    return minHeight;
}

void ResizingTextEdit::setMinHeight(int newMinHeight)
{
    minHeight = newMinHeight;
    updateHeight();
}

void ResizingTextEdit::setTextBetter(const QString &text, Qt::Alignment alignment)
{
    // Save the current scroll position and determine if we are at the bottom
    QScrollBar *scrollBar = verticalScrollBar();
    bool isAtBottom = (scrollBar->value() == scrollBar->maximum());
    int scrollPos = scrollBar->value();

    setText(text);
    setAlignment(alignment);

    // Restore the scroll position or scroll to the bottom if we were at the bottom
    if (isAtBottom) {
        scrollBar->setValue(scrollBar->maximum());
    } else {
        scrollBar->setValue(scrollPos);
    }

    QTimer::singleShot(5, this, &ResizingTextEdit::updateHeight);
}

bool ResizingTextEdit::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
        updateHeight();
        break;
    default:
        break;
    }

    return QTextEdit::event(e);
}






