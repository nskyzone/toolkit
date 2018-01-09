#include "stdafx.h"
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main() {
	Mat img = imread("C:\\Users\\lenovo\\Desktop\\mandrill.bmp", 0);

	cvNamedWindow("ԭͼ");
	imshow("ԭͼ ", img);
	imwrite("ԭͼ.jpg", img);
	waitKey(0);
	//�Ҹ���Ȥ����
	Mat roi = img(Rect(55, 10, 145, 40));
	//���׿�
	Mat src_roi = imread("C:\\Users\\lenovo\\Desktop\\mandrill.bmp", 0);
	rectangle(src_roi, Rect(55, 10, 145, 40), Scalar(255, 255, 255), 2); //������Ȥ��������
	imshow("����", src_roi);
	imwrite("����.jpg", src_roi);
	waitKey(0);
	//ƽ��
	Mat src_py = imread("C:\\Users\\lenovo\\Desktop\\mandrill.bmp", 0);
	Mat imageROI = src_py(Rect(55, 50, 145, 40));
	roi.copyTo(imageROI, roi);
	cvNamedWindow("ƽ��");
	imshow("ƽ��", src_py);
	imwrite("ƽ��.jpg", src_py);
	waitKey(0);
	//����ƽ��
	Mat roi1 = img(Rect(55, 10, 145, 40));
	Mat src_jx_py = imread("C:\\Users\\lenovo\\Desktop\\mandrill.bmp", 0);
	Mat dst;
	dst.create(roi1.size(), roi1.type());
	Mat map_x;
	Mat map_y;
	map_x.create(roi1.size(), CV_32FC1);
	map_y.create(roi1.size(), CV_32FC1);
	for (int i = 0; i < roi.rows; ++i)
	{
		for (int j = 0; j < roi.cols; ++j)
		{
			map_x.at<float>(i, j) = (float)j;//j;//(src.cols - j) ;
			map_y.at<float>(i, j) = (float)(roi1.rows - i);
		}
	}
	remap(roi1, dst, map_x, map_y, CV_INTER_LINEAR);

	cvNamedWindow("ROI1");
	imshow("ROI1", dst);
	waitKey(0);
	Mat imageROI1 = src_jx_py(Rect(55, 50, 145, 40));
	dst.copyTo(imageROI1, roi1);
	cvNamedWindow("�Գ�");
	imshow("�Գ�", src_jx_py);

	imwrite("�Գ�.jpg", src_jx_py);
	waitKey(0);
	return 0;
}