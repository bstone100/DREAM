#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QSettings>
#include <QRadioButton>
#include <QTouchEvent>
#include "QTextEdit"
#include "QStackedWidget"
#include "QtCore/qelapsedtimer.h"
#include "QtPositioning/qgeoaddress.h"
#include "qpropertyanimation.h"
#include "QQueue"
#include "QTableView"
#include "QTimer"
#include "QAudioSink"

class OpenAIRequest;
class AudioRecorder;
class SvgButton;
class ResizingTextEdit;
class AudioLevel;
class ResizingComboBox;
class MoonWidget;
class FullDreamWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow *self();

    void updateWidgets();

    void sendChat();

    void saveSettings();
    void loadSettings();

    static QString version;
    static QString currentPath;

    static QColor lightColor;
    static QColor lightMidColor;
    static QColor darkMidColor;
    static QColor darkColor;
    static QColor sidePanelColorDark;
    static QColor sidePanelColorLight;

    bool isDarkModeOn(){return isDarkMode;}
    bool isSystemDark();
    void handleThemeChange(bool isDarkMode);

    void dumpJsonToFile(QJsonObject jObj, QString fileName);
    QPropertyAnimation *fadeInWidget(QWidget *widget, int duration);
    QPropertyAnimation *fadeOutWidget(QWidget *widget, int duration);
    QPropertyAnimation *fadeInWidgets(QList<QWidget *> widgets, int duration);
    QPropertyAnimation *fadeOutWidgets(QList<QWidget *> widgets, int duration);
    void setWidgetOpacity(QWidget *widget, double opacity);
    double getWidgetOpacity(QWidget *widget);
    static double interpolate(double startVal, double endVal, double progress);
    static void smartSetVisible(QList<QWidget *> widgets, bool visible, int duration = 500, QEasingCurve::Type curveType = QEasingCurve::Linear);

    QString getNewOriginalDreamID() const;

    QMargins getAppMargins() const;

    void expandFullDreamWidget();
    void collapseFullDreamWidget();

signals:
    void backgroundColorChanged();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    static MainWindow *singleton;

    bool settingsLoaded = false;
    bool onboarded = false;

    void setDarkMode(bool isDarkMode);

    QSettings *settings;

    QMargins appMargins;

    QStackedWidget *stackedWidget;
    QWidget *centralWidget;
    QVBoxLayout *layout;

    ResizingComboBox *themeComboBox;
    MoonWidget *moonWidget;
    FullDreamWidget *fullDreamWidget;

    void handleDreamItemClicked(const QString &dreamID);

    bool fullDreamWidgetExpanding = false;
    bool fullDreamWidgetCollapsing = false;

    QString apiKey;
    bool isDarkMode;
    bool isAutoTheme;

    OpenAIRequest *chatRequest;

    // gestures

    void touchEvent(QTouchEvent *event);

    void exitFullDreamTouchEvent(QTouchEvent *event);
    void exitFullDreamHandleSwipeEnd();

    enum Gesture {
        SidePanel = 0,
        ExitFullDream,
        Undefined
    };

    Gesture currentGesture = Undefined;

    QPoint touchStartPoint;
    QPoint previousPoint;

    // v = x/t
    int dx;
    int dt;
    qreal progress;

    QElapsedTimer stopwatch;

    QPropertyAnimation *centralWidgetInterpolator = NULL;
    QPropertyAnimation *fullDreamWidgetInterpolator = NULL;
    QPropertyAnimation *backgroundColorInterpolator = NULL;

    // dream

    QString newOriginalDreamID;
    void handleLocationObtained(QGeoAddress location);
    void handleGenerationFinished();

    // other
    QColor backgroundColor() const {
        return palette().color(QPalette::Window);
    }

    void setBackgroundColor(const QColor &color) {
        QPalette pal = palette();
        pal.setColor(QPalette::Window, color);
        setPalette(pal);
    }
};

#endif // MAINWINDOW_H








