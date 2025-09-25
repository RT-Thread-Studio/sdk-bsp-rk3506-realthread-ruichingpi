/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef APPLICATIONS_YOLO_HPP_
#define APPLICATIONS_YOLO_HPP_

#ifndef __YOLO_HPP__
#define __YOLO_HPP__

#include "net.h"
#include <vector>
#include <iostream>
#include <rtthread.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

template<typename _Tp>
static inline double ccjaccardDistance(const cv::Rect_<_Tp>& a, const cv::Rect_<_Tp>& b);

template<typename T>
static inline float rectOverlap(const T& a, const T& b);

template<typename T>
static inline bool SortScorePairDescend(const std::pair<float, T>& pair1, const std::pair<float, T>& pair2);

inline void GetMaxScoreIndex(const std::vector<float>& scores, const float threshold, const int top_k,
        std::vector<std::pair<float, int> >& score_index_vec);

template<typename BoxType>
inline void NMSFast_(const std::vector<BoxType>& bboxes, const std::vector<float>& scores, const float score_threshold,
        const float nms_threshold, const float eta, const int top_k, std::vector<int>& indices,
        float (*computeOverlap)(const BoxType&, const BoxType&), int limit = std::numeric_limits<int>::max());

void NMSBoxes(const std::vector<cv::Rect>& bboxes, const std::vector<float>& scores, const float score_threshold,
        const float nms_threshold, std::vector<int>& indices, const float eta = 1.0f, const int top_k = 0);

static bool box_compare(const std::vector<float> &a, const std::vector<float> &b);

class IoUFilter
{
public:
    IoUFilter(const std::vector<float>& best_box, float iou_thresh);
    bool operator()(const std::vector<float>& box) const;

private:
    static float calculate_iou(const std::vector<float> &a, const std::vector<float> &b);
    const std::vector<float>& best;
    float threshold;
};

void pretty_print(const ncnn::Mat& m);

class YOLONCNN
{
public:
    YOLONCNN(const std::string &param_path, const std::string &bin_path);

    ncnn::Mat preprocess(const cv::Mat &img);

    std::vector<cv::Rect> decode_output(const ncnn::Mat &output, std::vector<float> &scores, int anchor_idx);

    std::vector<std::vector<float>> nms(std::vector<std::vector<float>> &boxes, float iou_thresh = 0.4f);

    std::vector<std::vector<float>> nms2(std::vector<cv::Rect> &boxes, std::vector<float> &scores, float iou_thresh =
            0.4f);

    std::vector<std::vector<float>> detect(const cv::Mat &img);

    void visualize(cv::Mat &img, const std::vector<std::vector<float>> &boxes, const std::string& output_path);

    bool visualize_to_buf(cv::Mat& img, const std::vector<std::vector<float>>& boxes,
            const std::string& format = ".jpg", int quality = 90);
private:
    ncnn::Net net;
    std::vector<std::vector<int>> anchors;
    int num_classes;
    int input_size;
    int orig_h, orig_w;
    float sigmoid(float x);
};

template<typename _Tp>
inline double ccjaccardDistance(const cv::Rect_<_Tp>& a, const cv::Rect_<_Tp>& b)
{
    _Tp Aa = a.area();
    _Tp Ab = b.area();

    if ((Aa + Ab) <= std::numeric_limits<_Tp>::epsilon())
    {
        return 0.0;
    }

    double Aab = (a & b).area();
    return 1.0 - Aab / (Aa + Ab - Aab);
}

template<typename T>
inline float rectOverlap(const T& a, const T& b)
{
    return 1.f - static_cast<float>(ccjaccardDistance(a, b));
}

template<typename T>
inline bool SortScorePairDescend(const std::pair<float, T>& pair1, const std::pair<float, T>& pair2)
{
    return pair1.first > pair2.first;
}

template<typename BoxType>
inline void NMSFast_(const std::vector<BoxType>& bboxes, const std::vector<float>& scores, const float score_threshold,
        const float nms_threshold, const float eta, const int top_k, std::vector<int>& indices,
        float (*computeOverlap)(const BoxType&, const BoxType&), int limit)
{
    std::vector<std::pair<float, int> > score_index_vec;
    GetMaxScoreIndex(scores, score_threshold, top_k, score_index_vec);

    float adaptive_threshold = nms_threshold;
    indices.clear();
    for (size_t i = 0; i < score_index_vec.size(); ++i)
    {
        const int idx = score_index_vec[i].second;
        bool keep = true;
        for (int k = 0; k < (int) indices.size() && keep; ++k)
        {
            const int kept_idx = indices[k];
            float overlap = computeOverlap(bboxes[idx], bboxes[kept_idx]);
            keep = overlap <= adaptive_threshold;
        }
        if (keep)
        {
            indices.push_back(idx);
            if (indices.size() >= limit)
            {
                break;
            }
        }
        if (keep && eta < 1 && adaptive_threshold > 0.5)
        {
            adaptive_threshold *= eta;
        }
    }
}

#endif // __YOLO_HPP__

#endif /* APPLICATIONS_YOLO_HPP_ */
