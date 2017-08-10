/*
This file is part of remove_sky.
copyright��Zhu Baohua
2016.6
*/

#include <iostream>    
#include <Eigen/Dense>   
#include <Eigen/LU>  
#include<vector>
#include<deque>
#include <cmath>
#include <highgui.h>
#include<opencv2/opencv.hpp>
#include "opencv2/core/core.hpp" 
using Eigen::MatrixXd;  
using namespace std;
using namespace cv;
using namespace Eigen;

float get_max_3(float a,float b,float c);
//
int border_avg(vector<int> outborder);
int absolute_border_dif(vector<int> outborder);
//����һ���ݶ���ֵ������ͼ�����һ�߽�
void Calculate_border(cv::Mat grad,vector<int>&outborder,int grad_threshold);
//���������Ϳ��
void get_no_sky_img( cv::Mat& SourceImg,vector<int> outborder);
//������պ͵�������ػҶȵ�ƽ��ֵ
void get_vector3d_mean(cv::Mat SourceImg,vector<int>border_tmp,Eigen::Vector3d &sky_meanRGB,Eigen::Vector3d &ground_meanRGB);
//����һ�� border ��������պ͵������ص���
void get_sky_num(cv::Mat SourceImg,vector<int>border_tmp,long int &num_sky,long int &num_ground);
//����������������
int get_sample_pointnum(int threshold_min,int  threshold_max,int search_step);
//����仯�ݶ���ֵt
int get_grad_threahold_t(int threshold_min,int  threshold_max,int sample_point_num,int k);
//ÿ�θ���һ��border����������������
 double get_Jnt(cv::Mat SourceImg, Eigen::Vector3d &sky_meanRGB,Eigen::Vector3d &ground_meanRGB,vector<int>border_tmp);
 //�õ�ȥ����յ�ͼƬ  
 // input���ݶ�������Сֵ�����ֵ���������������ĻҶ�ͼ��
 // output: ȥ����յ�ͼ��

int get_img_without_sky_opt(cv::Mat grayImg,cv::Mat &no_sky_img);
void get_img_without_sky(int threshold_min,int  threshold_max,int search_step,cv::Mat souceimg,cv::Mat &out);
//�������Ͼ���
double cal_vector3d_Mahalanobis(Eigen::Vector3d input1 ,Eigen::Vector3d input2);
//��������3ά��������ŷ�Ͼ���
double  Euclidean_distance(Eigen::Vector3d input1,Eigen::Vector3d input2);
//���Ż��������������ؾ�ֵ
Vector3d cal_Vector3d_mean(cv::Mat channel3img,cv::Mat out);
//����վ����Ϊ���࣬���ڻҶ�ͼƬ����ʾ����
void classify_sky_byK_means(vector<int> bopt ,cv::Mat& class_sky,cv::Mat channel3img);
//�������㷨�������¼�����ձ߽�
void Sky_borde_recalculation(vector<int> bopt ,cv::Mat class_sky,cv::Mat channel3img,cv::Mat&out);
void low_textrue_check(cv::Mat inputimg,cv::Mat&outputimg,int SizeN);