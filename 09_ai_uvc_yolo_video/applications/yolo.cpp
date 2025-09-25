/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "yolo.hpp"
#include "jpeg.h"

static bool box_compare(const std::vector<float> &a, const std::vector<float> &b)
{
    return a[4] > b[4];
}

IoUFilter::IoUFilter(const std::vector<float>& best_box, float iou_thresh) :
        best(best_box), threshold(iou_thresh)
{
}

bool IoUFilter::operator()(const std::vector<float>& box) const
{
    return calculate_iou(best, box) < threshold;
}

float IoUFilter::calculate_iou(const std::vector<float> &a, const std::vector<float> &b)
{
    float x1 = std::max(a[0], b[0]);
    float y1 = std::max(a[1], b[1]);
    float x2 = std::min(a[2], b[2]);
    float y2 = std::min(a[3], b[3]);
    float inter = std::max(0.f, x2 - x1) * std::max(0.f, y2 - y1);
    float area_a = (a[2] - a[0]) * (a[3] - a[1]);
    float area_b = (b[2] - b[0]) * (b[3] - b[1]);
    return inter / (area_a + area_b - inter + 1e-5f);
}

void pretty_print(const ncnn::Mat& m)
{
    for (int q = 0; q < m.c; q++)
    {
        const float* ptr = m.channel(q);
        for (int y = 0; y < m.h; y++)
        {
            for (int x = 0; x < m.w; x++)
            {
                printf("%f ", ptr[x]);
            }
            ptr += m.w;
            printf("\n");
        }
        printf("------------------------\n");
    }
}

inline void GetMaxScoreIndex(const std::vector<float>& scores, const float threshold, const int top_k,
        std::vector<std::pair<float, int> >& score_index_vec)
{
    CV_DbgAssert(score_index_vec.empty());
    for (size_t i = 0; i < scores.size(); ++i)
    {
        if (scores[i] > threshold)
        {
            score_index_vec.push_back(std::make_pair(scores[i], i));
        }
    }

    std::stable_sort(score_index_vec.begin(), score_index_vec.end(), SortScorePairDescend<int>);

    if (top_k > 0 && top_k < (int) score_index_vec.size())
    {
        score_index_vec.resize(top_k);
    }
}

void NMSBoxes(const std::vector<cv::Rect>& bboxes, const std::vector<float>& scores, const float score_threshold,
        const float nms_threshold, std::vector<int>& indices, const float eta, const int top_k)
{
    // CV_Assert_N(bboxes.size() == scores.size(), score_threshold >= 0,
    //     nms_threshold >= 0, eta > 0);
    NMSFast_(bboxes, scores, score_threshold, nms_threshold, eta, top_k, indices, rectOverlap);
}

YOLONCNN::YOLONCNN(const std::string &param_path, const std::string &bin_path)
{
    net.load_param(param_path.c_str());
    net.load_model(bin_path.c_str());

    anchors =
    {
        {   37, 94},
        {   83, 83},
        {   60, 137},
        {   15, 22},
        {   24, 51},
        {   60, 44}
    };
    num_classes = 1;
    input_size = 192;
}

ncnn::Mat YOLONCNN::preprocess(const cv::Mat &img)
{
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    orig_h = gray.rows;
    orig_w = gray.cols;

    cv::resize(gray, gray, cv::Size(input_size, input_size));
    ncnn::Mat in(input_size, input_size, 1);
    for (int i = 0; i < input_size; i++)
    {
        for (int j = 0; j < input_size; j++)
        {
            in[i * input_size + j] = gray.at<uchar>(i, j) / 255.f;
        }
    }
    return in;
}

std::vector<cv::Rect> YOLONCNN::decode_output(const ncnn::Mat &output, std::vector<float> &scores, int anchor_idx)
{
    const float *data = output.channel(0);
    // std::cout<<"out:w="<<output.w<<"h:="<<output.h<<"c:="<<output.c<<std::endl;
    int grid_h = output.h;
    int grid_w = output.w;
    float stride = 1.0f * input_size / grid_h;

    const auto &layer_anchors =
            (anchor_idx == 0) ?
                    std::vector<std::vector<int>>(anchors.begin(), anchors.begin() + 3) :
                    std::vector<std::vector<int>>(anchors.begin() + 3, anchors.end());
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < grid_h; i++)
    {
        for (int j = 0; j < grid_w; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                int offset = (i * grid_w + j) * (5 + num_classes) + k * (5 + num_classes);
                float tx = data[offset];
                float ty = data[offset + 1];
                float tw = data[offset + 2];
                float th = data[offset + 3];
                float conf = data[offset + 4];

                conf = 1.f / (1.f + exp(-conf));
                if (conf < 0.5f)
                    continue;

                float bx = (j + 1.f / (1.f + exp(-tx))) / grid_w;
                float by = (i + 1.f / (1.f + exp(-ty))) / grid_h;
                float bw = layer_anchors[k][0] * exp(tw) / input_size;
                float bh = layer_anchors[k][1] * exp(th) / input_size;

                int x1 = (bx - bw / 2) * orig_w;
                int y1 = (by - bh / 2) * orig_h;
                int x2 = (bx + bw / 2) * orig_w;
                int y2 = (by + bh / 2) * orig_h;
                if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0)
                    continue;

                // std::cout << "Decoded box: [" << x1 << "," << y1 << "," << x2 << ","
                //         << y2 << "], conf=" << conf << std::endl;
                // boxes.push_back({(float)x1, (float)y1, (float)x2, (float)y2, conf});
                boxes.emplace_back((float) x1, (float) y1, (float) x2 - (float) x1, (float) y2 - (float) y1);
                scores.push_back(conf);
            }
        }
    }
    return boxes;
}

std::vector<std::vector<float>> YOLONCNN::nms(std::vector<std::vector<float>> &boxes, float iou_thresh)
{
    // std::cout << "Before NMS: " << boxes.size() << std::endl;
    // rt_kprintf("Before:%d\n",boxes.size());
    std::sort(boxes.begin(), boxes.end(), box_compare);
    std::vector<std::vector<float>> result;
    while (!boxes.empty())
    {
        auto best = boxes[0];
        result.push_back(best);
        boxes.erase(boxes.begin());

        IoUFilter filter(best, iou_thresh);
        boxes.erase(std::remove_if(boxes.begin(), boxes.end(), filter), boxes.end());
    }
    // std::cout << "After: " << result.size() << std::endl;
    // rt_kprintf("After:%d\n",result.size());
    return result;
}

std::vector<std::vector<float>> YOLONCNN::nms2(std::vector<cv::Rect> &boxes, std::vector<float> &scores,
        float iou_thresh)
{

    float score_thresh = 0.5f;

    std::vector<std::vector<float>> result;

    std::vector<int> indices;

    NMSBoxes(boxes, scores, score_thresh, iou_thresh, indices, 1.0f, 0.0f);

    for (int i : indices)
    {
        result.push_back(std::vector<float> { static_cast<float>(boxes[i].x), static_cast<float>(boxes[i].y),
                static_cast<float>(boxes[i].x + boxes[i].width), static_cast<float>(boxes[i].y + boxes[i].height),
                scores[i] });
    }

    // std::cout << "After: " << result.size() << std::endl;
    return result;
}

std::vector<std::vector<float>> YOLONCNN::detect(const cv::Mat &img)
{
    ncnn::Mat in = preprocess(img);

    ncnn::Extractor ex = net.create_extractor();
    ex.input("data", in);

    ncnn::Mat out1, out2;
    ex.extract("conv2d_52", out1);
    ex.extract("detection_out", out2);
    std::vector<float> score1, score2;
    auto boxes1 = decode_output(out1, score1, 0);
    auto boxes2 = decode_output(out2, score2, 1);

    auto all_boxes = boxes1;
    auto all_score = score1;
    all_boxes.insert(all_boxes.end(), boxes2.begin(), boxes2.end());
    all_score.insert(all_score.end(), score2.begin(), score2.end());

    return nms2(all_boxes, all_score);
}

void YOLONCNN::visualize(cv::Mat &img, const std::vector<std::vector<float>> &boxes, const std::string& output_path)
{
    int inc = 0;
    float the_best_conf = 0.0f;
    int index = 0;
    for (const auto &box : boxes)
    {

        if (the_best_conf < box[4] && (std::fabs(box[4] - 1.0f) > 1e-6f))
        {
            the_best_conf = box[4];
            index = inc;
        }
        // cv::rectangle(img, cv::Point(box[0], box[1]), cv::Point(box[2], box[3]),
        //             cv::Scalar(inc * 30, 255, 0), 2);
        // cv::putText(img, cv::format("%.2f", box[4]),
        //             cv::Point(box[0], box[1] - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5,
        //             cv::Scalar(inc * 30, 255, 0), 1);
        inc++;
    }
    rt_kprintf("index:%d\n", index);
    const auto &EEbox = boxes[index];
    cv::rectangle(img, cv::Point(EEbox[0], EEbox[1]), cv::Point(EEbox[2], EEbox[3]), cv::Scalar(0, 0, 255), 2);
    //cv::putText(img, cv::format("%.2f", EEbox[4]), cv::Point(EEbox[0], EEbox[1] - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5,cv::Scalar(0, 0, 255), 1);
    cv::imwrite(output_path, img);
}
bool YOLONCNN::visualize_to_buf(cv::Mat &img, const std::vector<std::vector<float>> &boxes, const std::string& format, // 无默认值
        int quality)
{
    int inc = 0;
    float the_best_conf = 0.0f;
    int index = 0;

    for (const auto &box : boxes)
    {
        if (the_best_conf < box[4] && (std::fabs(box[4] - 1.0f) > 1e-6f))
        {
            the_best_conf = box[4];
            index = inc;
        }
        inc++;
    }

    rt_kprintf("index:%d\n", index);

    const auto &EEbox = boxes[index];
    cv::rectangle(img, cv::Point(EEbox[0], EEbox[1]), cv::Point(EEbox[2], EEbox[3]), cv::Scalar(0, 0, 255), 2);
    cv::putText(img, cv::format("%.2f", EEbox[4]), cv::Point(EEbox[0], EEbox[1] - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5,
            cv::Scalar(0, 0, 255), 1);

    if (g_img_buf != nullptr)
    {
        rt_free(g_img_buf);
        g_img_buf = nullptr;
        g_img_size = 0;
    }

    std::vector<uchar> temp_vec;
    std::vector<int> params;

    if (format == ".jpg" || format == ".jpeg")
    {
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(quality);
    }

    bool success = cv::imencode(format, img, temp_vec, params);

    if (!success)
    {
        rt_kprintf("Image encoding failed！\n");
        return false;
    }

    g_img_size = temp_vec.size();
    g_img_buf = (uint8_t*) rt_malloc(g_img_size);
    if (g_img_buf == nullptr)
    {
        rt_kprintf("Memory allocation failed！\n");
        g_img_size = 0;
        return false;
    }

    memcpy(g_img_buf, temp_vec.data(), g_img_size);
    return success;
}

float YOLONCNN::sigmoid(float x)
{
    return 1.f / (1.f + exp(-x));
}
