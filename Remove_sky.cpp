/*
This file is part of remove_sky.
copyright��Zhu Baohua
2016.6
*/
#include"Remove_sky.h"
#include"myKmeans.h"
static bool check;

static deque<double> lasts_sky_pixel_num;//������̬�洢��ǰ֮֡ǰʮ֡������������ظ���
static deque<Vector3d> lasts_sky_region_mean;////������̬�洢��ǰ֮֡ǰʮ֡���������ľ�ֵ
const int  threshold_min=5;
const int  threshold_max=100;
const int search_step=5;

const int lamada=2;
int thresh_avg_min;
int thresh_avg_large;
int thresh_absolute_border_dif;
int thresh4;
int H,W;
vector<int> bopt;
vector<int> bopt_opt;
vector<int> bfinal;
double Jmax_opt=0;
double Jmax=0;
int gradd=0;
void get_threshold(cv::Mat SourceImg)
{   
	H=SourceImg.rows;
	W=SourceImg.cols;
	thresh_avg_min=H/20;
	thresh_avg_large=H/10;
	thresh_absolute_border_dif=15;
	thresh4=H/3;
}
double get_original_skypixel_num(vector<int> outborder)
{    
	 double tmp=0;
	for(int i=0;i<outborder.size();i++)
	{
		tmp+=outborder[i];
	}
	return tmp;
}
float get_max_3(float a,float b,float c)
{
	float tmp=a>b?a:b;
	float max=tmp>c?tmp:c;
	return c;
}
int border_avg(vector<int> outborder)
{
	int tmp=0;
	int num=outborder.size();
	for(int i=0;i<num;i++)
	{
		tmp+=outborder[i];
	}
	return tmp/num;
}
//void diff_abs(vector<int> outborder,int absdiff)
void diff_abs(vector<int> outborder)
{

	
	int num=outborder.size();
	for(int i=1;i<num;i++)
	{
		if(abs(outborder[i]-outborder[i-1])>thresh4)///////////////////////////////////
		{
		check=true;break;
		}
			
	}
	//return tmp/num;


}
int absolute_border_dif(vector<int> outborder)
{

	int tmp=0;
	int num=outborder.size();
	for(int i=1;i<num;i++)
	{
		tmp+=abs(outborder[i]-outborder[i-1]);
	}
	return tmp/num;


}
void Calculate_border(cv::Mat grad,vector<int>&outborder,int grad_threshold)
{
	int width=grad.cols;
	int height=grad.rows;
	
	for(int x=0;x<width;x++)
		for(int y=0;y<height;y++)
		{
		           if( grad.at<uchar>(y,x)>grad_threshold)
				   {
				  outborder.push_back(y);					      	
					      break;
				   }
				   else if(y==height-1)
					 outborder.push_back(0);

		}	
}
void get_no_sky_img( cv::Mat& SourceImg,vector<int> outborder)
{
	int width=SourceImg.cols;
	int height=SourceImg.rows;

	for(int col=0;col<outborder.size();col++)
	{
		for(int row=0;row<outborder[col];row++)

	         SourceImg.at<uchar>(row,col)=0;

	}
}
int get_sample_pointnum(int threshold_min,int  threshold_max,int search_step)
{
	return (threshold_max-threshold_min)/search_step+1;
}

void get_vector3d_mean(cv::Mat SourceImg,vector<int>border_tmp,Eigen::Vector3d &sky_meanRGB,Eigen::Vector3d &ground_meanRGB)
{
	       
	      int width=SourceImg.cols;
          int height=SourceImg.rows;
		
		   Eigen::Vector3d mean_sky;
		    Eigen::Vector3d mean_ground;
			long int sky_count=0;
			long int ground_count=0;
		for(int col=0;col<width;col++)
		  for(int row=0;row<border_tmp[col];row++)
		  {
		    mean_sky(0)+= SourceImg.at<Vec3b>(row,col)[0];//B
		    mean_sky(1)+=SourceImg.at<Vec3b>(row,col)[1];//G
			mean_sky(2)+= SourceImg.at<Vec3b>(row,col)[2];//R
			sky_count++;
		  }
		mean_sky/=sky_count;

		for(int col=0;col<width;col++)
		  for(int row=border_tmp[col];row<height;row++)
		  {
		    mean_ground(0)+= SourceImg.at<Vec3b>(row,col)[0];//B
		    mean_ground(1)+=SourceImg.at<Vec3b>(row,col)[1];//G
			mean_ground(2)+= SourceImg.at<Vec3b>(row,col)[2];//R
			ground_count++;
		  }
		mean_ground/=ground_count;

		sky_meanRGB=mean_sky;
         ground_meanRGB=mean_ground;
//cout<<"mean_sky   "<<endl<<mean_sky<<endl;
//cout<<"ground_meanRGB   "<<endl<<ground_meanRGB<<endl;
}

void get_sky_num(cv::Mat SourceImg,vector<int>border_tmp,long int &num_sky,long int &num_ground)
{
	 int width=SourceImg.cols;
        int height=SourceImg.rows;
		for(int i=0;i<border_tmp.size();i++)
		{
			num_sky+=border_tmp[i];
		}
       num_ground=width*height-num_sky;

}

 int get_grad_threahold_t(int threshold_min,int  threshold_max,int sample_point_num,int k)
{
	int search_step_tmp=(threshold_max-threshold_min)/(sample_point_num-1);
	return  threshold_min+search_step_tmp*(k-1);
	//return  threshold_min+5*k;
}

 double cal_variance(vector<int> COL_label)
 {
	 double mean_col=0;
	 for(int i=0;i<COL_label.size();i++)
	 {
	   mean_col+=COL_label[i];
	 }
	 mean_col/=COL_label.size();
	 double variance;
	  for(int i=0;i<COL_label.size();i++)
	 {
	   variance+=(COL_label[i]-mean_col)*(COL_label[i]-mean_col);
	 }
	 variance/=COL_label.size();
 return variance;
 }
double get_Jnt(cv::Mat SourceImg, Eigen::Vector3d &sky_meanRGB,Eigen::Vector3d &ground_meanRGB,vector<int>border_tmp)
{
	long int sky_num=0;
	long int ground_num=0;
	 int width=SourceImg.cols;
     int height=SourceImg.rows;
	 Eigen::Matrix3d sky_covariance_matrix;
	 Eigen::Matrix3d ground_covariance_matrix;
	 sky_covariance_matrix<<0,0,0,
		                    0,0,0,
							0,0,0;
  ground_covariance_matrix<<0,0,0,
		                    0,0,0,
							0,0,0;

	
         for(int col=0;col<width;col++)
		  for(int row=0;row<border_tmp[col];row++)
		  {
         Eigen::Vector3d pixelcolor_tmp;
		 pixelcolor_tmp(0)=SourceImg.at<Vec3b>(row,col)[0];
		  pixelcolor_tmp(1)=SourceImg.at<Vec3b>(row,col)[1];
		   pixelcolor_tmp(2)=SourceImg.at<Vec3b>(row,col)[2];
		  Eigen::Vector3d tmp;
		 tmp= pixelcolor_tmp-sky_meanRGB;
		   sky_covariance_matrix+=tmp*tmp.transpose();
		   	 sky_num++;
		  }
		   // cout<<"sky_num : "<<sky_num<<endl;
		  sky_covariance_matrix=sky_covariance_matrix/sky_num;
///////////////////////////////////////////////////////////////////////ground
		   for(int col=0;col<width;col++)
		  for(int row=border_tmp[col];row<height;row++)
		  {
         Eigen::Vector3d pixelcolor_tmp;
		 pixelcolor_tmp(0)=SourceImg.at<Vec3b>(row,col)[0];
		  pixelcolor_tmp(1)=SourceImg.at<Vec3b>(row,col)[1];
		   pixelcolor_tmp(2)=SourceImg.at<Vec3b>(row,col)[2];
		  Eigen::Vector3d tmp;
		 tmp= pixelcolor_tmp-ground_meanRGB;
																					// cout<<"ground_tmp:  "<<endl<<tmp<<endl;
																					// cout<<"ground_tmp.transpose(): "<<endl<<tmp.transpose()<<endl;
																					// cout<<"tmp*tmp.transpose() "<<endl<<tmp*tmp.transpose()<<endl;
		   ground_covariance_matrix+=tmp*tmp.transpose();
		   	 ground_num++;
		  }
		  ground_covariance_matrix=ground_covariance_matrix/ground_num;
																							
																				//	  cout<<"sky_covariance_matrix"<<endl<<sky_covariance_matrix<<endl;
																				//	cout<<"ground_covariance_matrix"<<endl<<ground_covariance_matrix<<endl;
																							 
																							
/////////////////����Э������������ֵ
		  SelfAdjointEigenSolver<Matrix3d> eigensolver_sky(sky_covariance_matrix);
	double sky_eigen_value1=eigensolver_sky.eigenvalues()(0);
	double sky_eigen_value2=eigensolver_sky.eigenvalues()(1);
	double sky_eigen_value3=eigensolver_sky.eigenvalues()(2);
																		/*cout<<"����ֵ "<<endl<<"sky_eigen_value1 :"<<sky_eigen_value1<<endl
																			<<"sky_eigen_value2  :"<<sky_eigen_value2<<endl
																			<<"sky_eigen_value3  :"<<sky_eigen_value3<<endl;*/

		  SelfAdjointEigenSolver<Matrix3d> eigensolver_ground(ground_covariance_matrix);
	double ground_eigen_value1=eigensolver_ground.eigenvalues()(0);
	double ground_eigen_value2=eigensolver_ground.eigenvalues()(1);
	double ground_eigen_value3=eigensolver_ground.eigenvalues()(2);
																			//cout<<"����ֵ "<<endl<<"ground_eigen_value1  :"<<ground_eigen_value1<<endl
																			//	<<"ground_eigen_value2  :"<<ground_eigen_value2<<endl
																			//	<<"ground_eigen_value3  :"<<ground_eigen_value3<<endl;
	//�õ�Э�����������ֵ�����ֵ
	double sky_eigen_value_max=get_max_3(sky_eigen_value1,sky_eigen_value2,sky_eigen_value3);
	double ground_eigen_value_max=get_max_3(ground_eigen_value1,ground_eigen_value2,ground_eigen_value3);
	                                                                       // cout<<"sky_eigen_value_max  :"<<sky_eigen_value_max<<endl;
																			//cout<<"ground_eigen_value_max  :"<<ground_eigen_value_max<<endl;
	///////////////////////����Э������������ʽֵ
	double sky_det=sky_covariance_matrix.determinant();
	double ground_det=ground_covariance_matrix.determinant();
																				//cout<<"sky_det  :"<<sky_det<<endl;
																				//cout<<"ground_det  :"<<ground_det<<endl;
	///////////////////������������
	double Jnt=1/(lamada*sky_det+ground_det+lamada*abs(sky_eigen_value_max)+abs(ground_eigen_value_max));
	                                                                    //             	cout<<"Jtmp :"<<Jnt<<endl;
	                                                                  //cout<<"-----------------------------------------------------"<<endl;
	return  Jnt;


}


int get_img_without_sky_opt(cv::Mat souceimg,cv::Mat &out)
{
	Jmax_opt=0;
	 gradd=0;
	  Mat dst_x, dst_y, grad,gray;
	   if(souceimg.channels()==3)  	

		{	 cvtColor(souceimg,gray,CV_RGB2GRAY);	
				Sobel(gray, dst_x, souceimg.depth(), 1, 0);
				Sobel(gray, dst_y, souceimg.depth(), 0, 1);
				convertScaleAbs(dst_x, dst_x);
				convertScaleAbs(dst_y, dst_y);
				addWeighted(dst_x, 0.5, dst_y, 0.5, 0, grad);
	   }
	   else
	   {
	          Sobel(souceimg, dst_x, souceimg.depth(), 1, 0);
				Sobel(souceimg, dst_y, souceimg.depth(), 0, 1);
				convertScaleAbs(dst_x, dst_x);
				convertScaleAbs(dst_y, dst_y);
				addWeighted(dst_x, 0.5, dst_y, 0.5, 0, grad);
	   
	   }
		Mat channel3img;

		if(souceimg.channels()==1)
		{
		    cvtColor(souceimg,channel3img,CV_GRAY2RGB);
			souceimg.copyTo(out);   
		}
		else
		{
			souceimg.copyTo(channel3img);    
			cvtColor(souceimg,out,CV_RGB2GRAY);
		}                              
		                                                                                       /* imshow("  out  :",out);
																								cvWaitKey(10);*/
		int removefirst=0;
	int n=get_sample_pointnum(threshold_min,threshold_max,search_step);
	for(int k=1;k<=n;k++)
	{
		int grad_threahold_t=0;
	grad_threahold_t= get_grad_threahold_t(threshold_min,threshold_max, n, k);
	vector<int> border_tmp;
	Calculate_border( grad,border_tmp,grad_threahold_t);
	
	Eigen::Vector3d sky_meanRGB;
	Eigen::Vector3d ground_meanRGB;
	get_vector3d_mean( channel3img,border_tmp,sky_meanRGB,ground_meanRGB);
	double Jtmp=get_Jnt( channel3img, sky_meanRGB,ground_meanRGB,border_tmp);
	if(removefirst<4)
			{
				removefirst++;continue;
			}
	if(Jtmp>0)
	  {
	//cout<<"Jtmp  :  "<<Jtmp<<endl;/////////////////////////////////////////////////////////////////////////////////////////////////
	//cout<<"Jmax  :  "<<Jmax_opt<<endl;
     	if( Jtmp>Jmax_opt)
			{
				//cout<<"gggggggggggggggg"<<endl;
				Jmax_opt=Jtmp;
				 bopt_opt=border_tmp;
				 gradd=grad_threahold_t;
			}
		else
		{
			break;
	    }
    	}
	}
	// get_no_sky_img(out,bopt_opt);
	 // Detection of the Image without a Sky Region
	   int border_Avg=border_avg(bopt_opt);
	    int abs_border_dif= absolute_border_dif(bopt_opt);
		//cout<<"abs_border_dif"<<abs_border_dif<<endl;
		get_threshold(souceimg);
		/*if(border_Avg<thresh_avg_min||(border_Avg<thresh_avg_large)&&abs_border_dif>thresh_absolute_border_dif)
		{
			vector<int> border_zeros;
			for(int i=0;i<bopt_opt.size();i++)
			{
				border_zeros.push_back(0);
			}
			bopt_opt=border_zeros;
			return 0;
		}*/


		diff_abs(bopt_opt);//��������17  �����㣬check��Ϊtrue
		
		 if(1)
		 {
			 cout<<" check is true "<<endl;
			 Mat class_sky;
			classify_sky_byK_means( bopt_opt , class_sky, channel3img);
			Sky_borde_recalculation(bopt_opt ,class_sky,channel3img,out);
			//get_no_sky_img(out,bfinal);//�õ����ı߽磬��ʾ
		 }
		 else
		 {
		   get_no_sky_img(out,bopt_opt);//�õ����ı߽磬��ʾ
		 }
		 return 1;
}
void get_img_without_sky(int threshold_min,int  threshold_max,int search_step,cv::Mat souceimg,cv::Mat &out)
{
	Jmax=0;
    gradd=0;
	    Mat dst_x, dst_y, grad,gray; 
		 	 if(souceimg.channels()==3)  	

			{	 
				cvtColor(souceimg,gray,CV_RGB2GRAY);	  
				Sobel(gray, dst_x, souceimg.depth(), 1, 0);
				Sobel(gray, dst_y, souceimg.depth(), 0, 1);
				convertScaleAbs(dst_x, dst_x);
				convertScaleAbs(dst_y, dst_y);
				addWeighted(dst_x, 0.5, dst_y, 0.5, 0, grad);
			 }
		 else
	       {
				Sobel(souceimg, dst_x, souceimg.depth(), 1, 0);
				Sobel(souceimg, dst_y, souceimg.depth(), 0, 1);
				convertScaleAbs(dst_x, dst_x);
				convertScaleAbs(dst_y, dst_y);
				addWeighted(dst_x, 0.5, dst_y, 0.5, 0, grad);
			 }
		Mat channel3img;
		if(souceimg.channels()==1)
		{
		    cvtColor(souceimg,channel3img,CV_GRAY2RGB);
			souceimg.copyTo(out);   
		}
		else
		{
			souceimg.copyTo(channel3img);    
			cvtColor(souceimg,out,CV_RGB2GRAY);
		}                                                                
		                                                                                                   //   imshow("no_sky_img ",no_sky_img);
																					 
																											//				  cvWaitKey(500);
	int n=get_sample_pointnum(threshold_min,threshold_max,search_step);                                   //cout<<  "get_sample_pointnum : "<<n<<endl;
	int removefirst=0;
	for(int k=1;k<=n;k++)
	{
		int grad_threahold_t=0;
	grad_threahold_t= get_grad_threahold_t(threshold_min,threshold_max, n, k);                           cout<<"------------- grad_threahold_t: "<< grad_threahold_t<<endl;
	vector<int> border_tmp;
	Calculate_border( grad,border_tmp,grad_threahold_t);
																													/*	     cv::Mat test;
																										
																													 if(souceimg.channels()==3)
																				
																													 	cvtColor(souceimg,test,CV_RGB2GRAY);
																													 else
																															   souceimg.copyTo(test);
																															  get_no_sky_img(test,border_tmp);
																															  imshow("test ",test);
																					 
																															  cvWaitKey(500);*/
	Eigen::Vector3d sky_meanRGB;
	Eigen::Vector3d ground_meanRGB;
	get_vector3d_mean( channel3img,border_tmp,sky_meanRGB,ground_meanRGB);
	double Jtmp=get_Jnt( channel3img, sky_meanRGB,ground_meanRGB,border_tmp);
	

	cout<<"Jtmp  :  "<<Jtmp<<endl;/////////////////////////////////////////////////////////////////////////////////////////////////
	cout<<"Jmax  :  "<<Jmax<<endl;
	
			if(removefirst<4)//�����Ǹտ�ʼɨ��� ���� 5 10 15
			{
				removefirst++;continue;
			}
     	if( Jtmp>Jmax)
			{
				cout<<"gggggggggggggggg"<<endl;
				Jmax=Jtmp;
				// bopt=border_tmp;
				 bopt=border_tmp;  
				
				 gradd=grad_threahold_t;
				
			}
		else
		    {
		             break;
		    }
			
	    }
		
	

	                                                                       cout<<"ɨ�����õ��ݶȣ� "<<gradd<<endl;
		get_no_sky_img(out,bopt);//�õ����ı߽磬��ʾ
}
double  Euclidean_distance(Eigen::Vector3d input1,Eigen::Vector3d input2)
{
	Eigen::Vector3d tmp;
	tmp(0)=(input1[0]-input2[0])*(input1[0]-input2[0]);
	tmp(1)=(input1[1]-input2[1])*(input1[1]-input2[1]);
	tmp(2)=(input1[2]-input2[2])*(input1[2]-input2[2]);
	return sqrt(tmp(0)+tmp(1)+tmp(2));
}
double cal_vector3d_Mahalanobis(Eigen::Vector3d input1 ,Eigen::Vector3d input2)
	{
		//�������õ�ŷ�Ͼ���
	  Eigen::Vector3d mean_=(input1+input2)/2;	
	  Eigen::Vector3d  tmp1= input1-mean_;
	  Eigen::Vector3d  tmp2= input2-mean_;
	  Eigen::Matrix3d covariance_matrix=tmp1*tmp1.transpose()+tmp2*tmp2.transpose();
	  covariance_matrix/=2;
	  double covariance_matrix_det=covariance_matrix.determinant();
	   Eigen::Matrix3d covariance_matrix_inverse;
	  if (covariance_matrix_det==0)//covariance_matrix��Ϊ������������������Ϊ�Խ���Ԫ�ص���
	  {
		  double tmp_00=1/covariance_matrix(0,0);
		  double tmp_11=1/covariance_matrix(0,0);
		  double tmp_22=1/covariance_matrix(0,0);
	  covariance_matrix_inverse<<1/tmp_00,0,0,
		                        0,1/tmp_11,0,
								 0,0,1/tmp_22;
	  }
	  else
	  covariance_matrix_inverse=covariance_matrix.inverse();    
	  


																				cout<<endl<<"covariance_matrix : "<<endl<<covariance_matrix<<endl;			
																				cout<<endl<<"covariance_matrix_inverse : "<<endl<<covariance_matrix_inverse<<endl;
	  return input1.transpose()*covariance_matrix_inverse*input2;
	}


void classify_sky_byK_means(vector<int> bopt ,cv::Mat& class_sky,cv::Mat channel3img)
{
	 int height=channel3img.rows;
     int width=channel3img.cols;
	 int clusterCount=2;//���þ������
	 double totle_sky_num=get_original_skypixel_num(bopt);
	 cout<<"totle_pixel: "<<height*width<<endl;
	 cout<<"totle_sky_num: "<<totle_sky_num<<endl;
	Mat K_means_data(totle_sky_num,1,CV_32FC3);//����k��ֵ������������
	Mat labels(totle_sky_num,1,CV_8UC1);//��������������ǩ
	int data_Row=0;
   for(int col=0;col<width;col++)   																													
		 for(int row=0;row<bopt[col];row++)
		 {	//cout<<"channel3img.at<Vec3b>(row,col)"<<channel3img.at<Vec3b>(row,col)<<endl;

			 K_means_data.at<Vec3b>(data_Row,0)[0]=(float)channel3img.at<Vec3b>(row,col)[0];	
			 K_means_data.at<Vec3b>(data_Row,0)[1]=(float)channel3img.at<Vec3b>(row,col)[1];
			 K_means_data.at<Vec3b>(data_Row,0)[2]=(float)channel3img.at<Vec3b>(row,col)[2];
					//	 cout<<"K_means_data  : "<< K_means_data.at<Vec3b>(data_Row,0)<<endl;
			 data_Row++;	
		 }

     //����
	// kmeans(K_means_data, clusterCount, labels, TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0), 3, KMEANS_PP_CENTERS);  
		  Vector3d centers[2];
		 myKMeans(K_means_data,labels, centers);
	 data_Row=0;
																											//	cout<<"K_means_data "<<K_means_data<<endl;
																												 cout<<"labels.size() : "<<labels.size()<<endl;
																													 cout<<"K_means_data.size() : "<<K_means_data.size()<<endl;
																										//	cout<<"Labels: "<<": "<<labels<<endl;  
																												// int oo=labels.rows;
																												// for(int jj=0;jj<oo;jj++)
																												// {
																												//	 //if (labels.at<float>(0,jj)==1)
																												// cout<<"Labels: \n"<<jj<<": "<<(int)labels.at<uchar>(jj,0)<<endl;
																											// }
																												// system("pause");
 cvtColor(channel3img,class_sky,CV_RGB2GRAY);
																									//	 imshow("class_sky",class_sky);
																										// cvWaitKey(10);
																										 cout<<"class_sky.channels : "<<class_sky.channels()<<endl;
    for(int col=0;col<width;col++)    
	  {
	  for(int row=0;row<bopt[col];row++)
	   {	
		   //�Ѿ����Ľ������ڻҶ�ͼƬ��
		 class_sky.at<uchar>(row,col)=255*labels.at<uchar>(data_Row,0);
		 // class_sky.at<float>(row,col)=255*labels.at<uchar>(data_Row,0);//������float�Ͳ���
		//   imshow("",class_sky);
		//    cvWaitKey(10);
		 data_Row++;
	   }
	 }
	 cout<<"data_Row :  "<<data_Row<<endl;
	 imshow("class_sky",class_sky);
	imwrite("class_sky.png",class_sky);
		 cvWaitKey(10);
}


Vector3d cal_Vector3d_mean(cv::Mat channel3img,cv::Mat out)
{   
	Vector3d means; means<<0,0,0;
	double cnt=0;
	int height=channel3img.rows;
	int width=channel3img.cols;
	for(int row=0;row<height;row++)
      for(int col=0;col<width;col++)
	  {
		  if(out.at<uchar>(row,col)==0)
		  {
			        ++cnt;
		            means(0)+=channel3img.at<Vec3b>(row,col)(0);
					 means(1)+=channel3img.at<Vec3b>(row,col)(1);
					  means(2)+=channel3img.at<Vec3b>(row,col)(2);
		  }

	  }
	  means(0)/=cnt;
	  means(1)/=cnt;
	  means(2)/=cnt;
	  return means;
}
void low_textrue_check(cv::Mat inputimg,cv::Mat&outputimg,int SizeN)
{
	inputimg.copyTo(outputimg);
	int height=inputimg.rows;
	int width=inputimg.cols;
	for(int row=SizeN/2;row<height-SizeN/2;row++)
      for(int col=SizeN/2;col<width-SizeN/2;col++)
	  {
											if(inputimg.at<uchar>(row,col)==0)
													continue;
		      float tmp=0;
			  for(int i=-SizeN/2;i<=SizeN/2;i++)
				for(int j=-SizeN/2;j<=SizeN/2;j++)
				{
		
				tmp+=abs(inputimg.at<uchar>(row+i,col+j)-inputimg.at<uchar>(row,col));
				}
			   tmp/=SizeN*SizeN;
			 if(tmp<2.5)
			 {
                outputimg.at<uchar>(row,col)=0;
			 }
			  
	  }
	//  medianBlur(outputimg,outputimg,3);
}
void Sky_borde_recalculation(vector<int> bopt ,cv::Mat class_sky,cv::Mat channel3img,cv::Mat&out)
{
Eigen::Vector3d sky1_u_mean;sky1_u_mean<<0,0,0;//��һ����������������صľ�ֵ
int sky1_count=0;
Eigen::Vector3d sky2_u_mean;sky2_u_mean<<0,0,0;//�ڶ�����������������صľ�ֵ
int sky2_count=0;
Eigen::Vector3d ground_u_mean;ground_u_mean<<0,0,0;//����
int ground_count=0;

int height=channel3img.rows;
int width=channel3img.cols;



vector<int> sky1_col_label;
vector<int> sky2_col_label;
double sky1_col_variance;
double sky2_col_variance;
      for(int col=0;col<width;col++)    
	 for(int row=0;row<bopt[col];row++)
	 {
	          if(class_sky.at<uchar>(row,col)==0)
			  {   sky1_col_label.push_back(row);//��sky1�кŴ�����
			      sky1_u_mean(0)+=channel3img.at<Vec3b>(row,col)[0];
			      sky1_u_mean(1)+=channel3img.at<Vec3b>(row,col)[1];
				  sky1_u_mean(2)+=channel3img.at<Vec3b>(row,col)[2];
				  sky1_count++;			    
			  }
			  else if(class_sky.at<uchar>(row,col)==255)
			  {   
				  sky2_col_label.push_back(row);//��sky2�кŴ�����
			      sky2_u_mean(0)+=channel3img.at<Vec3b>(row,col)[0];
			      sky2_u_mean(1)+=channel3img.at<Vec3b>(row,col)[1];
				  sky2_u_mean(2)+=channel3img.at<Vec3b>(row,col)[2];
				  sky2_count++;			  			  
			  }
	 }
		sky1_col_variance=cal_variance(sky1_col_label);
		sky2_col_variance=cal_variance(sky2_col_label);	
																								cout<<"sky1_col_variance : "<<sky1_col_variance<<endl;
																								cout<<"sky2_col_variance : "<<sky2_col_variance<<endl;
	  for(int col=0;col<width;col++)    
	 for(int row=bopt[col];row<height;row++)
	 {
	         
			      ground_u_mean(0)+=channel3img.at<Vec3b>(row,col)[0];
			      ground_u_mean(1)+=channel3img.at<Vec3b>(row,col)[1];
				  ground_u_mean(2)+=channel3img.at<Vec3b>(row,col)[2];
				  ground_count++;			  			  
			
	 }
	 sky1_u_mean/=sky1_count;
	 sky2_u_mean/=sky2_count;
	 ground_u_mean/=ground_count;
																								cout<<"sky1_u_mean :  "<<sky1_u_mean<<endl;
																								cout<<"sky2_u_mean :  "<<sky2_u_mean<<endl;
																								cout<<"ground_u_mean :  "<<ground_u_mean<<endl;	
	
         for(int col=0;col<width;col++)
		  for(int row=0;row<bopt[col];row++)
		  {
			   if(class_sky.at<uchar>(row,col)==0)
			  {
				  Eigen::Vector3d pixelcolor_tmp;
				  pixelcolor_tmp(0)=channel3img.at<Vec3b>(row,col)[0];
				  pixelcolor_tmp(1)=channel3img.at<Vec3b>(row,col)[1];
				   pixelcolor_tmp(2)=channel3img.at<Vec3b>(row,col)[2];
				  Eigen::Vector3d tmp;
				  tmp= pixelcolor_tmp-sky1_u_mean;
				
				 
			   }
			    
			    else if(class_sky.at<uchar>(row,col)==255)
			  {
		          
				  Eigen::Vector3d pixelcolor_tmp;
				  pixelcolor_tmp(0)=channel3img.at<Vec3b>(row,col)[0];
				  pixelcolor_tmp(1)=channel3img.at<Vec3b>(row,col)[1];
				   pixelcolor_tmp(2)=channel3img.at<Vec3b>(row,col)[2];
				  Eigen::Vector3d tmp;
				  tmp= pixelcolor_tmp-sky2_u_mean;
				
				 
		      }
		  }
				
///////////////////////////////////////////////////////////////////////ground
		   for(int col=0;col<width;col++)
		  for(int row=bopt[col];row<height;row++)
		  {
                 Eigen::Vector3d pixelcolor_tmp;
				  pixelcolor_tmp(0)=channel3img.at<Vec3b>(row,col)[0];
				  pixelcolor_tmp(1)=channel3img.at<Vec3b>(row,col)[1];
				   pixelcolor_tmp(2)=channel3img.at<Vec3b>(row,col)[2];
				  Eigen::Vector3d tmp;
				  tmp= pixelcolor_tmp-ground_u_mean;
				
				
		  }
		
		                                                                            cout<<"sky1_count :  "<<sky1_count<<endl;
																					cout<<"sky2_count :  "<<sky2_count<<endl;
																				//	cout<<"ground_count :  "<<ground_count<<endl;
		                                                                        
																				
			 cvtColor(channel3img,out,CV_RGB2GRAY);																
           //����ŷʽ���룬�õ�sky_true��
		 double sky1_ground_M= Euclidean_distance(sky1_u_mean ,ground_u_mean);
		 double sky2_ground_M= Euclidean_distance(sky2_u_mean ,ground_u_mean);
	//////////////////////////////////////
		 double sky_variance_ratio=sky1_col_variance/sky2_col_variance;    //���кŷ���Լ����Ե��ǽ
		 float final_sky_pixel=0;
//  �б�ǰ֡��������򣺵�һ��
		 //����Ƶ���ϣ�����֪ʶ��
		 //1  ǰ��֡����� �����С����ͻ��
		 //2  ǰ��֡����� �����ֵҲ����ͻ��
		 //�ȸ��������� ȷ�����������������һ�������  
		 //�����һ����������� ƽ���Ҷ�ֵ����ǰ֡��ʵ��ջҶ�ֵ�����Ҳ���������С�����ظ���������ǰ֡��ʵ��������С���������Ϊ���������ǵ�ǰ֡���������
		 // ������ǰ֡����������࣬�жϣ�if(distance(sky1_count,last_sky_counts)<distance(sky2_count,last_sky_counts)
		 //                                      &&distance(sky1_u_mean,last_sky_means)<distance(sky2_u_mean,last_sky_means))
		 double  lasts_sky_pixel_num_mean=0;
		Vector3d lasts_sky_region_mean_means;
		float sky1_num_diatance=0;
		float sky2_num_diatance=0;
		float  sky1_mean_diatance=0;
		float  sky2_mean_diatance=0;
	if(lasts_sky_pixel_num.size()!=0)
	{   
		
		for(int i=0;i<lasts_sky_pixel_num.size();i++)
		{
		lasts_sky_pixel_num_mean+=lasts_sky_pixel_num[i];
		lasts_sky_region_mean_means+=lasts_sky_region_mean[i];
		}
		lasts_sky_pixel_num_mean/=lasts_sky_pixel_num.size();
		lasts_sky_region_mean_means(0)/=lasts_sky_region_mean.size();
		lasts_sky_region_mean_means(1)/=lasts_sky_region_mean.size();
		lasts_sky_region_mean_means(2)/=lasts_sky_region_mean.size();
	
		float sky1_num_diatance=abs( sky1_count-lasts_sky_pixel_num_mean);
		float sky2_num_diatance=abs( sky2_count-lasts_sky_pixel_num_mean);
		float  sky1_mean_diatance=Euclidean_distance(sky1_u_mean,lasts_sky_region_mean_means);
		float  sky2_mean_diatance=Euclidean_distance(sky2_u_mean,lasts_sky_region_mean_means);
	}
	cout<<"lasts_sky_pixel_num.size() :"<<lasts_sky_pixel_num.size()<<endl;
     cout<<"lasts_sky_pixel_num_mean : "<<lasts_sky_pixel_num_mean<<endl;
	 cout<<"lasts_sky_region_mean_means : "<<lasts_sky_region_mean_means<<endl;
	if(sky1_num_diatance<sky2_num_diatance&&sky1_mean_diatance<sky2_mean_diatance&&false)
		{cout<<"mmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"<<endl;
	                              lasts_sky_pixel_num.push_back(sky1_count);
								  lasts_sky_region_mean.push_back(sky1_u_mean);
								  if(lasts_sky_pixel_num.size()>10)

									{
										lasts_sky_pixel_num.pop_front();
										lasts_sky_region_mean.pop_front();
									}
		              for(int col=0;col<width;col++)
						  for(int row=0;row<bopt[col];row++)
						  {
							   if(class_sky.at<uchar>(row,col)==0)							  
								{
									out.at<uchar>(row,col)=0;
							     final_sky_pixel++;
								
							    }
						  }	  
		}
		else if(sky1_num_diatance>sky2_num_diatance&&sky1_mean_diatance>sky2_mean_diatance&&false)
		{           cout<<"iiiiiiiiiiiiiiiii"<<endl;      
			lasts_sky_pixel_num.push_back(sky2_count);
								lasts_sky_region_mean.push_back(sky2_u_mean);
								if(lasts_sky_pixel_num.size()>10)

									{
										lasts_sky_pixel_num.pop_front();
										lasts_sky_region_mean.pop_front();
									}
			  for(int col=0;col<width;col++)
						  for(int row=0;row<bopt[col];row++)
						  {
							   if(class_sky.at<uchar>(row,col)==255)						
								{
									out.at<uchar>(row,col)=0;	
							   final_sky_pixel++;
							    
							   }
					  }
		}
	//  �б�ǰ֡��������򣺵ڶ���
	else if((1.4<sky_variance_ratio||sky_variance_ratio<0.7)&&false)    //   ʵ�����һ���Ǻܼ������� ���������������� ������Ϊ �����һ������
														   //	�кŷ���̫��Ҳ���Ǵ����������ɢ ������Ϊ����������α�������         
		{   cout<<"jjjjjjjjjjjjjjjjjjjjjjjjj"<<endl;
		  if(sky_variance_ratio>1.4)
		          {
			              for(int col=0;col<width;col++)
						  for(int row=0;row<bopt[col];row++)
						  {
							   if(class_sky.at<uchar>(row,col)==255)						
								{
									out.at<uchar>(row,col)=0;	
							   final_sky_pixel++;
							    lasts_sky_pixel_num.push_back(sky2_count);
								lasts_sky_region_mean.push_back(sky2_u_mean);
								if(lasts_sky_pixel_num.size()>10)

									{
										lasts_sky_pixel_num.pop_front();
										lasts_sky_region_mean.pop_front();
									}
							   }
						  }
		           }
	       else 
		          {
		  		         for(int col=0;col<width;col++)
						  for(int row=0;row<bopt[col];row++)
						  {
							   if(class_sky.at<uchar>(row,col)==0)							  
								{
									out.at<uchar>(row,col)=0;
							     final_sky_pixel++;
								  lasts_sky_pixel_num.push_back(sky1_count);
								  lasts_sky_region_mean.push_back(sky1_u_mean);
								  if(lasts_sky_pixel_num.size()>10)

									{
										lasts_sky_pixel_num.pop_front();
										lasts_sky_region_mean.pop_front();
									}
							    }
						  }	  
		          }
		   }

 // �б�ǰ֡��������򣺵�����
		else                                                             //������������кŷ�������̫����������������ֱ���ground�ľ���
																		//����ʵ��������ж�,�� ������Ϊ��ʵ�������
		     {cout<<"kkkkkkkkkkkkkkkkkkkkkkkkk"<<endl;


				 if(sky1_ground_M>sky2_ground_M)
				 {
					             lasts_sky_pixel_num.push_back(sky1_count);
								 lasts_sky_region_mean.push_back(sky1_u_mean);
								 if(lasts_sky_pixel_num.size()>10)

									{
										lasts_sky_pixel_num.pop_front();
										lasts_sky_region_mean.pop_front();
									}
						for(int col=0;col<width;col++)
						  for(int row=0;row<bopt[col];row++)
						  {
							   if(class_sky.at<uchar>(row,col)==0)
							  {
								out.at<uchar>(row,col)=0;
								 final_sky_pixel++;
								
							   }
						  }
				 }
				 else
				 {                 lasts_sky_pixel_num.push_back(sky2_count);
									lasts_sky_region_mean.push_back(sky2_u_mean);
									if(lasts_sky_pixel_num.size()>10)

									{
										lasts_sky_pixel_num.pop_front();
										lasts_sky_region_mean.pop_front();
									}
						 for(int col=0;col<width;col++)
						  for(int row=0;row<bopt[col];row++)
						  {
							   if(class_sky.at<uchar>(row,col)==255)
							  {
								out.at<uchar>(row,col)=0;
								 final_sky_pixel++;
								   
							   }
						  }
				
			
				 }
																				//	cout<<"sky1_ground_M : "<<sky1_ground_M<<endl;
		}			
																		  //	cout<<"sky2_ground_M : "<<sky2_ground_M<<endl;
									 
																		
					/*if(final_sky_pixel<height*width/50)	
					{cout<<"bbbbbbbbbbbbbbbbbbb"<<endl;
					 cvtColor(channel3img,out,CV_RGB2GRAY);	
					}*/
					                                                    
	/////////////////////////////
																			
  //����Ѱ�����  
	Mat finalout;
	out.copyTo(finalout);
	Mat check_lowtextrue;
	Mat tmp;
	cvtColor(channel3img,tmp,CV_RGB2GRAY);
   low_textrue_check(tmp,check_lowtextrue,3);

    imshow("����ͼƬ������������",check_lowtextrue);
	 imwrite("����ͼƬ������������.png",check_lowtextrue);
	Vector3d sky_opt_mean=cal_Vector3d_mean( channel3img,out);
	cout<<"sky_opt_mean  "<<sky_opt_mean<<endl;
		
      for(int col=0;col<width;col++)
	  {
		  bool check_1=false;
		 //  for(int row=0;row<height-1;row++)
		   for(int row=0;row<height;row++)             //�޸�
			  {
				if(out.at<uchar>(row,col)==0&&out.at<uchar>(row+1,col)!=0)//ȷ��ÿһ������������������ķֽ�㣬�Ա㿪ʼɨ��
					check_1=true;
				////////////
				else if(out.at<uchar>(row,col)!=0)         //�޸�//�Ӷ�����������
					check_1=true;
					////////
				if(check_1)
						{
							//cout<<"check"<<endl;
						Vector3d check_pixel;
					   check_pixel(0)=channel3img.at<Vec3b>(row,col)(0);
					   check_pixel(1)=channel3img.at<Vec3b>(row,col)(1);
					   check_pixel(2)=channel3img.at<Vec3b>(row,col)(2);	
					   float distance=Euclidean_distance(check_pixel,sky_opt_mean);
					   if(distance<100&&check_lowtextrue.at<uchar>(row,col)==0)  //�޸�
						finalout.at<uchar>(row,col)=0;
						}
			  }
	  }
	  //������
	for(int row=0;row<height;row++)    
		 for(int col=0;col<width;col++)
		 {
		out.at<uchar>(row,col)= finalout.at<uchar>(row,col);
		 
		 }
	  
}	

