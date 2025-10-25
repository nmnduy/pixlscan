#pragma once

#include <QMainWindow>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QScrollArea>
#include <QGridLayout>
#include <QString>
#include <QFileInfo>
#include <vector>
#include <opencv2/opencv.hpp>
#include <optional>
#include <QUrl>
#include <QVBoxLayout>
#include <QStringList>
#include <QMouseEvent>
#include <QDrag>
#include <QDragMoveEvent>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>

// Widget to accept drag-and-drop of image files
class DropFrame : public QFrame {
    Q_OBJECT
public:
    explicit DropFrame(QWidget *parent = nullptr) : QFrame(parent) {
        setAcceptDrops(true);
        // Remove built-in frame to avoid extra padding; border via stylesheet
        setFrameShape(QFrame::NoFrame);
        setLineWidth(0);
        setCursor(Qt::PointingHandCursor);
    }
signals:
    void filesDropped(const QStringList &files);
    void clicked();
protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasUrls()) event->acceptProposedAction();
    }
    void dropEvent(QDropEvent *event) override {
        QStringList files;
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) files << url.toLocalFile();
        }
        if (!files.isEmpty()) emit filesDropped(files);
        event->acceptProposedAction();
    }
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
        QFrame::mousePressEvent(event);
    }
};

// Forward declarations
class MainWindow;
class ThumbnailWidget;

// Structure to track image processing state
struct ImageProcessingState {
    cv::Mat originalImage;
    cv::Mat currentImage;
    QString filename;
    int rotationAngle{0};  // 0, 90, 180, 270
    bool isSnapped{false};
};

// Self-contained thumbnail widget with encapsulated state and behavior
class ThumbnailWidget : public QWidget {
    Q_OBJECT
public:
    explicit ThumbnailWidget(ImageProcessingState *state, QWidget *parent = nullptr);

    ImageProcessingState* getState() const { return imageState; }
    void updateThumbnailImage();
    void setSelected(bool selected);
    static QImage cvMatToQImage(const cv::Mat &mat);

signals:
    void thumbnailClicked(ThumbnailWidget *widget);
    void thumbnailReordered(ThumbnailWidget *fromWidget, ThumbnailWidget *toWidget);
    void imageModified(ThumbnailWidget *widget);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onRotateLeft();
    void onRotateRight();
    void onSnap();

private:
    ImageProcessingState *imageState;  // Non-owning pointer to state managed by MainWindow
    QLabel *thumbnailLabel;
    QPoint dragStartPosition;

    cv::Mat rotateImage(const cv::Mat &image, int angle);
};

// Declare metatype for Qt signal/slot system
Q_DECLARE_METATYPE(ThumbnailWidget*)

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

public slots:
    void reorderThumbnails(ThumbnailWidget *fromWidget, ThumbnailWidget *toWidget);

private slots:
    void onThumbnailClicked(ThumbnailWidget *widget);
    void onImageModified(ThumbnailWidget *widget);
    void onUploadClicked();
    void onFilesDropped(const QStringList &files);
    void onProcessClicked();
    void onRotateLeftClicked();
    void onRotateRightClicked();
    void onExportClicked();
    void onNextClicked();
    void onBackClicked();
    void rotateHint();

private:
    // UI elements
    QPushButton *processButton{};
    QPushButton *rotateLeftButton{};
    QPushButton *rotateRightButton{};
    QPushButton *exportButton{};
    QPushButton *backButton{};
    QLabel *hintLabel{};
    QTimer *hintTimer{};
    std::vector<QString> hints;
    int currentHintIndex{0};
    QShortcut *uploadShortcut{};
    // Drop zone and staging view
    DropFrame *dropZone{};
    QPushButton *nextButton{};
    QScrollArea *stagingScrollArea{};
    QWidget *stagingContainer{};
    QHBoxLayout *stagingLayout{};
    std::vector<cv::Mat> stagedImages;
    std::vector<QString> stagedFilenames;
    // Corresponding staging item widgets for removal
    std::vector<QWidget*> stagingWidgets;

    // Processing view (1:3 column layout)
    QWidget *processingView{};
    QScrollArea *thumbnailScrollArea{};
    QWidget *thumbnailContainer{};
    QVBoxLayout *thumbnailLayout{};
    QLabel *previewLabel{};
    QScrollArea *previewScrollArea{};
    std::vector<ThumbnailWidget*> thumbnailWidgets;
    ThumbnailWidget *currentThumbnail{nullptr};

    // Image processing state
    std::vector<ImageProcessingState> processingStates;

    // Helper functions
    static QImage cvMatToQImage(const cv::Mat &mat);
    static cv::Mat qImageToCvMat(const QImage &image);
    void updatePreview();
    int getThumbnailIndex(ThumbnailWidget *widget) const;
    void exportToImages(const QString &directory, const QString &format);
    void exportToPdf(const QString &filePath);
};
