// OpencvTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"  
#include <iostream>    
#include <io.h>
#include <fstream>
#include <string>
#include<opencv2\opencv.hpp>
#include <stack>
#include <tesseract/baseapi.h>
#include <strngs.h>
#include <leptonica/allheaders.h>

using namespace cv;
using namespace std;
bool CLASS1 = false;

void getAllFile(string path, vector<string>& files) {
	char ext[_MAX_EXT];
	intptr_t hfile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hfile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			if ((fileinfo.attrib & _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					getAllFile(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else {
				_splitpath(p.assign(path).append("\\").append(fileinfo.name).c_str(), NULL, NULL, NULL, ext);
				if (strcmp(ext, ".jpg") == 0 ||
					strcmp(ext, ".png") == 0 ||
					strcmp(ext, ".bmp") == 0 ||
					strcmp(ext, ".jpeg") == 0 ||
					strcmp(ext, ".tiff") == 0 ||
					strcmp(ext, ".psd") == 0)
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hfile, &fileinfo) == 0);

		_findclose(hfile);
	}
}

//阈值化处理
void __Boolean(Mat& image) {
	int height = image.rows;
	int width = image.cols * image.channels();
	/*int Nullpoint = 0, Sumpoint = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j){
			if (image.at<uchar>(i, j) == 0) {
				++Nullpoint;
			}
			++Sumpoint;
		}
	}
	double Percent = 1.0*Nullpoint / Sumpoint;*/
	if (image.at<uchar>(0, 0) == 0) CLASS1 = true;
	if (CLASS1){
		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j){
				if (image.at<uchar>(i, j) == 0 || image.at<uchar>(i, j) >= 200) {
					image.at<uchar>(i, j) = 0;
				}
				else image.at<uchar>(i, j) = 255;
			}
		}
	}
}

//去除红色盖章
void __First_Deal(Mat& image) {
	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			if (image.at<Vec3b>(i, j)[2] - image.at<Vec3b>(i, j)[0] > 20 &&
				image.at<Vec3b>(i, j)[2] - image.at<Vec3b>(i, j)[1] > 20) {
				Vec3b pixel;
				pixel[0] = 240;
				pixel[1] = 240;
				pixel[2] = 240;
				image.at<Vec3b>(i, j) = pixel;
			}
		}
	}
}

//灰度化
void __Gray(Mat& image) {
	cvtColor(image, image, CV_BGR2GRAY);
}

//图片缩放
void __Zoom_Out(Mat& image) {
	int height = image.rows;
	int width = image.cols;
	while (height >= 1000) {
		height /= 2;
		width /= 2;
	}
	resize(image, image, Size(width, height), 0, 0, INTER_AREA);
}

//水印处理-1
void threshold_1(Mat& image, Mat& dst, int val1, int val2) {
	int img_row = image.rows, int_col = image.cols;
	for (int i = 0; i < image.rows; ++i) {
		for (int j = 0; j < image.cols; ++j) {
			if (image.at<uchar>(i, j) >= val1) {
				dst.at<uchar>(i, j) = val2;
			}
		}
	}
}

//去水印-总
void __InPaint(Mat& image) {
	Mat imageMask = Mat(image.size(), CV_8UC1, Scalar::all(0));
	threshold_1(image, imageMask, 183, 255);
/*
	Mat kernal1 = getStructuringElement(MORPH_RECT, Size(4, 4));
	Mat kernal2 = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imageMask, imageMask, kernal1);
	erode(imageMask, imageMask, kernal2);
*/
	cv::imshow("imageMask", imageMask);
	inpaint(image, imageMask, image, 0, INPAINT_TELEA);
}

//二值化
void __Bool_deal(Mat& image, int x) {
	if (x == 0) {
		int height = image.rows, width = image.cols;
		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j) {
				if (image.at<uchar>(i, j) >= 30)
					image.at<uchar>(i, j) = 0;
				else image.at<uchar>(i, j) = 255;
			}
		}
	}
	else {
		int height = image.rows, width = image.cols;
		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j) {
				if (image.at<uchar>(i, j) >= 200)
					image.at<uchar>(i, j) = 255;
			}
		}
	}
}

//腐蚀膨胀
Mat __Region(Mat& image) {
	//Mat sobel;
	//Sobel(image, sobel, CV_8U, 1, 0, 3);

	//Mat binary;
	//threshold(sobel, binary, 0, 255, THRESH_OTSU + THRESH_BINARY);

	Mat element1 = getStructuringElement(MORPH_RECT, Size(19, 2));//19,2
	Mat element2 = getStructuringElement(MORPH_RECT, Size(22, 1));//22,1

	//膨胀
	Mat dilate1;
	dilate(image, dilate1, element1);
	//腐蚀
	Mat erode1;
	erode(dilate1, erode1, element2);
	//再膨胀
	Mat dilate2;
	dilate(erode1, dilate2, element1);

	//namedWindow("Sobel", CV_WINDOW_AUTOSIZE);
	//imshow("Sobel", sobel);
	return dilate2;
}

//提取文字区域
void getConnectedRegion(Mat& src, vector<Rect>& RectList, Mat& image) {
	int img_row = src.rows, img_col = src.cols;
	/*
	//数组
	bool flag[1007][1007];
	memset(flag, 0, sizeof(flag));
	
	for (int i = 0; i < img_row; ++i) {
		for (int j = 0; j < img_col; ++j) {
			if (src.ptr<uchar>(i)[j] == 255 && flag[i][j] == 0) {

				queue<pair<int, int> > st;
				st.push(make_pair(i, j));
				flag[i][j] = 1;
				int min_x = i, min_y = j;
				int max_x = i, max_y = j;
				while (!st.empty()) {
					pair<int, int> t;
					t = st.front(); st.pop();
					if (min_x > t.first) min_x = t.first;
					if (min_y > t.second) min_y = t.second;
					if (max_x < t.first) max_x = t.first;
					if (max_y < t.second) max_y = t.second;
					Point p[4];
					p[0] = Point(t.first-1<0 ? 0:t.first-1 , t.second);
					p[1] = Point(t.first+1>=img_row ? img_row-1:t.first+1, t.second);
					p[2] = Point(t.first, t.second-1<=0 ? 0:t.second-1);
					p[3] = Point(t.first, t.second+1>=img_col ? img_col-1:t.second+1);
					for (int k = 0; k < 4; ++k) {
						int xx = p[k].x;
						int yy = p[k].y;
						if (flag[xx][yy] == 0 && src.ptr<uchar>(xx)[yy] == 255) {
							st.push(make_pair(xx, yy));
							flag[xx][yy] = 1;
						}
					}
				}



				Rect rec = Rect(Point(min_x, min_y), Point(max_x, max_y));
				RectList.push_back(rec);
			}

		}
	}
	*/
	
	//Mat
	//Mat flag = Mat::zeros(Size(img_row, img_col), CV_8UC1);
	Mat flag = Mat(src.size(), CV_8UC1, Scalar::all(0));
	for (int i = 0; i < img_row; ++i) {
		for (int j = 0; j < img_col; ++j) {
			if (src.ptr<uchar>(i)[j] == 255 && flag.ptr<uchar>(i)[j] == 0) {
				
				queue<pair<int, int> > st;
				st.push(make_pair(i, j));
				flag.ptr<uchar>(i)[j] = 255;
				int min_x = i, min_y = j;
				int max_x = i, max_y = j;
				while (!st.empty()) {
					pair<int, int> t;
					t = st.front(); st.pop();
					if (min_x > t.first) min_x = t.first;
					if (min_y > t.second) min_y = t.second;
					if (max_x < t.first) max_x = t.first;
					if (max_y < t.second) max_y = t.second;
					Point p[4];
					p[0] = Point(t.first-1<0 ? 0:t.first-1 , t.second);
					p[1] = Point(t.first+1>=img_row ? img_row-1:t.first+1, t.second);
					p[2] = Point(t.first, t.second-1<=0 ? 0:t.second-1);
					p[3] = Point(t.first, t.second+1>=img_col ? img_col-1:t.second+1);
					for (int k = 0; k < 4; ++k) {
						int xx = p[k].x;
						int yy = p[k].y;
						if (flag.ptr<uchar>(xx)[yy] == 0 && src.ptr<uchar>(xx)[yy] == 255) {
							st.push(make_pair(xx, yy));
							flag.ptr<uchar>(xx)[yy] = 255;
						}
					}
				}
				
				
				Rect rec = Rect(Point(min_x, min_y), Point(max_x, max_y));
				RectList.push_back(rec);
			}

		}
	}
	
	
	vector<Rect>::iterator it;
	for (it = RectList.begin(); it != RectList.end(); ++it) {
		int x = it->x, width = it->width;
		int y = it->y, height = it->height;
		for (int i = x; i < x + width; ++i) image.ptr<uchar>(i)[y] = 125;
		for (int i = x; i < x + width; ++i) image.ptr<uchar>(i)[y+height] = 125;

		for (int i = y; i < y + height; ++i) image.ptr<uchar>(x)[i] = 125;
		for (int i = y; i < y + height; ++i) image.ptr<uchar>(x+width)[i] = 125;

	}
	
}

Mat __PreTreatment_Image(Mat& image) {
	//去除红色盖章
	__First_Deal(image);

	//灰度化,
	__Gray(image);

	//阈值化操作(去背景色)
	__Boolean(image);

	if (!CLASS1) {
		//图片缩放
		__Zoom_Out(image);

		



		Mat tmp1 = image, tmp;

		//降噪与平滑
		//medianBlur(image, tmp1, 1);//中值滤波
		//GaussianBlur(image, tmp1, Size(3, 3), 0, 0);//高斯滤波
		//blur(image, tmp1, Size(3, 3), Point(-1, -1)); //均值滤波Point(-1, -1)为核中心

		//bilateralFilter(tmp1, tmp, 10, 50, 25);//双边滤波
		//Canny(tmp1, tmp1, 3, 9, 3);

		__Bool_deal(tmp1, 1);

		//直方图均衡
		equalizeHist(tmp1, tmp1);



		//二值化
		//__Bool_deal(tmp1, 1);

		//去水印
		Mat tmp2 = tmp1;


		//二次阈值
		__Bool_deal(tmp2, 0);


		Mat element2 = getStructuringElement(MORPH_RECT, Size(1, 1));
		erode(tmp2, tmp2, element2);
		return tmp2;
	}

	/*int height = image.rows;
	int width = image.cols * image.channels();
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (image.at<uchar>(i, j) >= 200) image.at<uchar>(i, j) = 255;
			else image.at<uchar>(i, j) = 0;
		}
	}*/
	Mat element2 = getStructuringElement(MORPH_RECT, Size(1,1));
	erode(image, image, element2);

	return image;
}

wchar_t * Utf_8ToUnicode(char* szU8) {
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), NULL, 0);
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen);
	wszString[wcsLen] = '\0';
	return wszString;
}

char* UnicodeToAnsi(const wchar_t* szStr) {
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0) {
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

void __DealPrint(char* str, char* namestr, char* numstr, bool &name, bool &num) {
	int len1 = strlen(str), len2 = 0;
	int k = 0;
	for (int i = 0; i < len1; ++i) {
		if (str[i] != ' ' && str[i] != '\n') str[k++] = str[i];
	}
	str[k++] = '\0';
	std::printf("last: %c%c\n", str[k - 3], str[k - 2]);
	std::printf("size: %d %s\n", k, str);

	if (!name) {
		int lenname = 0, idx = 0;
		if (str[k - 2] == "司"[1] && str[k - 3] == "司"[0] && str[k - 4] == "公"[1] && str[k - 5] == "公"[0]) {
			for (int i = 0; i < k; ++i) {
				if (str[i] == "称"[0] && str[i + 1] == "称"[1]) {
					idx = i+2;
					break;
				}
			}
			bool OK = false;
			if (str[idx] < 0 || str[idx] > 127) OK = true;
			for (int j = idx; j < k; ++j) {
				if (OK) {
					namestr[lenname++] = str[j];
					continue;
				}
				if (str[j + 1] < 0 || str[j + 1] > 127) OK = true;
			}
			namestr[lenname++] = '\0';
		}
		
		if (lenname > 0) name = true;
	}

	if (!num) {
		int lennum = 0, idx = 0;
		for (int i = 0; i < k-6; ++i) {
			if (str[i] == "注"[0] && str[i+1] == "注"[1] &&
				str[i+2] == "册"[0] && str[i+3] == "册"[1] &&
				str[i+4] == "号"[0] && str[i+5] == "号"[1]) {
				idx = i + 6;
				break;
			}
		}
		bool OK = false;
		if (str[idx] >= '0' && str[idx] <= '9') OK = true;
		for (int j = idx; j < k; ++j) {
			if (OK) {
				numstr[lennum++] = str[j];
			}
			if (str[j + 1] >= '0' && str[j + 1] <= '9') OK = true;
		}
		if(OK) numstr[lennum++] = '\0';
		if (lennum > 0) num = true;
	}
}

int _tmain(int argc, _TCHAR* argv[]){

	/*Mat image1 = imread("C:\\Users\\juntysun\\Desktop\\OpenCV_Temp\\TMExample\\19.png", IMREAD_COLOR);
	cv::imshow("ttest1", image1);
	cv::waitKey();
	image1 = __PreTreatment_Image(image1);
	cv::imshow("ttest2", image1);
	cv::waitKey();*/

	string path = "C:\\Users\\juntysun\\Desktop\\OpenCV_Temp\\TMExample";
	vector<string> files;
	getAllFile(path, files);
	size_t file_size = files.size();
	std::printf("处理图片个数：%d\n", file_size);

	ofstream outFile;
	outFile.open("C:\\Users\\juntysun\\Desktop\\PrintData\\data.csv", ios::out);
	outFile << "企业名称," << "注册号" << endl;
	for (int current = 0; current < file_size; ++current) {
		//读入图片，注意图片路径    
		Mat image = imread(files[current], IMREAD_COLOR);

		
		//图片读入成功与否判定    
		if (!image.data) {
			std::cout << "图像不存在!" << endl;
			system("pause");
			return -1;
		}
	
		//图像预处理
		image = __PreTreatment_Image(image);
		Mat dst = image;
	
	
		Mat res = image.clone();
		//提取文字区域
		//Canny(res, res, 3, 9, 3);
		Mat tmp = __Region(image);
		vector<Rect> RectList;
		getConnectedRegion(tmp, RectList, dst);
		cv::imshow(files[current], res);
		cv::waitKey();
		/*vector<int> a;
		a.push_back(IMWRITE_PNG_COMPRESSION);
		a.push_back(9);
		imwrite("1.png", res, a);*/

		/*Pix *img;
		img = pixRead("C:\\Users\\juntysun\\Documents\\Visual Studio 2013\\Projects\\OpencvTest\\OpencvTest\\1.png");*/

		/*cv::imshow("123", res);
		std::printf("REC.SIZE(): %d\n", RectList[0].width);*/



		tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
		api->Init(NULL, "chi_sim");

		//api->SetImage((uchar*)res.data, res.size().width, res.size().height, 1, res.size().width);
		//api->SetImage(img);
		/*api->Recognize(0);
		Boxa* boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
		printf("%d Textline\n", boxes->n);
		for (int i = 0; i < boxes->n; ++i) {
			BOX* box = boxaGetBox(boxes, i, L_CLONE);
			api->SetRectangle(box->x, box->y, box->w, box->h);
			char* out = api->GetUTF8Text();
			int conf = api->MeanTextConf();
			wchar_t* chi_out = Utf_8ToUnicode(out);
			char* chi_res = UnicodeToAnsi(chi_out);
			cout << chi_res << endl;
			cout << out << endl;
			///233
		}*/
		
		//print to Excel.
		
		bool nameok = false, numok = false;
		
		char* exl_name = new char[50];
		char* exl_num = new char[50];
		for (int rect = 0; rect < RectList.size(); ++rect) {
			if (RectList[rect].width >= 15 && RectList[rect].width <= 37) {


				Rect rec;
				rec.width = RectList[rect].height+14;
				rec.height = RectList[rect].width+14;

				Mat restmp = Mat(rec.size(), CV_8UC1, Scalar::all(255));

				int x = RectList[rect].x, y = RectList[rect].y;
				int width = RectList[rect].width, height = RectList[rect].height;

				for (int i = x; i < width + x; ++i)
					for (int j = y; j < height + y; ++j)
						restmp.ptr<uchar>(i - x+7)[j - y+7] = res.ptr<uchar>(i)[j] == 255 ? 0 : 255;

			
			
				api->SetImage((uchar*)restmp.data, restmp.size().width, restmp.size().height, 1, restmp.size().width);
			
				//api->SetImage(img);
			
				char* out = api->GetUTF8Text();
				wchar_t* chi_out = Utf_8ToUnicode(out);
				char* chi_res = UnicodeToAnsi(chi_out);
				std::cout << chi_res;
			
			
				__DealPrint(chi_res, exl_name, exl_num, nameok, numok);
			
				std::printf("%d %d\n", nameok, numok);
				if (nameok == true && numok == true) {
					break;
				}

				//测试
				/*cv::imshow("test", restmp);
				cv::waitKey();*/
			}
		}
	
		api->End();
		outFile << exl_name << ",'" << exl_num << endl;
		
	
	}
	outFile.close();

	/*int len_chi_res = 0;
	for (int i = 0; i < strlen(chi_res); ++i) {
		if (chi_res[i] != ' ') {
			chi_res[len_chi_res++] = chi_res[i];
		}
	}
	char qi = "企"[0], qii = "企"[1];
	if (qi == chi_res[0] && qii == chi_res[1])
		printf("企");
	printf("%c%c\n%c%c\n", chi_res[2], chi_res[3], chi_res[4], chi_res[5]);
	printf("%c%c", qi, qii);
	printf("11,12: %c%c\n", chi_res[11], chi_res[12]);
	printf("%d%d%d%d\n", chi_res[0], chi_res[1], chi_res[2], chi_res[3]);
	printf("%d\n", chi_res[11]);
	int k = 0;
	char anss[10000];
	for (int i = 0; i < len_chi_res; ++i) {
		if (chi_res[i] >= 0 && chi_res[i] <= 127)
			anss[k++] = chi_res[i];
	}
	anss[k++] = '\0';
	cout << anss << endl;*/

	cv::waitKey();

	return 0;

}