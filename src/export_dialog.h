#pragma once

#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>

enum class ExportFormat {
    PNG,
    JPEG,
    BMP,
    PDF
};

class ExportDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExportDialog(int imageCount, QWidget *parent = nullptr);
    ~ExportDialog() override = default;

    ExportFormat getExportFormat() const;

private:
    QComboBox *formatCombo{};
    int imageCount{0};
};
