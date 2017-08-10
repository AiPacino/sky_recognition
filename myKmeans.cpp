/*
This file is part of remove_sky.
copyright��Zhu Baohua
2016.6
*/
#include"myKmeans.h"
#define k 2
using namespace std;  
 
//��������Ԫ����ŷ�������  
float getDistance(Vector3d t1, Vector3d t2)   
{  
    return sqrt((t1(0)- t2(0)) * (t1(0)- t2(0)) + (t1(1) - t2(1))* (t1(1) - t2(1))+(t1(2) - t2(2))*(t1(2) - t2(2)));  
}  
  
//�������ģ�������ǰԪ�������ĸ���  
int class_inputdata(Vector3d means[],Vector3d tuple){  
    float dist=getDistance(means[0],tuple);  
    float tmp;  
    int label=0;//��ʾ������һ����  
    for(int i=1;i<k;i++){  
        tmp=getDistance(means[i],tuple);  
        if(tmp<dist) {dist=tmp;label=i;}  
    }  
    return label;     
}  
//��ø����ؼ���ƽ�����  
float getVar(vector<Vector3d> clusters[],Vector3d means[])
{  
    float var = 0;
	float count=0;
    for (int i = 0; i < k; i++)  
    {  
        vector<Vector3d> t = clusters[i];  
        for (int j = 0; j< t.size(); j++)  
        {  count++;
            var += getDistance(t[j],means[i]);  
        }  
    }  
    //cout<<"sum:"<<sum<<endl;  
    return var/count;  
  
}  
//��õ�ǰ�صľ�ֵ�����ģ�  
Vector3d getMeans(vector<Vector3d> cluster){  
      
    double num = cluster.size();  
    Vector3d t;  
    for (double i = 0; i < num; i++)  
    {																						//	cout<<"cluster: "<<cluster[i]<<endl;
        t(0) += cluster[i](0);  
        t(1) += cluster[i](1);  
		t(2) += cluster[i](2);  
    }  
	    t(0)/=num;
		  t(1)/=num;
		    t(2)/=num;
    return t;  

}  
bool check_repeat(Vector3d means[],int tmp)
{
	for(int i=tmp;i>=1;i--)
	{
	if(means[tmp]==means[i-1])
		return true;
	}
	return false;
}
void myKMeans(Mat inputdata,Mat &outlabel, Vector3d centers[])
{  
    vector<Vector3d> clusters[k];  
 																									
    //Ĭ��һ��ʼ��ǰK��Ԫ���ֵ��Ϊk���ص����ģ���ֵ��  
    for(int i=0;i<k;i++)
	  {  
        centers[i](0)=inputdata.at<Vec3b>(i,0)(0);  
        centers[i](1)=inputdata.at<Vec3b>(i,0)(1);   
		centers[i](2)=inputdata.at<Vec3b>(i,0)(2); 
		if(i>0)
		{        int t=1;
				while(check_repeat( centers, i))
				{
				 centers[i](0)=inputdata.at<Vec3b>(i+t,0)(0);  
				centers[i](1)=inputdata.at<Vec3b>(i+t,0)(1);   
				centers[i](2)=inputdata.at<Vec3b>(i+t,0)(2); 
				t++;
				}
		}
		//cout<< "centers[i]  "<< centers[i]<<endl;
    }  
    int lable=0;  
    //����Ĭ�ϵ����ĸ��ظ�ֵ  
	int width=inputdata.cols;
	int height=inputdata.rows;
	double data_num=height;//������
																							//cout<<"data_num : "<<data_num<<endl;
    for( int i=0;i!=data_num;++i){  
		Vector3d tmp;
	    tmp(0)=inputdata.at<Vec3b>(i,0)(0);  
        tmp(1)=inputdata.at<Vec3b>(i,0)(1);   
		tmp(2)=inputdata.at<Vec3b>(i,0)(2);   
        lable=class_inputdata(centers,tmp);  
																				/*if(lable==1)
																				cout<<" lable : "<<lable<<endl;*/
        clusters[lable].push_back(tmp);  
    }  
																				//����տ�ʼ�Ĵ�  
																				/*for(lable=0;lable<k;lable++){  
																					cout<<"��"<<lable+1<<"���أ�"<<endl;  
																					cout<<clusters[lable].size()<<endl;
																					 }  */
    float oldVar=-1;  
    float newVar=getVar(clusters,centers);
																							//	cout<<"newVar  "<<newVar<<endl;
	int count_=0;
    while(abs(newVar - oldVar) >= 1) //���¾ɺ���ֵ����1��׼����ֵ���������Ա仯ʱ���㷨��ֹ  
      {  
																						// cout<<" count_ "<<count_++<<endl;
        for (int i = 0; i < k; i++) //����ÿ���ص����ĵ�  
        {  
            centers[i] = getMeans(clusters[i]);  //////////////
																							//			 cout<<"centers "<<centers[i]<<endl;  
        }  
        oldVar = newVar;  
        newVar = getVar(clusters,centers); //�����µ�׼����ֵ  
																							//	cout<<"newVar  "<<newVar<<endl;

        for (int i = 0; i < k; i++) //���ÿ����  
          {  
             clusters[i].clear();  
          }  
        //�����µ����Ļ���µĴ�  
        for(int i=0;i!=data_num;++i)
		 {  			
						Vector3d tmp;
					tmp(0)=inputdata.at<Vec3b>(i,0)(0);  
					tmp(1)=inputdata.at<Vec3b>(i,0)(1);   
					tmp(2)=inputdata.at<Vec3b>(i,0)(2);   
					lable=class_inputdata(centers,tmp);  
					clusters[lable].push_back(tmp); 
					
					outlabel.at<uchar>(i,0)=lable;
        }  
           
      }																		 /*
																			for(lable=0;lable<k;lable++){  
																					cout<<"��"<<lable+1<<"���أ�"<<endl;  
																					cout<<clusters[lable].size()<<endl; 
		
																			}  */
}     
 
