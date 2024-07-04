#include "svgbutton.h"
#include "qapplication.h"
#include <QMouseEvent>
#include "QRegularExpression"
#include "QFile"

#if defined(Q_OS_IOS)
#include "../iOS/hapticfeedback.h"
#endif

QColor SvgButton::s_defaultColor(Qt::black);
QColor SvgButton::s_disabledColor(Qt::black);
QColor SvgButton::s_hoverColor(Qt::black);
QColor SvgButton::s_pressedColor(Qt::black);

QList<SvgButton *> SvgButton::instances;

SvgButton::SvgButton(QWidget *parent) : QPushButton(parent),
    m_defaultColor(Qt::black),
    m_disabledColor(Qt::black),
    m_hoverColor(Qt::black),
    m_pressedColor(Qt::black),
    m_overrideColor(Qt::black),
    override(false)
{
    instances.append(this);

    setMouseTracking(true); // Necessary to track the mouse without pressing any buttons
    setCursor(Qt::PointingHandCursor);

    setStyleSheet("background: transparent; border: none; border-radius: 0px;");

#if defined(Q_OS_IOS)
    connect(this, &QPushButton::clicked, this, &generateHapticFeedback);
#endif

    shouldModifyStrokeColor = true;
    shouldModifyFillColor = false;

    usingAppColors = false;
}

SvgButton::~SvgButton()
{
    instances.removeAll(this);
}

void SvgButton::setSvgPath(const QString &path) {
    svgPath = path;

    QFile file(svgPath);
    if (file.open(QIODevice::ReadOnly)) {
        originalSvgString = file.readAll();
        file.close();
    }

    updateColor();
}

void SvgButton::setIconSize(const QSize &size) {
    iconSize = size;
    setFixedSize(size);
    updateColor();
}

bool SvgButton::isUsingAppColors()
{
    return usingAppColors;
}

void SvgButton::setUsingAppColors(bool newUsingAppColors)
{
    usingAppColors = newUsingAppColors;
}

void SvgButton::setAppColors(const QColor &defaultColor, const QColor &disabledColor,
                             const QColor &hoverColor, const QColor &pressedColor)
{
    s_defaultColor = defaultColor;
    s_disabledColor = disabledColor;
    s_hoverColor = hoverColor;
    s_pressedColor = pressedColor;

    foreach (auto button, instances) {
        if (button->isUsingAppColors()) {
            button->updateColor();
        }
    }
}

void SvgButton::setColors(const QColor &defaultColor, const QColor &disabledColor,
                          const QColor &hoverColor, const QColor &pressedColor)
{
    m_defaultColor = defaultColor;
    m_disabledColor = disabledColor;
    m_hoverColor = hoverColor;
    m_pressedColor = pressedColor;
    updateColor();
}

QColor SvgButton::defaultColor() const {
    return m_defaultColor;
}

void SvgButton::setDefaultColor(const QColor &color) {
    m_defaultColor = color;
    updateColor();
}

QColor SvgButton::disabledColor() const {
    return m_disabledColor;
}

void SvgButton::setDisabledColor(const QColor &color) {
    m_disabledColor = color;
    updateColor();
}

QColor SvgButton::hoverColor() const {
    return m_hoverColor;
}

void SvgButton::setHoverColor(const QColor &color) {
    m_hoverColor = color;
    updateColor();
}

QColor SvgButton::pressedColor() const {
    return m_pressedColor;
}

void SvgButton::setPressedColor(const QColor &color) {
    m_pressedColor = color;
    updateColor();
}

void SvgButton::startColorOverride(const QColor &newOverrideColor)
{
    m_overrideColor = newOverrideColor;
    override = true;
    updateColor();
}

void SvgButton::stopColorOverride()
{
    override = false;
    updateColor();
}

void SvgButton::setShouldModifyFillColor(bool newShouldModifyFillColor)
{
    shouldModifyFillColor = newShouldModifyFillColor;
    update();
}

void SvgButton::setShouldModifyStrokeColor(bool newShouldModifyStrokeColor)
{
    shouldModifyStrokeColor = newShouldModifyStrokeColor;
    updateColor();
}

bool SvgButton::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::Leave:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::EnabledChange:
        updateColor();
        break;
    default:
        break;
    }

    return QPushButton::event(event);
}

void SvgButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);

    if (originalSvgString == "") return;

    QString newSvgString = modifySvgColor(originalSvgString, m_currentColor, shouldModifyFillColor, shouldModifyStrokeColor);
    QString svgContent = newSvgString;

    QSvgRenderer renderer(svgContent.toUtf8());
    QPainter painter(this);

    // Set the fill color for the SVG
    painter.setPen(Qt::NoPen);

    // Calculate the center position for the icon
    QPointF iconTopLeft((width() - iconSize.width()) / 2.0,
                        (height() - iconSize.height()) / 2.0);
    QRectF bounds(iconTopLeft, iconSize);

    // Render the SVG
    renderer.render(&painter, bounds);
}

QColor SvgButton::appPressedColor()
{
    return s_pressedColor;
}

QColor SvgButton::activeDefaultColor()
{
    return usingAppColors ? SvgButton::s_defaultColor : m_defaultColor;
}

QColor SvgButton::activeDisabledColor()
{
    return usingAppColors ? SvgButton::s_disabledColor : m_disabledColor;
}

QColor SvgButton::activeHoverColor()
{
    return usingAppColors ? SvgButton::s_hoverColor : m_hoverColor;
}

QColor SvgButton::activePressedColor()
{
    return usingAppColors ? SvgButton::s_pressedColor : m_pressedColor;
}

QColor SvgButton::appHoverColor()
{
    return s_hoverColor;
}

QColor SvgButton::appDisabledColor()
{
    return s_disabledColor;
}

QColor SvgButton::appDefaultColor()
{
    return s_defaultColor;
}

// update the color based on the state of the button
void SvgButton::updateColor()
{
    QColor newColor;

    if (override) {
        newColor = m_overrideColor;
    } else {
        // Convert global cursor position to local widget coordinates
        QPoint localCursorPos = mapFromGlobal(QCursor::pos());
        // Check if the cursor is within the widget's bounds
        bool cursorOverWidget = rect().contains(localCursorPos);

        // Check if the left mouse button is pressed
        bool leftButtonPressed = QApplication::mouseButtons() & Qt::LeftButton;

        if (isEnabled()) {
            if (leftButtonPressed && cursorOverWidget) {
                newColor = activePressedColor();
            } else if (cursorOverWidget) {
                newColor = activeHoverColor();
            } else {
                newColor = activeDefaultColor();
            }
        } else {
            newColor = activeDisabledColor();
        }
    }

    if (m_currentColor == newColor) return;

    m_currentColor = newColor;

    update();
}

QString SvgButton::modifySvgColor(const QString &svgContent, const QColor &color, bool colorFill, bool colorStroke) {
    QString modifiedSvgContent = svgContent;

    // Regex to match fill and stroke attributes with various color values including named colors
    static QRegularExpression fillAttrRegex("fill\\s*=\\s*\"(#[0-9a-fA-F]{3,6}|[a-zA-Z]+)\"");
    static QRegularExpression strokeAttrRegex("stroke\\s*=\\s*\"(#[0-9a-fA-F]{3,6}|[a-zA-Z]+)\"");

    // Regex to match CSS styles for fill and stroke including named colors
    static QRegularExpression cssFillStyleRegex("(\\.st\\d+\\{[^}]*)(fill):(#[0-9a-fA-F]{3,6}|[a-zA-Z]+)([^}]*\\})");
    static QRegularExpression cssStrokeStyleRegex("(\\.st\\d+\\{[^}]*)(stroke):(#[0-9a-fA-F]{3,6}|[a-zA-Z]+)([^}]*\\})");

    QString replacementColor = color.name(QColor::HexRgb);

    // Replace inline fill attributes if colorFill is true
    if (colorFill) {
        modifiedSvgContent.replace(fillAttrRegex, QString("fill=\"%1\"").arg(replacementColor));
        modifiedSvgContent.replace(cssFillStyleRegex, QString("\\1fill:%1\\3").arg(replacementColor));
    }

    // Replace inline stroke attributes if colorStroke is true
    if (colorStroke) {
        modifiedSvgContent.replace(strokeAttrRegex, QString("stroke=\"%1\"").arg(replacementColor));
        modifiedSvgContent.replace(cssStrokeStyleRegex, QString("\\1stroke:%1\\3").arg(replacementColor));
    }

    return modifiedSvgContent;
}









