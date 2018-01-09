#pragma once
#pragma once
#include <Kinect.h>
#include <opencv2\opencv.hpp>
#include <iostream>
#include <string>
// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

class CBodyBasics

{
	static constexpr  const char*		rgbSavePathBase="D:\\env\\kinect\\rgb\\";
	static constexpr  const char*		rgbDepthSavePathBase = "D:\\env\\kinect\\rgb_depth\\";
	static constexpr  const char*		depthSavePathBase = "D:\\env\\kinect\\depth\\";
	static constexpr  const char*		depthRgbSavePathBase = "D:\\env\\kinect\\depth_rgb\\";
	static const int        cColorWidth = 1920;
	static const int        cColorHeight = 1080;
	//kinect 2.0 ����ȿռ�ĸ�*���� 424 * 512���ڹ�������˵��
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
	static const int        cInfraredWidth = 512;
	static const int        cInfraredHeight = 424;

public:
	CBodyBasics();
	~CBodyBasics();
	void                    Update();//��ùǼܡ�������ֵͼ�������Ϣ
	HRESULT                 InitializeDefaultSensor();//���ڳ�ʼ��kinect

private:
	IKinectSensor*          m_pKinectSensor;//kinectԴ
	ICoordinateMapper*      m_pCoordinateMapper;//��������任
	IBodyFrameReader*       m_pBodyFrameReader;//���ڹǼ����ݶ�ȡ
	IDepthFrameReader*      m_pDepthFrameReader;//����������ݶ�ȡ
	IBodyIndexFrameReader*  m_pBodyIndexFrameReader;//���ڱ�����ֵͼ��ȡ
	IColorFrameReader*		m_pColorFrameReader; //rgb
	IInfraredFrameReader*	m_pInfraredFrameReader; //infrared
	DepthSpacePoint*        m_pDepthCoordinates;
	CameraSpacePoint*		m_depthCameraSpacePoint;
	CameraSpacePoint*		m_colorCameraSpacePoint;
	CameraSpacePoint*		m_colorOupputCameraSpacePoint;
	ColorSpacePoint*		m_pColorCoordinates;
													//ͨ����õ�����Ϣ���ѹǼܺͱ�����ֵͼ������
	void                    ProcessBody(int nBodyCount, IBody** ppBodies);
	//���Ǽܺ���
	void DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1);
	//���ֵ�״̬����
	void DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState);
	void ProcessFrame(INT64 nTime,
		const UINT16* pDepthBuffer,
		const RGBQUAD* pColorBuffer);
	void writeToFile(CameraSpacePoint* points, std::string savePath);
	
	//��ʾͼ���Mat
	cv::Mat skeletonImg;
	cv::Mat depthImg;
	cv::Mat depthNormalImg;
	cv::Mat rgbImg;
	cv::Mat infraredImg;
	cv::Mat infraredNormImg;
	cv::Mat rgbdImg;
	char*	writeArray;
};

