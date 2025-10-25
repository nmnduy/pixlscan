#include "mainwindow.h"
#include "doc_snapper.h"
#include "export_dialog.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QScrollArea>
#include <QGridLayout>
#include <QStyle>
#include <QString>
#include <algorithm>
#include <QFileInfo>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPrinter>
#include <QPainter>
#include <QPageSize>

// Implementation of ThumbnailWidget
ThumbnailWidget::ThumbnailWidget(ImageProcessingState *state, QWidget *parent)
    : QWidget(parent), imageState(state), thumbnailLabel(nullptr)
{
    setAcceptDrops(true);

    const int thumbnailSize = 120;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // Thumbnail image (clickable)
    thumbnailLabel = new QLabel(this);
    thumbnailLabel->setAlignment(Qt::AlignCenter);
    thumbnailLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
    thumbnailLabel->setLineWidth(2);
    thumbnailLabel->setStyleSheet("QLabel { border: 2px solid #555; }");
    thumbnailLabel->setCursor(Qt::PointingHandCursor);
    thumbnailLabel->setFixedSize(thumbnailSize, thumbnailSize);
    thumbnailLabel->setScaledContents(false);

    updateThumbnailImage();
    mainLayout->addWidget(thumbnailLabel, 0, Qt::AlignHCenter);

    // Control buttons row
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(4);
    controlsLayout->setContentsMargins(0, 0, 0, 0);

    // Rotate left button
    QToolButton *rotateLeftBtn = new QToolButton(this);
    rotateLeftBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    rotateLeftBtn->setToolTip(tr("Rotate Left"));
    rotateLeftBtn->setIconSize(QSize(20, 20));
    connect(rotateLeftBtn, &QToolButton::clicked, this, &ThumbnailWidget::onRotateLeft);

    // Rotate right button
    QToolButton *rotateRightBtn = new QToolButton(this);
    rotateRightBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    rotateRightBtn->setToolTip(tr("Rotate Right"));
    rotateRightBtn->setIconSize(QSize(20, 20));
    connect(rotateRightBtn, &QToolButton::clicked, this, &ThumbnailWidget::onRotateRight);

    // Snap button
    QToolButton *snapBtn = new QToolButton(this);
    snapBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    snapBtn->setToolTip(tr("Snap Document"));
    snapBtn->setIconSize(QSize(20, 20));
    connect(snapBtn, &QToolButton::clicked, this, &ThumbnailWidget::onSnap);

    controlsLayout->addWidget(rotateLeftBtn);
    controlsLayout->addWidget(rotateRightBtn);
    controlsLayout->addWidget(snapBtn);

    // Center the buttons horizontally
    QHBoxLayout *buttonContainerLayout = new QHBoxLayout();
    buttonContainerLayout->addStretch();
    buttonContainerLayout->addLayout(controlsLayout);
    buttonContainerLayout->addStretch();

    mainLayout->addLayout(buttonContainerLayout, 0);
}

void ThumbnailWidget::updateThumbnailImage()
{
    if (!thumbnailLabel || !imageState)
        return;

    const int thumbnailSize = 120;
    QImage qimg = cvMatToQImage(imageState->currentImage);
    thumbnailLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
        thumbnailSize, thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QImage ThumbnailWidget::cvMatToQImage(const cv::Mat &mat)
{
    // Convert based on mat type
    if (mat.empty())
        return QImage();
    switch (mat.type()) {
    case CV_8UC1: {
        QImage img(mat.cols, mat.rows, QImage::Format_Grayscale8);
        memcpy(img.bits(), mat.data, static_cast<size_t>(mat.total() * mat.elemSize()));
        return img;
    }
    case CV_8UC3: {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
        return img.copy(); // deep copy because cv::Mat will be freed
    }
    case CV_8UC4: {
        QImage img(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
        return img.copy();
    }
    default:
        // Unsupported format
        return QImage();
    }
}

void ThumbnailWidget::setSelected(bool selected)
{
    if (!thumbnailLabel)
        return;

    if (selected) {
        thumbnailLabel->setStyleSheet("QLabel { border: 3px solid #3daee9; }");
    } else {
        thumbnailLabel->setStyleSheet("QLabel { border: 2px solid #555; }");
    }
}

void ThumbnailWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if click is on the thumbnail label
        if (thumbnailLabel && thumbnailLabel->geometry().contains(event->pos())) {
            emit thumbnailClicked(this);
        } else {
            // Start potential drag operation
            dragStartPosition = event->pos();
        }
    }
    QWidget::mousePressEvent(event);
}

void ThumbnailWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    // Start drag operation
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    // Store pointer to this widget in mime data (as string for compatibility)
    mimeData->setText(QString::number(reinterpret_cast<qintptr>(this)));
    drag->setMimeData(mimeData);

    // Create a pixmap of the widget for drag visualization
    QPixmap pixmap(size());
    render(&pixmap);
    drag->setPixmap(pixmap.scaled(pixmap.size() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    drag->setHotSpot(event->pos());

    drag->exec(Qt::MoveAction);
}

void ThumbnailWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() && event->source() != this) {
        event->acceptProposedAction();
    }
}

void ThumbnailWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasText() && event->source() != this) {
        event->acceptProposedAction();
    }
}

void ThumbnailWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText() && event->source() != this) {
        ThumbnailWidget *fromWidget = reinterpret_cast<ThumbnailWidget*>(
            event->mimeData()->text().toLongLong());
        emit thumbnailReordered(fromWidget, this);
        event->acceptProposedAction();
    }
}

void ThumbnailWidget::onRotateLeft()
{
    if (!imageState)
        return;

    imageState->rotationAngle = (imageState->rotationAngle + 270) % 360;
    imageState->currentImage = rotateImage(imageState->originalImage, imageState->rotationAngle);

    // If image was snapped, need to re-snap the rotated image
    if (imageState->isSnapped) {
        auto resultOpt = snapDocument(imageState->currentImage, true);
        if (resultOpt) {
            imageState->currentImage = *resultOpt;
        }
    }

    updateThumbnailImage();
    emit imageModified(this);
}

void ThumbnailWidget::onRotateRight()
{
    if (!imageState)
        return;

    imageState->rotationAngle = (imageState->rotationAngle + 90) % 360;
    imageState->currentImage = rotateImage(imageState->originalImage, imageState->rotationAngle);

    // If image was snapped, need to re-snap the rotated image
    if (imageState->isSnapped) {
        auto resultOpt = snapDocument(imageState->currentImage, true);
        if (resultOpt) {
            imageState->currentImage = *resultOpt;
        }
    }

    updateThumbnailImage();
    emit imageModified(this);
}

void ThumbnailWidget::onSnap()
{
    if (!imageState)
        return;

    // Apply snapping to the rotated image
    cv::Mat rotatedImage = rotateImage(imageState->originalImage, imageState->rotationAngle);
    auto resultOpt = snapDocument(rotatedImage, true);

    if (!resultOpt) {
        QMessageBox::warning(this, tr("Processing Error"),
            tr("Failed to detect document in image: %1").arg(imageState->filename));
        return;
    }

    imageState->currentImage = *resultOpt;
    imageState->isSnapped = true;
    imageState->rotationAngle = 0;

    updateThumbnailImage();
    emit imageModified(this);
}

cv::Mat ThumbnailWidget::rotateImage(const cv::Mat &image, int angle)
{
    if (image.empty())
        return image;

    cv::Mat rotated;
    switch (angle) {
    case 0:
        rotated = image.clone();
        break;
    case 90:
        cv::rotate(image, rotated, cv::ROTATE_90_CLOCKWISE);
        break;
    case 180:
        cv::rotate(image, rotated, cv::ROTATE_180);
        break;
    case 270:
        cv::rotate(image, rotated, cv::ROTATE_90_COUNTERCLOCKWISE);
        break;
    default:
        rotated = image.clone();
        break;
    }
    return rotated;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Central widget and layout
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    // Drop zone for images
    dropZone = new DropFrame(this);
    dropZone->setObjectName("dropZone");
    // Constrain drop zone height for clearer visibility
    dropZone->setMinimumHeight(480);
    dropZone->setMaximumHeight(640);
    // Ensure adequate width for drop zone
    dropZone->setMinimumWidth(640);
    // Transparent background for dark themes, subtle dashed border (scoped)
    dropZone->setStyleSheet(
        "#dropZone { background: transparent; border: 2px dashed #888; }"
    );
    // Add icon and help text inside drop zone
    {
        // Layout for icon above text
        QVBoxLayout *dzLayout = new QVBoxLayout(dropZone);
        // Reduce spacing and margins between icon and text
        dzLayout->setContentsMargins(0, 0, 0, 0);
        dzLayout->setSpacing(2);  // Reduced from 4 to 2
        // Use native Qt icon for upload
        QToolButton *iconButton = new QToolButton(dropZone);
        iconButton->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
        iconButton->setIconSize(QSize(48, 48));
        iconButton->setStyleSheet(
            "QToolButton { border: none; color: orange; background: transparent; }"
            "QToolButton::menu-indicator { image: none; }"
            "QToolButton:disabled { background: transparent; }"
        );
        iconButton->setEnabled(false);  // Make it non-clickable to function as a display icon
        iconButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // Center the icon in the container using the layout
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        buttonLayout->addWidget(iconButton, 0, Qt::AlignCenter);
        buttonLayout->addStretch();

        // Add stretch before icon to push everything up
        dzLayout->addStretch();
        dzLayout->addLayout(buttonLayout);
        // Help text
        QLabel *dzLabel = new QLabel(
            tr("Drag & drop images here\nor click to select"), dropZone
        );
        dzLabel->setAlignment(Qt::AlignCenter);
        dzLabel->setWordWrap(true);
        dzLayout->addWidget(dzLabel);
        // Add stretch after text to center everything vertically
        dzLayout->addStretch();
    }
    mainLayout->addWidget(dropZone);
    connect(dropZone, &DropFrame::filesDropped, this, &MainWindow::onFilesDropped);
    connect(dropZone, &DropFrame::clicked, this, &MainWindow::onUploadClicked);

    // Staging area - horizontal view of dropped/uploaded images
    // This is placed here so it appears below the drop zone and above the buttons
    stagingScrollArea = new QScrollArea(this);
    stagingContainer = new QWidget(this);
    stagingLayout = new QHBoxLayout(stagingContainer);
    stagingLayout->setAlignment(Qt::AlignCenter);  // Center the images
    stagingContainer->setLayout(stagingLayout);
    stagingScrollArea->setWidget(stagingContainer);
    stagingScrollArea->setWidgetResizable(true);  // Allow the container to resize
    stagingScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    stagingScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    stagingScrollArea->setFixedHeight(240);
    mainLayout->addWidget(stagingScrollArea);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QHBoxLayout *imageLayout = new QHBoxLayout();

    // Buttons
    // Advance to processing view
    nextButton = new QPushButton(tr("Next"), this);
    nextButton->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    // Primary action hidden until images are staged
    processButton = new QPushButton(tr("Snap Document"), this);
    // Rotate buttons
    rotateLeftButton = new QPushButton(tr("Rotate Left"), this);
    rotateLeftButton->setEnabled(false);
    rotateRightButton = new QPushButton(tr("Rotate Right"), this);
    rotateRightButton->setEnabled(false);
    processButton->setEnabled(false);
    buttonLayout->addWidget(nextButton);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextClicked);
    buttonLayout->addWidget(rotateLeftButton);
    buttonLayout->addWidget(rotateRightButton);
    buttonLayout->addWidget(processButton);
    // Export button
    exportButton = new QPushButton(tr("Export"), this);
    exportButton->setEnabled(false);

    // Back button
    backButton = new QPushButton(tr("Back"), this);
    backButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));

    // Initialize hints
    hints.push_back(tr("Tip: Hold and drag thumbnails to rearrange images"));
    hints.push_back(tr("Tip: Click a thumbnail to preview the image"));
    hints.push_back(tr("Tip: Use the rotate buttons below each thumbnail"));
    hints.push_back(tr("Tip: Click the snap button to auto-detect documents"));
    // Detect OS for keyboard shortcut hint
#ifdef Q_OS_MACOS
    hints.push_back(tr("Tip: Press Cmd+U to upload more images"));
#else
    hints.push_back(tr("Tip: Press Ctrl+U to upload more images"));
#endif

    // Hint label for displaying rotating hints
    hintLabel = new QLabel(this);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet("QLabel { color: #888; font-style: italic; padding: 8px; }");
    hintLabel->setText(hints[0]);
    hintLabel->hide();  // Hidden initially, shown in processing view

    // Timer to rotate hints every 5 seconds
    hintTimer = new QTimer(this);
    connect(hintTimer, &QTimer::timeout, this, &MainWindow::rotateHint);

    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(exportButton);

    // Note: uploadButton was removed as users can click directly on the drop zone

    // Processing view with 1:3 column layout
    processingView = new QWidget(this);
    QHBoxLayout *processingLayout = new QHBoxLayout(processingView);
    processingLayout->setContentsMargins(0, 0, 0, 0);

    // Left column (1 part): Thumbnail list
    thumbnailScrollArea = new QScrollArea(processingView);
    thumbnailContainer = new QWidget(thumbnailScrollArea);
    thumbnailLayout = new QVBoxLayout(thumbnailContainer);
    thumbnailLayout->setAlignment(Qt::AlignTop);
    thumbnailContainer->setLayout(thumbnailLayout);
    thumbnailScrollArea->setWidget(thumbnailContainer);
    thumbnailScrollArea->setWidgetResizable(true);
    thumbnailScrollArea->setMinimumWidth(200);
    thumbnailScrollArea->setMaximumWidth(300);

    // Right column (3 parts): Preview
    previewScrollArea = new QScrollArea(processingView);
    previewLabel = new QLabel(previewScrollArea);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setText(tr("Select an image to preview"));
    previewLabel->setMinimumSize(400, 400);
    previewScrollArea->setWidget(previewLabel);
    previewScrollArea->setWidgetResizable(true);

    // Add to processing layout with 1:3 ratio
    processingLayout->addWidget(thumbnailScrollArea, 1);
    processingLayout->addWidget(previewScrollArea, 3);

    mainLayout->addWidget(processingView);
    mainLayout->addWidget(hintLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(imageLayout);

    // Connections
    connect(processButton, &QPushButton::clicked, this, &MainWindow::onProcessClicked);
    connect(rotateLeftButton, &QPushButton::clicked, this, &MainWindow::onRotateLeftClicked);
    connect(rotateRightButton, &QPushButton::clicked, this, &MainWindow::onRotateRightClicked);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::onExportClicked);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::onBackClicked);

    // Keyboard shortcuts
    // Use Cmd+U on macOS, Ctrl+U on Linux/Windows
    // Qt::CTRL automatically maps to Cmd on macOS and Ctrl on other platforms
    uploadShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_U), this);
    connect(uploadShortcut, &QShortcut::activated, this, &MainWindow::onUploadClicked);

    // Initial view: only show drop zone (next button shown when images are staged)
    nextButton->hide();
    processButton->hide();
    rotateLeftButton->hide();
    rotateRightButton->hide();
    exportButton->hide();
    backButton->hide();
    stagingScrollArea->hide();
    processingView->hide();
}

// Rotate action no-op (unsupported in batch mode)
void MainWindow::onRotateLeftClicked()
{
    // Rotation is disabled when processing multiple images
}

// Rotate action no-op (unsupported in batch mode)
void MainWindow::onRotateRightClicked()
{
    // Rotation is disabled when processing multiple images
}

void MainWindow::onUploadClicked()
{
    // Select and add images to staging
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Select Images"), QString(),
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)"));
    if (fileNames.isEmpty())
        return;
    onFilesDropped(fileNames);
}

// Handle files dropped or selected for staging
void MainWindow::onFilesDropped(const QStringList &fileNames)
{
    if (fileNames.isEmpty())
        return;
    // Add new files to staging without clearing previous
    const int thumbnailWidth = 150;
    const int thumbnailHeight = 150;
    for (const QString &fileName : fileNames) {
        // Skip duplicates
        if (std::find(stagedFilenames.begin(), stagedFilenames.end(), fileName) != stagedFilenames.end())
            continue;
        cv::Mat img = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
        if (img.empty())
            continue;
        stagedImages.push_back(img);
        stagedFilenames.push_back(fileName);
        // Create thumbnail and delete icon (vertical layout)
        QWidget *itemWidget = new QWidget(this);
        itemWidget->setFixedWidth(thumbnailWidth);
        QVBoxLayout *itemLayout = new QVBoxLayout(itemWidget);
        itemLayout->setContentsMargins(4, 4, 4, 4);
        itemLayout->setSpacing(4);

        QImage qimg = cvMatToQImage(img);
        QLabel *thumb = new QLabel(itemWidget);
        thumb->setPixmap(QPixmap::fromImage(qimg).scaled(thumbnailWidth - 8, thumbnailHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        thumb->setAlignment(Qt::AlignCenter);
        thumb->setFixedHeight(thumbnailHeight);
        itemLayout->addWidget(thumb, 0);  // Don't stretch

        // Add delete icon at the bottom using Qt native icon
        QToolButton *deleteButton = new QToolButton(itemWidget);
        deleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
        deleteButton->setIconSize(QSize(18, 18));
        deleteButton->setToolTip(tr("Remove image"));
        deleteButton->setStyleSheet("QToolButton { border: none; background: transparent; }");

        // Center the delete button
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        buttonLayout->addWidget(deleteButton);
        buttonLayout->addStretch();
        itemLayout->addLayout(buttonLayout);

        stagingLayout->addWidget(itemWidget);
        stagingWidgets.push_back(itemWidget);
        // Connect removal
        connect(deleteButton, &QToolButton::clicked, this, [this, itemWidget]() {
            auto it = std::find(stagingWidgets.begin(), stagingWidgets.end(), itemWidget);
            if (it != stagingWidgets.end()) {
                int idx = std::distance(stagingWidgets.begin(), it);
                stagingWidgets.erase(it);
                stagedImages.erase(stagedImages.begin() + idx);
                stagedFilenames.erase(stagedFilenames.begin() + idx);
                stagingLayout->removeWidget(itemWidget);
                delete itemWidget;
                // Hide staging area and next button if no files remain
                if (stagedImages.empty()) {
                    stagingScrollArea->hide();
                    nextButton->hide();
                }
            }
        });
    }
    // Show or hide staging area and next button based on whether we have images
    if (!stagedImages.empty()) {
        stagingScrollArea->show();
        nextButton->show();
    } else {
        stagingScrollArea->hide();
        nextButton->hide();
    }
}

// Reorder thumbnails when drag and drop occurs
void MainWindow::reorderThumbnails(ThumbnailWidget *fromWidget, ThumbnailWidget *toWidget)
{
    if (!fromWidget || !toWidget || fromWidget == toWidget)
        return;

    int fromIndex = getThumbnailIndex(fromWidget);
    int toIndex = getThumbnailIndex(toWidget);

    if (fromIndex < 0 || toIndex < 0)
        return;

    // Reorder thumbnailWidgets vector (widgets maintain their state references)
    thumbnailWidgets.erase(thumbnailWidgets.begin() + fromIndex);
    thumbnailWidgets.insert(thumbnailWidgets.begin() + toIndex, fromWidget);

    // Update layout to reflect new order
    for (ThumbnailWidget *widget : thumbnailWidgets) {
        thumbnailLayout->removeWidget(widget);
    }
    for (ThumbnailWidget *widget : thumbnailWidgets) {
        thumbnailLayout->addWidget(widget);
    }

    // Note: We don't reorder processingStates because each widget has a direct pointer to its state
    // The state order doesn't matter for saving since we iterate through thumbnailWidgets
}

// Transition from staging to processing view
void MainWindow::onNextClicked()
{
    if (stagedImages.empty()) {
        QMessageBox::information(this, tr("No Images"), tr("Please add images before proceeding."));
        return;
    }

    // Initialize processing states from staged images
    processingStates.clear();
    for (size_t i = 0; i < stagedImages.size(); ++i) {
        ImageProcessingState state;
        state.originalImage = stagedImages[i].clone();
        state.currentImage = stagedImages[i].clone();
        state.filename = stagedFilenames[i];
        state.rotationAngle = 0;
        state.isSnapped = false;
        processingStates.push_back(state);
    }

    // Clear old thumbnails
    for (ThumbnailWidget *widget : thumbnailWidgets) {
        thumbnailLayout->removeWidget(widget);
        delete widget;
    }
    thumbnailWidgets.clear();

    // Create thumbnail widgets - each manages its own state
    for (size_t i = 0; i < processingStates.size(); ++i) {
        ThumbnailWidget *thumbWidget = new ThumbnailWidget(&processingStates[i], thumbnailContainer);

        // Connect signals
        connect(thumbWidget, &ThumbnailWidget::thumbnailClicked, this, &MainWindow::onThumbnailClicked);
        connect(thumbWidget, &ThumbnailWidget::thumbnailReordered, this, &MainWindow::reorderThumbnails);
        connect(thumbWidget, &ThumbnailWidget::imageModified, this, &MainWindow::onImageModified);

        thumbnailLayout->addWidget(thumbWidget);
        thumbnailWidgets.push_back(thumbWidget);
    }

    // Switch visibility first
    dropZone->hide();
    nextButton->hide();
    stagingScrollArea->hide();
    processButton->hide();
    rotateLeftButton->hide();
    rotateRightButton->hide();
    backButton->show();
    exportButton->show();
    exportButton->setEnabled(true);
    processingView->show();

    // Select first image by default (after showing processingView so dimensions are correct)
    if (!thumbnailWidgets.empty()) {
        currentThumbnail = thumbnailWidgets[0];
        currentThumbnail->setSelected(true);
        updatePreview();
    }

    // Show hint label and start rotating hints
    hintLabel->show();
    currentHintIndex = 0;
    hintLabel->setText(hints[0]);
    hintTimer->start(5000);  // Rotate hints every 5 seconds
}

void MainWindow::onProcessClicked()
{
    // This function is no longer used in the new workflow
}

QImage MainWindow::cvMatToQImage(const cv::Mat &mat)
{
    // Convert based on mat type
    if (mat.empty())
        return QImage();
    switch (mat.type()) {
    case CV_8UC1: {
        QImage img(mat.cols, mat.rows, QImage::Format_Grayscale8);
        memcpy(img.bits(), mat.data, static_cast<size_t>(mat.total() * mat.elemSize()));
        return img;
    }
    case CV_8UC3: {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
        return img.copy(); // deep copy because cv::Mat will be freed
    }
    case CV_8UC4: {
        QImage img(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
        return img.copy();
    }
    default:
        // Unsupported format
        return QImage();
    }
}

cv::Mat MainWindow::qImageToCvMat(const QImage &image)
{
    switch (image.format()) {
    case QImage::Format_Grayscale8: {
        cv::Mat mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), static_cast<size_t>(image.bytesPerLine()));
        return mat.clone();
    }
    case QImage::Format_RGB888: {
        cv::Mat mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), static_cast<size_t>(image.bytesPerLine()));
        cv::Mat bgr;
        cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
        return bgr.clone();
    }
    case QImage::Format_ARGB32: {
        cv::Mat mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), static_cast<size_t>(image.bytesPerLine()));
        return mat.clone();
    }
    default:
        return cv::Mat();
    }
}

// Handle thumbnail click to select and preview an image
void MainWindow::onThumbnailClicked(ThumbnailWidget *widget)
{
    if (!widget)
        return;

    // Deselect previous thumbnail
    if (currentThumbnail) {
        currentThumbnail->setSelected(false);
    }

    // Select new thumbnail
    currentThumbnail = widget;
    currentThumbnail->setSelected(true);

    // Update preview
    updatePreview();
}

// Handle image modification (rotation, snapping, etc.)
void MainWindow::onImageModified(ThumbnailWidget *widget)
{
    // Update preview if this is the currently selected image
    if (widget == currentThumbnail) {
        updatePreview();
    }
}

// Update the preview pane with the currently selected image
void MainWindow::updatePreview()
{
    if (!currentThumbnail) {
        previewLabel->setText(tr("Select an image to preview"));
        previewLabel->setPixmap(QPixmap());
        return;
    }

    const ImageProcessingState *state = currentThumbnail->getState();
    if (!state) {
        previewLabel->setText(tr("Select an image to preview"));
        previewLabel->setPixmap(QPixmap());
        return;
    }

    const QImage qimg = cvMatToQImage(state->currentImage);

    // Scale to fit preview area while maintaining aspect ratio
    const int maxWidth = previewScrollArea->width() - 20;
    const int maxHeight = previewScrollArea->height() - 20;

    QPixmap pixmap = QPixmap::fromImage(qimg);
    if (pixmap.width() > maxWidth || pixmap.height() > maxHeight) {
        pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    previewLabel->setPixmap(pixmap);
    previewLabel->setText(QString());
}

// Get the index of a thumbnail widget in the thumbnailWidgets vector
int MainWindow::getThumbnailIndex(ThumbnailWidget *widget) const
{
    auto it = std::find(thumbnailWidgets.begin(), thumbnailWidgets.end(), widget);
    if (it == thumbnailWidgets.end())
        return -1;
    return static_cast<int>(std::distance(thumbnailWidgets.begin(), it));
}

// Rotate through the hint messages
void MainWindow::rotateHint()
{
    if (hints.empty())
        return;

    currentHintIndex = (currentHintIndex + 1) % static_cast<int>(hints.size());
    hintLabel->setText(hints[currentHintIndex]);
}

// Handle back button click to return to upload view
void MainWindow::onBackClicked()
{
    // Show confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("Discard Changes?"),
        tr("Going back will discard all changes (rotations, snapping, etc.).\n\nAre you sure you want to continue?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // Stop hint timer
    hintTimer->stop();

    // Hide processing view
    processingView->hide();
    backButton->hide();
    exportButton->hide();
    hintLabel->hide();

    // Show upload view
    dropZone->show();
    if (!stagedImages.empty()) {
        stagingScrollArea->show();
        nextButton->show();
    }

    // Clear current thumbnail selection
    if (currentThumbnail) {
        currentThumbnail->setSelected(false);
        currentThumbnail = nullptr;
    }
}

// Handle export button click
void MainWindow::onExportClicked()
{
    if (thumbnailWidgets.empty())
        return;

    // Show export dialog
    ExportDialog dialog(static_cast<int>(thumbnailWidgets.size()), this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    ExportFormat exportFormat = dialog.getExportFormat();

    if (exportFormat == ExportFormat::PDF) {
        // Export to PDF
        QString filePath = QFileDialog::getSaveFileName(this,
            tr("Export to PDF"), QString(),
            tr("PDF Files (*.pdf)"));
        if (filePath.isEmpty())
            return;

        exportToPdf(filePath);
    } else {
        // Export to images
        QString directory = QFileDialog::getExistingDirectory(this,
            tr("Select Export Directory"));
        if (directory.isEmpty())
            return;

        QString formatExtension;
        switch (exportFormat) {
        case ExportFormat::PNG:
            formatExtension = "png";
            break;
        case ExportFormat::JPEG:
            formatExtension = "jpg";
            break;
        case ExportFormat::BMP:
            formatExtension = "bmp";
            break;
        default:
            formatExtension = "png";
            break;
        }

        exportToImages(directory, formatExtension);
    }
}

// Export all images to a directory
void MainWindow::exportToImages(const QString &directory, const QString &format)
{
    int successCount = 0;
    int failCount = 0;

    for (size_t i = 0; i < thumbnailWidgets.size(); ++i) {
        const ImageProcessingState *state = thumbnailWidgets[i]->getState();
        if (!state)
            continue;

        QImage qimg = cvMatToQImage(state->currentImage);
        QString base = QFileInfo(state->filename).completeBaseName();
        QString outPath = directory + "/" + base + "_processed." + format;

        if (qimg.save(outPath)) {
            successCount++;
        } else {
            failCount++;
        }
    }

    // Show result message
    if (failCount == 0) {
        QMessageBox::information(this, tr("Export Complete"),
            tr("Successfully exported %1 image(s) to:\n%2").arg(successCount).arg(directory));
    } else {
        QMessageBox::warning(this, tr("Export Completed with Errors"),
            tr("Exported %1 image(s) successfully.\nFailed to export %2 image(s).")
            .arg(successCount).arg(failCount));
    }
}

// Export all images to a single PDF file
void MainWindow::exportToPdf(const QString &filePath)
{
    if (thumbnailWidgets.empty())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);

    QPainter painter;
    bool firstPage = true;

    for (size_t i = 0; i < thumbnailWidgets.size(); ++i) {
        const ImageProcessingState *state = thumbnailWidgets[i]->getState();
        if (!state)
            continue;

        QImage qimg = cvMatToQImage(state->currentImage);
        if (qimg.isNull())
            continue;

        // Set page size to match image dimensions (in points, 72 DPI)
        // Convert pixel dimensions to points (1 point = 1/72 inch)
        const double dpi = 300.0;  // Assume 300 DPI for image
        const double pointsPerPixel = 72.0 / dpi;
        const QSizeF pageSize(qimg.width() * pointsPerPixel, qimg.height() * pointsPerPixel);
        printer.setPageSize(QPageSize(pageSize, QPageSize::Point));

        // Start painter on first page or add new page
        if (firstPage) {
            if (!painter.begin(&printer)) {
                QMessageBox::warning(this, tr("Export Error"),
                    tr("Failed to create PDF file: %1").arg(filePath));
                return;
            }
            firstPage = false;
        } else {
            if (!printer.newPage()) {
                QMessageBox::warning(this, tr("Export Error"),
                    tr("Failed to add page %1 to PDF").arg(i + 1));
                painter.end();
                return;
            }
        }

        // Get page rect and draw image to fill entire page
        QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
        painter.drawImage(pageRect, qimg);
    }

    painter.end();

    QMessageBox::information(this, tr("Export Complete"),
        tr("Successfully exported %1 image(s) to PDF:\n%2")
        .arg(thumbnailWidgets.size()).arg(filePath));
}
