#include <rtthread.h>
#include <rtdevice.h>
#include "layer.h"
#include "net.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <float.h>
#include <stdio.h>
#include <vector>

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

static int detect_yolov3(const cv::Mat &bgr, std::vector<Object> &objects)
{
    ncnn::Net yolov3;

    yolov3.opt.use_vulkan_compute = true;

    // original pretrained model from https://github.com/eric612/MobileNet-YOLO
    // param : https://drive.google.com/open?id=1V9oKHP6G6XvXZqhZbzNKL6FI_clRWdC-
    // bin : https://drive.google.com/open?id=1DBcuFCr-856z3FRQznWL_S5h-Aj3RawA
    // the ncnn model https://github.com/nihui/ncnn-assets/tree/master/models
    if (yolov3.load_param("mobilenetv2_yolov3.param"))
        exit(-1);
    if (yolov3.load_model("mobilenetv2_yolov3.bin"))
        exit(-1);

    const int target_size = 352;

    int img_w = bgr.cols;
    int img_h = bgr.rows;

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR,
        bgr.cols, bgr.rows, target_size, target_size);

    const float mean_vals[3] = { 127.5f, 127.5f, 127.5f };
    const float norm_vals[3] = { 0.007843f, 0.007843f, 0.007843f };
    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = yolov3.create_extractor();

    ex.input("data", in);

    ncnn::Mat out;
    ex.extract("detection_out", out);

    rt_kprintf("%d %d %d\n", out.w, out.h, out.c);
    objects.clear();
    for (int i = 0; i < out.h; i++)
    {
        const float *values = out.row(i);

        Object object;
        object.label = values[0];
        object.prob = values[1];
        object.rect.x = values[2] * img_w;
        object.rect.y = values[3] * img_h;
        object.rect.width = values[4] * img_w - object.rect.x;
        object.rect.height = values[5] * img_h - object.rect.y;

        objects.push_back(object);
    }

    return 0;
}

static void draw_objects(const cv::Mat &bgr, const std::vector<Object> &objects,const std::string& output_path)
{
    static const char *class_names[] = { "background", "aeroplane", "bicycle",
        "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow",
        "diningtable", "dog", "horse", "motorbike", "person", "pottedplant",
        "sheep", "sofa", "train", "tvmonitor" };

    cv::Mat image = bgr.clone();

    for (size_t i = 0; i < objects.size(); i++)
    {
        const Object &obj = objects[i];

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label,
            obj.prob, obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        cv::rectangle(image, obj.rect, cv::Scalar(255, 0, 0));

        char text[256];
        sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

        int baseLine = 0;
        cv::Size label_size =
            cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 1, 1, &baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(image,
            cv::Rect(cv::Point(x, y),
                cv::Size(label_size.width, label_size.height + baseLine)),
            cv::Scalar(255, 0, 0), -1);

        cv::putText(image, text, cv::Point(x, y + label_size.height),
            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0));
    }

    cv::imwrite(output_path, image);
}

int mnet_yolov3_test(int argc, char *argv[])
{
    rt_kprintf("Hello RT-Thread NCNN\n");
    char default_input[]="bus.png",default_output[]="image1.jpg";
    char *input_img,*output_img;
    if (argc < 3)
    {
        input_img=default_input;
        output_img=default_output;
        rt_kprintf(" input_img=bus.png,output_img=image1.jpg\n");
    }
    else if (argc == 3)
    {
        input_img = argv[1];
        output_img = argv[2];
        rt_kprintf(" input_img=%s,output_img=%s\n",input_img,output_img);
    }
    else
    {
        rt_kprintf("input_img or output_img is invalid\n");
        return -1;
    }
    rt_uint32_t time_start = 0,time_end=0;
    rt_kprintf("execute imread start \n");
    time_start = rt_tick_get();
    cv::Mat m = cv::imread(input_img, 1);
    time_end = rt_tick_get();
    rt_kprintf("execute imread end : %d ms\n", time_end - time_start);
    if (m.empty())
    {
        rt_kprintf("cv::imread failed\n");
    }
    cv::Mat image = m.clone();
    std::vector<Object> objects;
    
    rt_kprintf("execute detect_yolov3 start \n");
    time_start = rt_tick_get();
    detect_yolov3(m, objects);
    time_end = rt_tick_get();
    rt_kprintf("execute detect_yolov3 end : %d ms\n", time_end - time_start);
    rt_kprintf("execute draw_objects start\n");
    time_start = rt_tick_get();
    draw_objects(m, objects,output_img);
    time_end = rt_tick_get();
    rt_kprintf("execute draw_objects end : %d ms\n", time_end - time_start);
    rt_kprintf("execute done\n");
    return 0;
}
MSH_CMD_EXPORT(mnet_yolov3_test, test yolo);