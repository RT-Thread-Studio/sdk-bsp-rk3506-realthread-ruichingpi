/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtthread.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

static int canny_example(int argc, char *argv[])
{
    const char *default_input = "/sdmmc/input.jpg";
    const char *default_output = "/sdmmc/output.jpg";

    const char *input_path = default_input;
    const char *output_path = default_output;

    if (argc >= 2)
    {
        input_path = argv[1];
    }
    if (argc >= 3)
    {
        output_path = argv[2];
    }

    Mat image = imread(input_path, IMREAD_COLOR);
    if (image.empty())
    {
        rt_kprintf("Error: open %s failed\n", input_path);
        return -1;
    }

    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);

    Mat edges;
    Canny(grayImage, edges, 100, 200);

    bool result = imwrite(output_path, edges);
    if (!result)
    {
        rt_kprintf("Error: Failed to save the image to %s\n", output_path);
        return -1;
    }

    rt_kprintf("Success: Edge detection completed\n");

    return 0;
}
MSH_CMD_EXPORT(canny_example, canny_example);
