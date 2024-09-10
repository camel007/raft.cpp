#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <catch2/catch_approx.hpp>

#include "cuda_functional.hpp"

using Catch::Approx;

TEST_CASE("grid_sample 基本功能测试", "[grid_sample]")
{
    // 设置输入参数
    const int N = 1, C = 1, H_in = 4, W_in = 4, H_out = 2, W_out = 2;
    const int input_size  = N * C * H_in * W_in;
    const int grid_size   = N * H_out * W_out * 2;
    const int output_size = N * C * H_out * W_out;

    // 分配主机内存
    float* h_input           = new float[input_size];
    float* h_grid            = new float[grid_size];
    float* h_output          = new float[output_size];
    float* h_expected_output = new float[output_size];

    // 初始化输入数据
    for (int i = 0; i < input_size; ++i)
    {
        h_input[i] = static_cast<float>(i);
    }

    // 设置网格数据（简单的恒等变换）
    h_grid[0] = -1.0f;
    h_grid[1] = -1.0f;  // 左上
    h_grid[2] = 1.0f;
    h_grid[3] = -1.0f;  // 右上
    h_grid[4] = -1.0f;
    h_grid[5] = 1.0f;  // 左下
    h_grid[6] = 1.0f;
    h_grid[7] = 1.0f;  // 右下

    // 设置预期输出
    h_expected_output[0] = 0.0f;   // 左上角
    h_expected_output[1] = 3.0f;   // 右上角
    h_expected_output[2] = 12.0f;  // 左下角
    h_expected_output[3] = 15.0f;  // 右下角

    // 分配设备内存
    float *d_input, *d_grid, *d_output;
    cudaMalloc(&d_input, input_size * sizeof(float));
    cudaMalloc(&d_grid, grid_size * sizeof(float));
    cudaMalloc(&d_output, output_size * sizeof(float));

    // 将数据复制到设备
    cudaMemcpy(d_input, h_input, input_size * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_grid, h_grid, grid_size * sizeof(float), cudaMemcpyHostToDevice);

    // 调用grid_sample函数
    int error = grid_sample(d_input, d_grid, d_output, N, C, H_in, W_in, H_out, W_out);

    // 检查是否有CUDA错误
    REQUIRE(error == cudaSuccess);

    // 将结果复制回主机
    cudaMemcpy(h_output, d_output, output_size * sizeof(float), cudaMemcpyDeviceToHost);

    // 验证结果
    for (int i = 0; i < output_size; ++i)
    {
        REQUIRE(h_output[i] == Catch::Approx(h_expected_output[i]).epsilon(0.01f));
    }

    // 释放内存
    delete[] h_input;
    delete[] h_grid;
    delete[] h_output;
    delete[] h_expected_output;
    cudaFree(d_input);
    cudaFree(d_grid);
    cudaFree(d_output);
}
TEST_CASE("grid_sample resize 测试", "[grid_sample]")
{
    // 设置输入参数
    const int N = 1, C = 1, H_in = 4, W_in = 4, H_out = 2, W_out = 3;
    const int input_size  = N * C * H_in * W_in;
    const int grid_size   = N * H_out * W_out * 2;
    const int output_size = N * C * H_out * W_out;

    // 分配主机内存
    float* h_input           = new float[input_size];
    float* h_grid            = new float[grid_size];
    float* h_output          = new float[output_size];
    float* h_expected_output = new float[output_size];

    // 初始化输入数据
    for (int i = 0; i < input_size; ++i)
    {
        h_input[i] = static_cast<float>(i);
    }

    // 设置网格数据（均匀分布的采样点）
    for (int i = 0; i < H_out; ++i)
    {
        for (int j = 0; j < W_out; ++j)
        {
            h_grid[(i * W_out + j) * 2]     = -1.0f + 2.0f * j / (W_out - 1);  // x
            h_grid[(i * W_out + j) * 2 + 1] = -1.0f + 2.0f * i / (H_out - 1);  // y
        }
    }

    // 设置预期输出
    h_expected_output[0] = 0.0f;   // 左上
    h_expected_output[1] = 1.5f;   // 中上
    h_expected_output[2] = 3.0f;   // 右上
    h_expected_output[3] = 12.0f;  // 左下
    h_expected_output[4] = 13.5f;  // 中下
    h_expected_output[5] = 15.0f;  // 右下

    // 分配设备内存
    float *d_input, *d_grid, *d_output;
    cudaMalloc(&d_input, input_size * sizeof(float));
    cudaMalloc(&d_grid, grid_size * sizeof(float));
    cudaMalloc(&d_output, output_size * sizeof(float));

    // 将数据复制到设备
    cudaMemcpy(d_input, h_input, input_size * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_grid, h_grid, grid_size * sizeof(float), cudaMemcpyHostToDevice);

    // 调用grid_sample函数
    int error = grid_sample(d_input, d_grid, d_output, N, C, H_in, W_in, H_out, W_out);

    // 检查是否有CUDA错误
    REQUIRE(error == cudaSuccess);

    // 将结果复制回主机
    cudaMemcpy(h_output, d_output, output_size * sizeof(float), cudaMemcpyDeviceToHost);

    // 验证结果
    for (int i = 0; i < output_size; ++i)
    {
        REQUIRE(h_output[i] == Catch::Approx(h_expected_output[i]).epsilon(0.01f));
    }

    // 释放内存
    delete[] h_input;
    delete[] h_grid;
    delete[] h_output;
    delete[] h_expected_output;
    cudaFree(d_input);
    cudaFree(d_grid);
    cudaFree(d_output);
}
TEST_CASE("grid_sample zoom 测试", "[grid_sample]")
{
    float h_input[] = {
        0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0};
    float h_grid[]            = {-1.0,
                                 -1.0,
                                 -0.7142857313156128,
                                 -1.0,
                                 -0.4285714030265808,
                                 -1.0,
                                 -0.14285707473754883,
                                 -1.0,
                                 0.14285707473754883,
                                 -1.0,
                                 0.4285714030265808,
                                 -1.0,
                                 0.7142857313156128,
                                 -1.0,
                                 1.0,
                                 -1.0,
                                 -1.0,
                                 -0.7142857313156128,
                                 -0.7142857313156128,
                                 -0.7142857313156128,
                                 -0.4285714030265808,
                                 -0.7142857313156128,
                                 -0.14285707473754883,
                                 -0.7142857313156128,
                                 0.14285707473754883,
                                 -0.7142857313156128,
                                 0.4285714030265808,
                                 -0.7142857313156128,
                                 0.7142857313156128,
                                 -0.7142857313156128,
                                 1.0,
                                 -0.7142857313156128,
                                 -1.0,
                                 -0.4285714030265808,
                                 -0.7142857313156128,
                                 -0.4285714030265808,
                                 -0.4285714030265808,
                                 -0.4285714030265808,
                                 -0.14285707473754883,
                                 -0.4285714030265808,
                                 0.14285707473754883,
                                 -0.4285714030265808,
                                 0.4285714030265808,
                                 -0.4285714030265808,
                                 0.7142857313156128,
                                 -0.4285714030265808,
                                 1.0,
                                 -0.4285714030265808,
                                 -1.0,
                                 -0.14285707473754883,
                                 -0.7142857313156128,
                                 -0.14285707473754883,
                                 -0.4285714030265808,
                                 -0.14285707473754883,
                                 -0.14285707473754883,
                                 -0.14285707473754883,
                                 0.14285707473754883,
                                 -0.14285707473754883,
                                 0.4285714030265808,
                                 -0.14285707473754883,
                                 0.7142857313156128,
                                 -0.14285707473754883,
                                 1.0,
                                 -0.14285707473754883,
                                 -1.0,
                                 0.14285707473754883,
                                 -0.7142857313156128,
                                 0.14285707473754883,
                                 -0.4285714030265808,
                                 0.14285707473754883,
                                 -0.14285707473754883,
                                 0.14285707473754883,
                                 0.14285707473754883,
                                 0.14285707473754883,
                                 0.4285714030265808,
                                 0.14285707473754883,
                                 0.7142857313156128,
                                 0.14285707473754883,
                                 1.0,
                                 0.14285707473754883,
                                 -1.0,
                                 0.4285714030265808,
                                 -0.7142857313156128,
                                 0.4285714030265808,
                                 -0.4285714030265808,
                                 0.4285714030265808,
                                 -0.14285707473754883,
                                 0.4285714030265808,
                                 0.14285707473754883,
                                 0.4285714030265808,
                                 0.4285714030265808,
                                 0.4285714030265808,
                                 0.7142857313156128,
                                 0.4285714030265808,
                                 1.0,
                                 0.4285714030265808,
                                 -1.0,
                                 0.7142857313156128,
                                 -0.7142857313156128,
                                 0.7142857313156128,
                                 -0.4285714030265808,
                                 0.7142857313156128,
                                 -0.14285707473754883,
                                 0.7142857313156128,
                                 0.14285707473754883,
                                 0.7142857313156128,
                                 0.4285714030265808,
                                 0.7142857313156128,
                                 0.7142857313156128,
                                 0.7142857313156128,
                                 1.0,
                                 0.7142857313156128,
                                 -1.0,
                                 1.0,
                                 -0.7142857313156128,
                                 1.0,
                                 -0.4285714030265808,
                                 1.0,
                                 -0.14285707473754883,
                                 1.0,
                                 0.14285707473754883,
                                 1.0,
                                 0.4285714030265808,
                                 1.0,
                                 0.7142857313156128,
                                 1.0,
                                 1.0,
                                 1.0};
    float h_expected_output[] = {0.0,
                                 0.4285714030265808,
                                 0.8571429252624512,
                                 1.2857143878936768,
                                 1.7142856121063232,
                                 2.142857074737549,
                                 2.5714285373687744,
                                 3.0,
                                 1.7142856121063232,
                                 2.142857074737549,
                                 2.5714285373687744,
                                 3.0,
                                 3.4285712242126465,
                                 3.857142448425293,
                                 4.285714149475098,
                                 4.714285850524902,
                                 3.4285717010498047,
                                 3.8571431636810303,
                                 4.285714626312256,
                                 4.714285850524902,
                                 5.142857074737549,
                                 5.5714287757873535,
                                 6.0,
                                 6.428571701049805,
                                 5.142857551574707,
                                 5.571429252624512,
                                 6.000000476837158,
                                 6.428571701049805,
                                 6.857143402099609,
                                 7.285714149475098,
                                 7.7142863273620605,
                                 8.142857551574707,
                                 6.857142448425293,
                                 7.285714149475098,
                                 7.714284896850586,
                                 8.14285659790039,
                                 8.571428298950195,
                                 8.999999046325684,
                                 9.428571701049805,
                                 9.857142448425293,
                                 8.571428298950195,
                                 8.999999046325684,
                                 9.428571701049805,
                                 9.857141494750977,
                                 10.285713195800781,
                                 10.714284896850586,
                                 11.142857551574707,
                                 11.571428298950195,
                                 10.285714149475098,
                                 10.714285850524902,
                                 11.14285659790039,
                                 11.571428298950195,
                                 12.0,
                                 12.428571701049805,
                                 12.85714340209961,
                                 13.285714149475098,
                                 12.0,
                                 12.428571701049805,
                                 12.85714340209961,
                                 13.285715103149414,
                                 13.714284896850586,
                                 14.14285659790039,
                                 14.571428298950195,
                                 15.0};

    // 设置输入参数
    const int N = 1, C = 1, H_in = 4, W_in = 4, H_out = 8, W_out = 8;
    const int input_size  = N * C * H_in * W_in;
    const int grid_size   = N * H_out * W_out * 2;
    const int output_size = N * C * H_out * W_out;
    float*    h_output    = new float[output_size];

    REQUIRE(sizeof(h_input) == input_size * sizeof(float));

    // 分配设备内存
    float *d_input, *d_grid, *d_output;
    cudaMalloc(&d_input, input_size * sizeof(float));
    cudaMalloc(&d_grid, grid_size * sizeof(float));
    cudaMalloc(&d_output, output_size * sizeof(float));

    // 将数据复制到设备
    cudaMemcpy(d_input, h_input, input_size * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_grid, h_grid, grid_size * sizeof(float), cudaMemcpyHostToDevice);

    // 调用grid_sample函数
    int error = grid_sample(d_input, d_grid, d_output, N, C, H_in, W_in, H_out, W_out);

    // 检查是否有CUDA错误
    REQUIRE(error == cudaSuccess);

    // 将结果复制回主机
    cudaMemcpy(h_output, d_output, output_size * sizeof(float), cudaMemcpyDeviceToHost);

    // 验证结果
    for (int i = 0; i < output_size; ++i)
    {
        REQUIRE(h_output[i] == Catch::Approx(h_expected_output[i]).epsilon(0.01f));
    }

    // 释放内存
    delete[] h_output;
    cudaFree(d_input);
    cudaFree(d_grid);
    cudaFree(d_output);
}
TEST_CASE("Test broadcast_add function", "[cuda]")
{
    std::vector<float> h_coords = {0.31507110595703125,
                                   0.8340060710906982,
                                   -1.06624174118042,
                                   -0.2685492932796478,
                                   -0.38019227981567383,
                                   1.1630624532699585,
                                   -0.6126071214675903,
                                   -0.062394484877586365};

    std::vector<float> h_delta = {0.29610395431518555,
                                  1.4841848611831665,
                                  -0.32678917050361633,
                                  1.1189029216766357,
                                  0.5077429413795471,
                                  0.21504785120487213,
                                  -1.1452820301055908,
                                  1.295804738998413,
                                  0.9516400098800659,
                                  0.7790578007698059,
                                  1.059418797492981,
                                  0.706134021282196,
                                  1.3014522790908813,
                                  -0.15326550602912903,
                                  1.7090282440185547,
                                  0.012932383455336094,
                                  -1.7056523561477661,
                                  -0.6191113591194153};

    std::vector<float> h_expected_output = {
        0.6111750602722168,    2.3181910514831543,    -0.011718064546585083, 1.952908992767334,
        0.8228140473365784,    1.0490539073944092,    -0.8302109241485596,   2.1298108100891113,
        1.2667111158370972,    1.6130638122558594,    1.3744899034500122,    1.540140151977539,
        1.6165233850479126,    0.6807405948638916,    2.024099349975586,     0.84693843126297,
        -1.3905812501907349,   0.21489471197128296,   -0.7701377868652344,   1.2156355381011963,
        -1.3930308818817139,   0.8503535985946655,    -0.5584987998008728,   -0.053501442074775696,
        -2.2115237712860107,   1.0272554159164429,    -0.114601731300354,    0.5105085372924805,
        -0.006822943687438965, 0.4375847280025482,    0.23521053791046143,   -0.42181479930877686,
        0.6427865028381348,    -0.2556169033050537,   -2.7718939781188965,   -0.8876606225967407,
        -0.08408832550048828,  2.647247314453125,     -0.7069814205169678,   2.2819652557373047,
        0.1275506615638733,    1.3781102895736694,    -1.5254743099212646,   2.458867073059082,
        0.5714477300643921,    1.9421203136444092,    0.6792265176773071,    1.8691964149475098,
        0.9212599992752075,    1.0097969770431519,    1.3288359642028809,    1.175994873046875,
        -2.0858445167541504,   0.5439510941505432,    -0.3165031671524048,   1.421790361404419,
        -0.9393962621688843,   1.0565084218978882,    -0.10486418008804321,  0.15265336632728577,
        -1.7578891515731812,   1.2334102392196655,    0.3390328884124756,    0.7166633009910583,
        0.4468116760253906,    0.6437395215034485,    0.688845157623291,     -0.2156599909067154,
        1.0964211225509644,    -0.049462102353572845, -2.3182594776153564,   -0.6815058588981628};

    // 设置测试参数

    int N   = 4;
    int len = N * 1 * 1;
    int W = 3, H = 3;
    int delta_len = 1 * W * H;  // Assume delta is a 3x3 grid
    int iter      = 1;

    std::vector<float> h_output(N * W * H * 2);

    // Allocate device memory
    float *d_coords, *d_delta, *d_output;
    REQUIRE(cudaMalloc(&d_coords, len * 2 * sizeof(float)) == cudaSuccess);
    REQUIRE(cudaMalloc(&d_delta, delta_len * 2 * sizeof(float)) == cudaSuccess);
    REQUIRE(cudaMalloc(&d_output, N * W * H * 2 * sizeof(float)) == cudaSuccess);

    // Copy data to device
    REQUIRE(
        cudaMemcpy(d_coords, h_coords.data(), len * 2 * sizeof(float), cudaMemcpyHostToDevice) ==
        cudaSuccess);
    REQUIRE(cudaMemcpy(
                d_delta, h_delta.data(), delta_len * 2 * sizeof(float), cudaMemcpyHostToDevice) ==
            cudaSuccess);

    // Call broadcast_add function
    broadcast_add(d_coords, len, d_delta, delta_len, iter, W, H, d_output);

    // Copy result back to host
    REQUIRE(cudaMemcpy(
                h_output.data(), d_output, N * W * H * 2 * sizeof(float), cudaMemcpyDeviceToHost) ==
            cudaSuccess);

    // 验证结果
    SECTION("Check output range")
    {
        for (int i = 0; i < h_output.size(); ++i)
        {
            REQUIRE(h_output[i] == Catch::Approx(h_expected_output[i] - 1.0f).epsilon(0.01f));
        }
    }

    // Free device memory
    REQUIRE(cudaFree(d_coords) == cudaSuccess);
    REQUIRE(cudaFree(d_delta) == cudaSuccess);
    REQUIRE(cudaFree(d_output) == cudaSuccess);
}
TEST_CASE("create_delta generates correct delta values", "[cuda]")
{
    const int r              = 2;
    const int size           = 2 * r + 1;
    const int total_elements = size * size * 2;

    // 在设备上分配内存
    float* d_delta;
    cudaMalloc(&d_delta, total_elements * sizeof(float));

    // 调用create_delta函数
    cudaStream_t stream = nullptr;
    create_delta(d_delta, r, stream);

    // 将结果复制回主机
    float* h_delta = new float[total_elements];
    cudaMemcpy(h_delta, d_delta, total_elements * sizeof(float), cudaMemcpyDeviceToHost);

    // 验证结果
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int   idx         = y * size + x;
            float expected_dx = -r + x * (2.0f * r / (size - 1));
            float expected_dy = -r + y * (2.0f * r / (size - 1));

            REQUIRE(h_delta[idx * 2] == Catch::Approx(expected_dx));
            REQUIRE(h_delta[idx * 2 + 1] == Catch::Approx(expected_dy));
        }
    }

    // 清理内存
    delete[] h_delta;
    cudaFree(d_delta);
}
TEST_CASE("create_coords_grid generate coords", "[cuda]")
{
    const int w    = 4;
    const int h    = 3;
    const int size = w * h * 2;

    // 在设备上分配内存
    float* d_coords;
    cudaMalloc(&d_coords, size * sizeof(float));

    // 调用被测试的函数
    create_coords_grid(d_coords, w, h);

    // 将结果从设备复制到主机
    float* h_coords = new float[size];
    cudaMemcpy(h_coords, d_coords, size * sizeof(float), cudaMemcpyDeviceToHost);

    // 验证结果
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int idx = (y * w + x) * 2;
            REQUIRE(h_coords[idx] == x);
            REQUIRE(h_coords[idx + 1] == y);
        }
    }

    // 清理内存
    delete[] h_coords;
    cudaFree(d_coords);
}
