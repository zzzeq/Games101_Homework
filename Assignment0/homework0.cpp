#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<iostream>

int main() {
    // 文件内容说明：
    // 1. 给定一个点P，在齐次坐标下给出平移和旋转之后的结果

    // 01 基本变量定义
    Eigen::Vector3f P(2.0f, 1.0f, 1.0f);
    float pi = acos(-1);
    float theta = 45.0 / 180.0 * pi;

    // 02 M_rotation & M_translation
    Eigen::Matrix3f M_rotation,M_translation;
    M_rotation << cos(theta), -sin(theta), 0.0f,
        sin(theta), cos(theta), 0.0f,
        0.0f, 0.0f, 1.0f;
    M_translation << 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 2.0f,
        0.0f, 0.0f, 1.0f;

   // 03 P_after_transformation -> 

    Eigen::Vector3f P_prime = M_translation * M_rotation * P;

    std::cout << "P_after_transformation: \n ";
    std::cout << P_prime << std::endl;

    return 0;
}