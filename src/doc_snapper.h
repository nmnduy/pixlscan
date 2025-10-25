#ifndef DOC_SNAPPER_H
#define DOC_SNAPPER_H

#include <opencv2/opencv.hpp>
#include <optional>

/**
 * Snap a photographed document to a top‑down, perspective‑corrected view.
 *
 * @param image Input image containing a document (may be rotated, skewed, etc.)
 * @return An {@code std::optional<cv::Mat>} containing the processed image.
 *         If no document can be detected, the optional is empty.
 */
/**
 * Snap a photographed document to a top-down, perspective-corrected view.
 *
 * @param image       Input image containing a document.
 * @param returnColor If true, returns the color-corrected image; if false, returns a B/W scanned look.
 * @return An {@code std::optional<cv::Mat>} containing the processed image.
 *         If no document can be detected, the optional is empty.
 */
std::optional<cv::Mat> snapDocument(const cv::Mat& image, bool returnColor = true);

#endif // DOC_SNAPPER_H
