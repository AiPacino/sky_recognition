ʶ����յĴ���ӿ���Remove_sky.h����

get_img_without_sky_opt(cv::Mat souceimg,cv::Mat &out)



ʹ��˵�� ��
1 ������opencv 
        Eigen 


2 ���� Mat���ͣ�֧�ֵ�ͨ������ͨ��
   ���Mat���ͣ���ͨ��
  �½�һ��mainSky.cpp

#include"Remove_sky.h"
void main{
 Mat souceimg = imread("test.png",0);	
 Mat out;																					
  get_img_without_sky_opt( souceimg ,out);
  imshow(" ȥ����պ��ͼ��",out);
  cvWaitKey(0);

}
