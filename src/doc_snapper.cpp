#include "doc_snapper.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "Logger.hpp"
#include <algorithm>

using namespace cv;
using namespace std;

static vector<Point2f> orderPoints(const vector<Point>& pts) {
    vector<Point2f> pts2f(pts.begin(), pts.end());
    // Sort by x (left to right)
    sort(pts2f.begin(), pts2f.end(), [](const Point2f& a, const Point2f& b) {
        return a.x < b.x;
    });

    vector<Point2f> left(pts2f.begin(), pts2f.begin() + 2);
    vector<Point2f> right(pts2f.begin() + 2, pts2f.end());

    sort(left.begin(), left.end(), [](const Point2f& a, const Point2f& b) {
        return a.y < b.y;
    });
    sort(right.begin(), right.end(), [](const Point2f& a, const Point2f& b) {
        return a.y < b.y;
    });

    return { left[0], right[0], right[1], left[1] }; // TL, TR, BR, BL
}


static Mat fourPointTransform(const Mat& image, const vector<Point2f>& ordered) {
    Point2f tl = ordered[0];
    Point2f tr = ordered[1];
    Point2f br = ordered[2];
    Point2f bl = ordered[3];

    double widthA = norm(br - bl);
    double widthB = norm(tr - tl);
    double maxWidth = max(widthA, widthB);
    double heightA = norm(tr - br);
    double heightB = norm(tl - bl);
    double maxHeight = max(heightA, heightB);
    // Debug: log dimensions
    Logger::debug(std::string("fourPointTransform: widthA=") + std::to_string(widthA) + " widthB=" + std::to_string(widthB) + " maxWidth=" + std::to_string(maxWidth) + " heightA=" + std::to_string(heightA) + " heightB=" + std::to_string(heightB) + " maxHeight=" + std::to_string(maxHeight));

    Point2f src[4] = { tl, tr, br, bl };
    Point2f dst[4] = {
        {0, 0},
        {static_cast<float>(maxWidth - 1), 0},
        {static_cast<float>(maxWidth - 1), static_cast<float>(maxHeight - 1)},
        {0, static_cast<float>(maxHeight - 1)}
    };
    Mat M = getPerspectiveTransform(src, dst);
    Mat warped;
    warpPerspective(image, warped, M, Size(static_cast<int>(maxWidth), static_cast<int>(maxHeight)));
    return warped;
}

std::optional<cv::Mat> snapDocument(const cv::Mat& image, bool returnColor) {
    if (image.empty()) {
        Logger::error("snapDocument: empty input image");
        return std::nullopt;
    }
    // 1. Pre-process: downsample, gray, blur, and edge-detect
    constexpr int kResizeWidth = 600;
    const double ratio = static_cast<double>(image.cols) / kResizeWidth;
    Mat resized;
    cv::resize(image, resized,
               Size(kResizeWidth, static_cast<int>(image.rows / ratio)),
               0, 0, INTER_AREA);
    Mat gray, blurred, edged;
    cvtColor(resized, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(5, 5), 0);
    Canny(blurred, edged, 75, 200);

    // 2. Find contours on resized image
    vector<vector<Point>> contours;
    findContours(edged, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

    // 3. Locate the largest 4‑point convex contour
    vector<Point> docContour;
    double maxArea = 0.0;
    for (const auto& cnt : contours) {
        vector<Point> approx;
        double peri = arcLength(cnt, true);
        approxPolyDP(cnt, approx, 0.02 * peri, true);
        if (approx.size() == 4 && isContourConvex(approx)) {
            double area = fabs(contourArea(approx));
            if (area > maxArea) {
                maxArea = area;
                docContour = approx;
                // Log new max contour
                Logger::debug("snapDocument: new max contour area=" + std::to_string(maxArea) + " pts:");
                for (const auto& p : docContour) Logger::debug("max contour point (" + std::to_string(p.x) + "," + std::to_string(p.y) + ")");
                Logger::debug("");
            }
        }
    }
    if (docContour.empty()) {
        Logger::warn("snapDocument: no document contour found");
        return std::nullopt;
    }
    // Scale contour points back to original image size
    vector<Point> scaledContour;
    scaledContour.reserve(docContour.size());
    for (const auto& p : docContour) {
        scaledContour.emplace_back(
            static_cast<int>(p.x * ratio),
            static_cast<int>(p.y * ratio)
        );
    }

    // 4. Warp perspective on original image
    // Debug: log selected (scaled) contour
    Logger::debug("snapDocument: selected docContour points:");
    for (const auto& p : scaledContour) Logger::debug("scaled contour point (" + std::to_string(p.x) + "," + std::to_string(p.y) + ")");
    Logger::debug("");
    auto ordered = orderPoints(scaledContour);
    // 4.a Refine corner points to subpixel accuracy
    Mat grayOrig;
    cvtColor(image, grayOrig, COLOR_BGR2GRAY);
    const cv::Size winSize(5, 5);
    const cv::Size zeroZone(-1, -1);
    const TermCriteria criteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.1);
    cornerSubPix(grayOrig, ordered, winSize, zeroZone, criteria);
    // Debug: log ordered corners
    Logger::debug("snapDocument: ordered corners TL=" + std::to_string(ordered[0].x) + "," + std::to_string(ordered[0].y) + " TR=" + std::to_string(ordered[1].x) + "," + std::to_string(ordered[1].y) + " BR=" + std::to_string(ordered[2].x) + "," + std::to_string(ordered[2].y) + " BL=" + std::to_string(ordered[3].x) + "," + std::to_string(ordered[3].y));
    // 5. Warp full-color image and return per mode
    Mat warped = fourPointTransform(image, ordered);
    if (returnColor) {
        return warped;  // full-color perspective-corrected image
    } else {
        // scanner-like B/W enhancement
        Mat warpedGray, enhanced;
        cvtColor(warped, warpedGray, COLOR_BGR2GRAY);
        adaptiveThreshold(warpedGray, enhanced,
                          255, ADAPTIVE_THRESH_MEAN_C,
                          THRESH_BINARY, 15, 10);
        return enhanced;
    }
}
