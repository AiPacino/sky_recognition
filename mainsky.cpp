#include"Remove_sky.h"



void main()
{
 Mat souceimg = imread("E:/matchpic/12_10left.png",1);	
 Mat out;										

											
  get_img_without_sky_opt( souceimg ,out);
  imshow(" ȥ����պ��ͼ��",out);
  cvWaitKey(0);

}