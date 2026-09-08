#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926; // MY_PI = acos(-1);

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    // 定义绕z轴旋转变换矩阵
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f translate;

    // 弧度制计算
    float radius = rotation_angle * MY_PI / 180.0;

    // 计算旋转变换矩阵
    translate << cos(radius), -sin(radius), 0.0f, 0.0f,
        sin(radius), cos(radius), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f;
    model = translate * model;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float z_near, float z_far)
{
    // 定义透视与投影变换矩阵
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f persp_to_ortho = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f orthoscale = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f orthopos = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f rotate = Eigen::Matrix4f::Identity();

    // 立方体投影相关变量
    float theta = eye_fov / 180 * MY_PI;
    float t = z_near * tan(theta/2) ;
    float r = t * aspect_ratio;
    float l = -r;
    float b = -t;

    // 计算投影变换矩阵
    persp_to_ortho << z_near, 0, 0, 0,
        0, z_near, 0, 0,
        0, 0, z_near + z_far, -z_near * z_far,
        0, 0, 1, 0;
    orthopos << 1, 0, 0, -(r+l)/2,
        0, 1, 0, -(t + b) /2,
        0, 0, 1, -(z_near+z_far)/2,
        0, 0, 0, 1;
    orthoscale << 2 / (r - l), 0, 0, 0,
        0, 2 / (t - b), 0, 0,
        0, 0, 2 / (z_near - z_far), 0,
        0, 0, 0, 1;
    rotate << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1;
    projection =  orthoscale * orthopos * persp_to_ortho * rotate * projection;
    
    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    Eigen::Matrix3f rotation;
    
    // 将任意轴向量转变为方向向量
    Eigen::Vector3f normal_axis = axis.normalized();

    // 将角度转变为弧度制
    float radius = angle / 180.0f * MY_PI;

    // 构建辅助矩阵N
    Eigen::Matrix3f N;
    N << 0, -normal_axis[2], normal_axis[1],
        normal_axis[2], 0, -normal_axis[0],
        -normal_axis[1], normal_axis[0], 0;

    // 建立罗德里格旋转矩阵
    Eigen::Matrix3f mat1 = cos(radius) * Eigen::Matrix3f::Identity();
    Eigen::Matrix3f mat2 = (1 - cos(radius)) * normal_axis * normal_axis.transpose();
    Eigen::Matrix3f mat3 = sin(radius) * N;
    rotation = mat1 + mat2 + mat3;

    model.block<3, 3>(0, 0) = rotation;

    return model;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    Eigen::Vector3f rotate_axis = { 1,1,0 };


    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //r.set_model(get_model_matrix(angle));
        r.set_model(get_rotation(rotate_axis, angle));

        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
