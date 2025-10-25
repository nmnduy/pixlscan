#include "export_dialog.h"
#include <QGroupBox>
#include <QLabel>

ExportDialog::ExportDialog(int count, QWidget *parent)
    : QDialog(parent), imageCount(count)
{
    setWindowTitle(tr("Export Options"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Info label
    QLabel *infoLabel = new QLabel(
        tr("Export %1 processed image%2").arg(imageCount).arg(imageCount > 1 ? "s" : ""),
        this
    );
    infoLabel->setStyleSheet("QLabel { font-weight: bold; padding: 8px; }");
    mainLayout->addWidget(infoLabel);

    // Export format options
    QGroupBox *exportGroup = new QGroupBox(tr("Export"), this);
    QHBoxLayout *exportLayout = new QHBoxLayout(exportGroup);

    QLabel *formatLabel = new QLabel(tr("Choose format:"), exportGroup);
    formatCombo = new QComboBox(exportGroup);
    formatCombo->addItem("PDF", static_cast<int>(ExportFormat::PDF));
    formatCombo->addItem("PNG", static_cast<int>(ExportFormat::PNG));
    formatCombo->addItem("JPEG", static_cast<int>(ExportFormat::JPEG));
    formatCombo->addItem("BMP", static_cast<int>(ExportFormat::BMP));
    formatCombo->setMinimumWidth(100);
    formatCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    formatCombo->setCurrentIndex(0);  // PDF as default

    exportLayout->addWidget(formatLabel);
    exportLayout->addWidget(formatCombo);
    exportLayout->addStretch();

    mainLayout->addWidget(exportGroup);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

ExportFormat ExportDialog::getExportFormat() const
{
    return static_cast<ExportFormat>(formatCombo->currentData().toInt());
}
