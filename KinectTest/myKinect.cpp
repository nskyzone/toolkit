#include "stdafx.h"
#include "myKinect.h"
#pragma warning(disable:4996)
using namespace std;
static const int stride = 2;
static const int outputHeight = 1080 / stride;
static const int outputWidth = 1920 / stride;
int CNT = 0;
void CBodyBasics:: writeToFile(CameraSpacePoint* points, string savePath) {
	int size = 0;
	for (int i = 0; i < cColorHeight; i += stride) {
		for (int j = 0; j < cColorWidth; j += stride,size++) {
			CameraSpacePoint csp = points[i * cColorWidth + j];
			m_colorOupputCameraSpacePoint[size] = CameraSpacePoint{ csp.X, -csp.Y, csp.Z };
		}
	}
	//for (int i = 0; i < cColorHeight * cColorWidth; i++) {
	//	int t = sprintf(writeArray + size, "%.4f,%.4f,%.4f\t", points[i].X,points[i].Y,points[i].Z);
	//	size += t;
	//}
	//clock_t start, finish;
	//start = clock(); 
	FILE * fp = fopen(savePath.data(), "wb");
	fwrite(m_colorOupputCameraSpacePoint, sizeof(CameraSpacePoint), outputHeight * outputWidth, fp);
	//fclose(fp);
	//finish = clock();
	//double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	//printf("fwrite %f seconds\n", duration);
	
}
// 转换depth图像到cv::Mat  
cv::Mat ConvertMat(const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth)
{
	cv::Mat img(nHeight, nWidth, CV_8UC3);
	uchar* p_mat = img.data;

	const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

	while (pBuffer < pBufferEnd)
	{
		USHORT depth = *pBuffer;

		BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);

		*p_mat = intensity;
		p_mat++;
		*p_mat = intensity;
		p_mat++;
		*p_mat = intensity;
		p_mat++;

		++pBuffer;
	}
	return img;
}
// 转换color图像到cv::Mat  
cv::Mat ConvertMat(const RGBQUAD* pBuffer, int nWidth, int nHeight)
{
	cv::Mat img(nHeight, nWidth, CV_8UC3);
	uchar* p_mat = img.data;

	const RGBQUAD* pBufferEnd = pBuffer + (nWidth * nHeight);

	while (pBuffer < pBufferEnd)
	{
		*p_mat = pBuffer->rgbBlue;
		p_mat++;
		*p_mat = pBuffer->rgbGreen;
		p_mat++;
		*p_mat = pBuffer->rgbRed;
		p_mat++;

		++pBuffer;
	}
	return img;
}

void saveDepth2RGB(cv::Mat i_rgb, ColorSpacePoint* m_pColorCoordinates, std::string savePath) {
	cv::Mat i_depthToRgb(424, 512, CV_8UC3);
	for (int i = 0; i < 424 * 512; i++)
	{
		ColorSpacePoint p = m_pColorCoordinates[i];
		if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
		{
			int colorX = static_cast<int>(p.X + 0.5f);
			int colorY = static_cast<int>(p.Y + 0.5f);

			if ((colorX >= 0 && colorX < 1920) && (colorY >= 0 && colorY < 1080))
			{
				i_depthToRgb.data[i * 3] = i_rgb.data[(colorY * 1920 + colorX) * 3];
				i_depthToRgb.data[i * 3 + 1] = i_rgb.data[(colorY * 1920 + colorX) * 3 + 1];
				i_depthToRgb.data[i * 3 + 2] = i_rgb.data[(colorY * 1920 + colorX) * 3 + 2];
			}
		}
	}
	imshow("depthToRgb", i_depthToRgb);
	//cv::imwrite(savePath, i_depthToRgb);
}
void saveRGB2Depth(cv::Mat i_rgb, UINT16* depth, DepthSpacePoint* m_pColorCoordinates, std::string savePath, USHORT nDepthMinReliableDistance, USHORT nDepthMaxReliableDistance) {
	cv::Mat i_depthToRgb(1080, 1920, CV_16UC1);
	UINT16* depthData = (UINT16*)i_depthToRgb.data;
	for (int i = 0; i < 1080 * 1920; i++)
	{
		DepthSpacePoint p = m_pColorCoordinates[i];
		if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
		{
			int colorX = static_cast<int>(p.X + 0.5f);
			int colorY = static_cast<int>(p.Y + 0.5f);

			if ((colorX >= 0 && colorX < 512) && (colorY >= 0 && colorY < 424)){
				int tmp = depth[(colorY * 512 + colorX)];
				//if(tmp > nDepthMinReliableDistance && tmp < nDepthMaxReliableDistance){
					//i_depthToRgb.data[i * 3] = i_rgb.data[(colorY * 1920 + colorX) * 3];
					//i_depthToRgb.data[i * 3 + 1] = i_rgb.data[(colorY * 1920 + colorX) * 3 + 1];
					//i_depthToRgb.data[i * 3 + 2] = i_rgb.data[(colorY * 1920 + colorX) * 3 + 2];
					depthData[i] = (depth[(colorY * 512 + colorX)] & 0xfff8) >> 3;
				//}
			}
		}
	}
	cv::Mat rgbd; resize(i_depthToRgb, rgbd, cv::Size(outputWidth, outputHeight));
	cv::namedWindow("rgbToDepth",1);
	imshow("rgbToDepth", rgbd);
	//cv::imwrite(savePath, rgbd);

	//cv::Mat rgbdImg;
	//static const int        cColorWidth = 1920;
	//static const int        cColorHeight = 1080;
	//rgbdImg.create(cColorHeight, cColorWidth, CV_16UC1); rgbdImg.setTo(0);
	//for (int i = 0; i < 1080; i++) {
	//	for (int j = 0; j < 1920; j++) {
	//		DepthSpacePoint depthPoint = m_pColorCoordinates[i * 1920 + j];
	//		int colorX = static_cast<int>(depthPoint.X + 0.5f);
	//		int colorY = static_cast<int>(depthPoint.Y + 0.5f);
	//		if ((colorX >= 0) && (colorX < 512) && (colorY >= 0) && (colorY < 424))
	//		{
	//			rgbdImg.at<UINT16>(i, j) = (depth.data[(colorY * 512 + colorX)] & 0xfff8) >> 3;
	//		}
	//	}
	//}
	//cv::Mat rgbd; resize(rgbdImg, rgbd, cv::Size(outputWidth, outputHeight));
	//cv::imshow("rgbd", rgbd);
	//cv::imwrite(savePath, rgbd);
}
/// Initializes the default Kinect sensor
HRESULT CBodyBasics::InitializeDefaultSensor()
{
	//用于判断每次读取操作的成功与否
	HRESULT hr;

	//搜索kinect
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr)) {
		return hr;
	}

	//找到kinect设备
	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		IBodyFrameSource* pBodyFrameSource = NULL;//读取骨架
		IDepthFrameSource* pDepthFrameSource = NULL;//读取深度信息
		IInfraredFrameSource *pInfraredFrameSource = NULL; // 红外信息
		IBodyIndexFrameSource* pBodyIndexFrameSource = NULL;//读取背景二值图
		IColorFrameSource* pColorFrameSource = NULL;
															//打开kinect
		hr = m_pKinectSensor->Open();

		//coordinatemapper
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		//bodyframe
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		}

		//depth frame
		if (SUCCEEDED(hr)) {
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr)) {
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}
		//color frame
		if (SUCCEEDED(hr)) {
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr)) {
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}
		
		//infrared frame
		if (SUCCEEDED(hr)) {
			hr = m_pKinectSensor->get_InfraredFrameSource(&pInfraredFrameSource);
		}

		if (SUCCEEDED(hr)) {
			hr = pInfraredFrameSource->OpenReader(&m_pInfraredFrameReader);
		}
		//body index frame
		if (SUCCEEDED(hr)) {
			hr = m_pKinectSensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
		}

		if (SUCCEEDED(hr)) {
			hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
		}

		SafeRelease(pBodyFrameSource);
		SafeRelease(pDepthFrameSource);
		SafeRelease(pInfraredFrameSource);
		SafeRelease(pBodyIndexFrameSource);
		CameraIntrinsics* m_pCameraIntrinsics = new CameraIntrinsics();

		if (SUCCEEDED(hr))
		{
			hr = m_pCoordinateMapper->GetDepthCameraIntrinsics(m_pCameraIntrinsics);
		}
		if (SUCCEEDED(hr))
		{
			cout << "FocalLengthX : " << m_pCameraIntrinsics->FocalLengthX << endl;
			cout << "FocalLengthY : " << m_pCameraIntrinsics->FocalLengthY << endl;
			cout << "PrincipalPointX : " << m_pCameraIntrinsics->PrincipalPointX << endl;
			cout << "PrincipalPointY : " << m_pCameraIntrinsics->PrincipalPointY << endl;
			cout << "RadialDistortionFourthOrder : " << m_pCameraIntrinsics->RadialDistortionFourthOrder << endl;
			cout << "RadialDistortionSecondOrder : " << m_pCameraIntrinsics->RadialDistortionSecondOrder << endl;
			cout << "RadialDistortionSixthOrder : " << m_pCameraIntrinsics->RadialDistortionSixthOrder << endl;
		}
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		std::cout << "Kinect initialization failed!" << std::endl;
		return E_FAIL;
	}

	//skeletonImg,用于画骨架、背景二值图的MAT
	skeletonImg.create(cDepthHeight, cDepthWidth, CV_8UC3);
	skeletonImg.setTo(0);

	//depthImg,用于画深度信息的MAT
	depthImg.create(cDepthHeight, cDepthWidth, CV_16UC1);
	depthImg.setTo(0);
	depthNormalImg.create(cDepthHeight, cDepthWidth, CV_8UC1);
	depthNormalImg.setTo(0);

	//infraredImg,用于画红外信息的MAT
	infraredImg.create(cInfraredHeight, cInfraredWidth, CV_16UC1);
	infraredImg.setTo(0);
	infraredNormImg.create(cInfraredHeight, cInfraredWidth, CV_8UC1);
	infraredNormImg.setTo(0);

	rgbdImg.create(cInfraredHeight, cInfraredWidth, CV_8UC3);
	rgbdImg.setTo(0);
	//rgbImg,用于画彩色信息的MAT
	rgbImg.create(cColorHeight, cColorWidth, CV_8UC3);
	rgbImg.setTo(0);
	return hr;
}


/// Main processing function
void CBodyBasics::Update()
{
	char buffer[100];
	sprintf(buffer, "%05d", CNT);
	clock_t start = clock(), finish;
	double duration;
	if (!m_pColorFrameReader)
	{
		return;
	}

	//每次先清空skeletonImg
	skeletonImg.setTo(0);

	//如果丢失了kinect，则不继续操作
	if (!m_pBodyFrameReader)
	{
		return;
	}
	IColorFrame* pColorFrame = NULL;//rgb信息
	IBodyFrame* pBodyFrame = NULL;//骨架信息
	IDepthFrame* pDepthFrame = NULL;//深度信息
	IInfraredFrame *pInfraredFrame = NULL; // infrared
	IBodyIndexFrame* pBodyIndexFrame = NULL;//背景二值图

											//记录每次操作的成功与否
	HRESULT hr = S_OK;

	////---------------------------------------获取背景二值图并显示---------------------------------
	//if (SUCCEEDED(hr)) {
	//	hr = m_pBodyIndexFrameReader->AcquireLatestFrame(&pBodyIndexFrame);//获得背景二值图信息
	//}
	//if (SUCCEEDED(hr)) {
	//	BYTE *bodyIndexArray = new BYTE[cDepthHeight * cDepthWidth];//背景二值图是8为uchar，有人是黑色，没人是白色
	//	pBodyIndexFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, bodyIndexArray);

	//	//把背景二值图画到MAT里
	//	uchar* skeletonData = (uchar*)skeletonImg.data;
	//	for (int j = 0; j < cDepthHeight * cDepthWidth; ++j) {
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//	}
	//	delete[] bodyIndexArray;
	//}
	//SafeRelease(pBodyIndexFrame);//必须要释放，否则之后无法获得新的frame数据

	 //-----------------------获取rgb数据并显示--------------------------
	
	
	hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);
	INT64 nTime = 0;
	if (SUCCEEDED(hr))
	{
		
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = NULL;
		RGBQUAD* m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RelativeTime(&nTime);
		}
		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX)
			{
				pBuffer = m_pColorRGBX;
				nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
		}
		if (SUCCEEDED(hr))
		{
			//ProcessColor(nTime, pBuffer, nWidth, nHeight);
			rgbImg = ConvertMat(pBuffer, cColorWidth, cColorHeight);
		}
		SafeRelease(pFrameDescription);
	}

	USHORT nDepthMinReliableDistance = 0;
	USHORT nDepthMaxReliableDistance = 0;
								 //-----------------------获取深度数据并显示--------------------------
	if (SUCCEEDED(hr)) {
		hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);//获得深度数据
	}
	if (SUCCEEDED(hr))
	{
		hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
	}

	if (SUCCEEDED(hr))
	{
		hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);
	}
	if (SUCCEEDED(hr)) {
		//UINT16 *depthArray = new UINT16[cDepthHeight * cDepthWidth];//深度数据是16位unsigned int
		pDepthFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, depthArray);
		UINT nBufferSize_depth = 0;
		UINT16 *pBuffer_depth = NULL; 
		//pDepthFrame->AccessUnderlyingBuffer(&nBufferSize_depth, &pBuffer_depth);
		if (m_pCoordinateMapper->MapDepthFrameToColorSpace(cDepthHeight * cDepthWidth, depthArray, cDepthHeight * cDepthWidth, m_pColorCoordinates) == S_OK) {
			saveDepth2RGB(rgbImg, m_pColorCoordinates, depthRgbSavePathBase + std::to_string(nTime) + ".png");
		}
		//
		//if (m_pCoordinateMapper->MapColorFrameToDepthSpace(cDepthHeight * cDepthWidth, depthArray, cColorHeight * cColorWidth, m_pDepthCoordinates) == S_OK) {
		//	saveRGB2Depth(rgbImg, depthArray, m_pDepthCoordinates, rgbDepthSavePathBase + std::to_string(nTime) + ".png", nDepthMinReliableDistance, nDepthMaxReliableDistance);
		//}

		//hr = m_pCoordinateMapper->MapDepthFrameToCameraSpace(cDepthHeight * cDepthWidth, depthArray, cDepthHeight * cDepthWidth, m_depthCameraSpacePoint);
		//if (SUCCEEDED(hr)) {
		//	std::string depth_camera = "D:\\env\\kinect\\depth_camera\\" + std::to_string(nTime) + ".txt";
		//	ofstream SaveFile(depth_camera);int infNum = 0;
		//	for (int j = 0; j < cInfraredHeight * cInfraredWidth; ++j) {
		//		if(m_depthCameraSpacePoint[j].Z > 1e-9)
		//			infNum++;
		//		//SaveFile << m_depthCameraSpacePoint[j].X << "," << m_depthCameraSpacePoint[j].Y << "," << m_depthCameraSpacePoint[j].Z << ";";
		//	}
		//	SaveFile.close();
		//}
		//start = clock(),
		hr = m_pCoordinateMapper->MapColorFrameToCameraSpace(cDepthHeight * cDepthWidth, depthArray, cColorHeight * cColorWidth, m_colorCameraSpacePoint);
		//finish = clock();
		//duration = (double)(finish - start) / CLOCKS_PER_SEC;
		//printf("transfer time %f seconds\n", duration);
		if (SUCCEEDED(hr)) {
			
			 writeToFile(m_colorCameraSpacePoint, rgbDepthSavePathBase + string(buffer) + ".txt");
		}
		//finish = clock();
		//duration = (double)(finish - start) / CLOCKS_PER_SEC;
		//printf("pull depth time %f seconds\n", duration); // 0.04

		//把深度数据画到MAT中
		//uchar* depthData1 = (uchar*)depthNormalImg.data;
		//for (int j = 0; j < cDepthHeight * cDepthWidth; ++j) {
		//	//if (depthArray[j] == pBuffer_depth[j])
		//	//	printf("not same\n");
		//	UINT16 tmp = (depthArray[j] & 0xfff8) >> 3;
		//	//if (tmp > nDepthMaxReliableDistance)
		//	//	*depthData1 = 255;
		//	//else if (tmp < nDepthMinReliableDistance)
		//	//	*depthData1 = 0;
		//	//else
		//		*depthData1 = static_cast<BYTE>(256.* tmp / nDepthMaxReliableDistance);
		//	++depthData1;
		//}
		//把深度数据画到MAT中
		//depthImg,用于画深度信息的MAT
		//cv::Mat dep = ConvertMat(depthArray, cDepthWidth, cDepthHeight, nDepthMinReliableDistance, nDepthMaxReliableDistance);
		//imshow("depImg", dep);

		UINT16* depthData = (UINT16*)depthImg.data;
		for (int j = 0; j < cDepthHeight * cDepthWidth; ++j) {
			UINT16 realDepth = (depthArray[j] & 0xfff8) >> 3;
			*depthData = depthArray[j] % 256;
			++depthData;
		}
	}
	

							 //-----------------------获取红外数据并显示--------------------------
	//if (SUCCEEDED(hr)) {
	//	hr = m_pInfraredFrameReader->AcquireLatestFrame(&pInfraredFrame);//获得红外数据
	//}
	//if (SUCCEEDED(hr)) {
	//	UINT16 *infraredArray = new UINT16[cInfraredHeight * cInfraredWidth];//深度数据是16位unsigned int
	//	pInfraredFrame->CopyFrameDataToArray(cInfraredHeight * cInfraredWidth, infraredArray);


	//	//把深度数据画到MAT中
	//	//uchar* infraredData = (uchar*)infraredNormImg.data;
	//	//for (int j = 0; j < cInfraredHeight * cInfraredWidth; ++j) {
	//	//	BYTE readInfrared = 255 - static_cast<BYTE>(256 * ((infraredArray[j] & 0xfff8) >> 3) / 0x0fff);
	//	//	*infraredData = readInfrared;
	//	//	++infraredData;
	//	//}
	////	
	////	//UINT16* infraredData1 = (UINT16*)infraredImg.data;
	////	//for (int j = 0; j < cInfraredHeight * cInfraredWidth; ++j) {
	////	//	UINT16 readInfrared = ((infraredArray[j] & 0xfff8) >> 3);
	////	//	*infraredData1 = readInfrared;
	////	//	++infraredData1;
	////	//}
	//	delete[] infraredArray;
	//}

	SafeRelease(pColorFrame);//必须要释放，否则之后无法获得新的frame数据
	SafeRelease(pDepthFrame);//必须要释放，否则之后无法获得新的frame数据
	//SafeRelease(pInfraredFrame);//必须要释放，否则之后无法获得新的frame数据

	std::string rgbPath = rgbSavePathBase + string(buffer) + ".png";
	std::string depthPath = depthSavePathBase + std::to_string(nTime) + ".png";
	//std::string depthNormalPath = "D:\\env\\kinect\\depth_normal\\" + std::to_string(nTime) + ".png";
	//std::string infraredNormPath = "D:\\env\\kinect\\infrared_norm\\" + std::to_string(nTime) + ".png";
	//imshow("depthImg", depthImg);
	//imshow("rgbImg", rgbImg);
	cv::Mat dst;
	cv::resize(rgbImg, dst, cv::Size(outputWidth, outputHeight));
	cv::namedWindow("rgbImg", 1);
	imshow("rgbImg", dst);
	cv::waitKey(1);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("pull depth time %f seconds\n", duration); // 0.06
	//imshow("infraredNormImg", infraredNormImg);
	//cv::namedWindow("depthImg", 1);
	//imshow("depthImg", depthImg);

	start = clock();
	cv::imwrite(rgbPath, dst);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	//printf("save time %f seconds\n", duration); // 
	//cv::imwrite(depthPath, depthImg);
	//cv::imwrite(depthPath, depthImg);
	//cv::imwrite(infraredNormPath, infraredNormImg);
	//cv::waitKey(5);

	//-----------------------------获取骨架并显示----------------------------
	//if (SUCCEEDED(hr)) {
	//	hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);//获取骨架信息
	//}
	//if (SUCCEEDED(hr))
	//{
	//	IBody* ppBodies[BODY_COUNT] = { 0 };//每一个IBody可以追踪一个人，总共可以追踪六个人

	//	if (SUCCEEDED(hr))
	//	{
	//		//把kinect追踪到的人的信息，分别存到每一个IBody中
	//		hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
	//	}

	//	if (SUCCEEDED(hr))
	//	{
	//		//对每一个IBody，我们找到他的骨架信息，并且画出来
	//		ProcessBody(BODY_COUNT, ppBodies);
	//	}

	//	for (int i = 0; i < _countof(ppBodies); ++i)
	//	{
	//		SafeRelease(ppBodies[i]);//释放所有
	//	}
	//}
	//SafeRelease(pBodyFrame);//必须要释放，否则之后无法获得新的frame数据
	CNT++;

}
void CBodyBasics::ProcessFrame(INT64 nTime,
	const UINT16* pDepthBuffer,
	const RGBQUAD* pColorBuffer)
{

}

/// Handle new body data
void CBodyBasics::ProcessBody(int nBodyCount, IBody** ppBodies)
{
	//记录操作结果是否成功
	HRESULT hr;

	//对于每一个IBody
	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];
		if (pBody)//还没有搞明白这里pBody和下面的bTracked有什么区别
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];//存储关节点类
				HandState leftHandState = HandState_Unknown;//左手状态
				HandState rightHandState = HandState_Unknown;//右手状态

															 //获取左右手状态
				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				//存储深度坐标系中的关节点位置
				DepthSpacePoint *depthSpacePosition = new DepthSpacePoint[_countof(joints)];

				//获得关节点类
				hr = pBody->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(hr))
				{
					for (int j = 0; j < _countof(joints); ++j)
					{
						//将关节点坐标从摄像机坐标系（-1~1）转到深度坐标系（424*512）
						m_pCoordinateMapper->MapCameraPointToDepthSpace(joints[j].Position, &depthSpacePosition[j]);
					}

					//------------------------hand state left-------------------------------
					DrawHandState(depthSpacePosition[JointType_HandLeft], leftHandState);
					DrawHandState(depthSpacePosition[JointType_HandRight], rightHandState);

					//---------------------------body-------------------------------
					DrawBone(joints, depthSpacePosition, JointType_Head, JointType_Neck);
					DrawBone(joints, depthSpacePosition, JointType_Neck, JointType_SpineShoulder);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_SpineMid);
					DrawBone(joints, depthSpacePosition, JointType_SpineMid, JointType_SpineBase);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderLeft);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipLeft);

					// -----------------------Right Arm ------------------------------------ 
					DrawBone(joints, depthSpacePosition, JointType_ShoulderRight, JointType_ElbowRight);
					DrawBone(joints, depthSpacePosition, JointType_ElbowRight, JointType_WristRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_HandRight);
					DrawBone(joints, depthSpacePosition, JointType_HandRight, JointType_HandTipRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_ThumbRight);

					//----------------------------------- Left Arm--------------------------
					DrawBone(joints, depthSpacePosition, JointType_ShoulderLeft, JointType_ElbowLeft);
					DrawBone(joints, depthSpacePosition, JointType_ElbowLeft, JointType_WristLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_HandLeft);
					DrawBone(joints, depthSpacePosition, JointType_HandLeft, JointType_HandTipLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_ThumbLeft);

					// ----------------------------------Right Leg--------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipRight, JointType_KneeRight);
					DrawBone(joints, depthSpacePosition, JointType_KneeRight, JointType_AnkleRight);
					DrawBone(joints, depthSpacePosition, JointType_AnkleRight, JointType_FootRight);

					// -----------------------------------Left Leg---------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipLeft, JointType_KneeLeft);
					DrawBone(joints, depthSpacePosition, JointType_KneeLeft, JointType_AnkleLeft);
					DrawBone(joints, depthSpacePosition, JointType_AnkleLeft, JointType_FootLeft);
				}
				delete[] depthSpacePosition;
			}
		}
	}
	cv::imshow("skeletonImg", skeletonImg);
	cv::waitKey(5);
	//cv::imwrite()
}

//画手的状态
void CBodyBasics::DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState)
{
	//给不同的手势分配不同颜色
	CvScalar color;
	switch (handState) {
	case HandState_Open:
		color = cvScalar(255, 0, 0);
		break;
	case HandState_Closed:
		color = cvScalar(0, 255, 0);
		break;
	case HandState_Lasso:
		color = cvScalar(0, 0, 255);
		break;
	default://如果没有确定的手势，就不要画
		return;
	}

	circle(skeletonImg,
		cvPoint(depthSpacePosition.X, depthSpacePosition.Y),
		20, color, -1);
}


/// Draws one bone of a body (joint to joint)
void CBodyBasics::DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1)
{
	TrackingState joint0State = pJoints[joint0].TrackingState;
	TrackingState joint1State = pJoints[joint1].TrackingState;

	// If we can't find either of these joints, exit
	if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
	{
		return;
	}

	// Don't draw if both points are inferred
	if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
	{
		return;
	}

	CvPoint p1 = cvPoint(depthSpacePosition[joint0].X, depthSpacePosition[joint0].Y),
		p2 = cvPoint(depthSpacePosition[joint1].X, depthSpacePosition[joint1].Y);

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
	{
		//非常确定的骨架，用白色直线
		line(skeletonImg, p1, p2, cvScalar(255, 255, 255));
	}
	else
	{
		//不确定的骨架，用红色直线
		line(skeletonImg, p1, p2, cvScalar(0, 0, 255));
	}
}


/// Constructor
CBodyBasics::CBodyBasics() :
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pBodyFrameReader(NULL),
	m_pDepthCoordinates(NULL) {
	// create heap storage for the coorinate mapping from color to depth
	m_pDepthCoordinates = new DepthSpacePoint[cColorWidth * cColorHeight];
	m_depthCameraSpacePoint = new CameraSpacePoint[cInfraredHeight * cInfraredWidth];
	m_pColorCoordinates = new ColorSpacePoint[cInfraredHeight * cInfraredWidth];
	m_colorCameraSpacePoint = new CameraSpacePoint[cColorHeight * cColorWidth];
	m_colorOupputCameraSpacePoint = new CameraSpacePoint[outputHeight * outputWidth];
	writeArray = new char[1024 * 1024 * 100];
	depthArray = new UINT16[cDepthHeight * cDepthWidth];
}

/// Destructor
CBodyBasics::~CBodyBasics()
{
	SafeRelease(m_pBodyFrameReader);
	SafeRelease(m_pCoordinateMapper);
	if (m_pDepthCoordinates)
	{
		delete[] m_pDepthCoordinates;
		m_pDepthCoordinates = NULL;
	}
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
	SafeRelease(m_pKinectSensor);
}
