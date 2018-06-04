#pragma once

#include <iostream>
#include <chrono>
#include <opencv2\opencv.hpp>
#pragma comment(lib, "opencv_world341d.lib")
#pragma comment(lib, "opencv_world341.lib")

using namespace std;
using namespace cv;
using namespace chrono;

class FaceDetector
{
public:
	FaceDetector(const string CASCADE_FILE_NAME, VideoCapture & cap); // コンストラクタ
	FaceDetector(const string CASCADE_FILE_NAME, const string SMILE_CASCADE_FILE_NAME, VideoCapture &cap); // コンストラクタ
	~FaceDetector(); // デストラクタ

	void setFaceCascade(const string CASCADE_FILE_NAME); // カスケードファイルをセットする
	void setSmileCascade(const string SMILE_CASCADE_FILE_NAME); // スマイルカスケードファイルをセットする
	void setVideoCapture(VideoCapture &cap); // カメラ映像をセットする

	bool isFaceFound(); // 顔が見つかったかどうか
	bool isSmileFound(); // 笑顔が見つかったかどうか

	void operator>>(Mat & frame); // オブジェクト生成時にカメラ映像とcascade file渡す

	Rect getFaceRect(); // 顔領域を検出する

	Point getFaceCenterPoint(); // 顔領域の中心点を取得
	Point getSmileCenterPoint(); // 笑顔領域の中心点を取得

private:

	CascadeClassifier* m_faceCascade = nullptr; // 顔のカスケード
	CascadeClassifier* m_smileCascade = nullptr; // 笑顔のカスケード

	VideoCapture* m_videoCapture = nullptr; // ビデオキャプチャー
	
	Point  m_facePosition; // 検出された顔の中心座標
	Point  m_smilePosition; // 検出された笑顔の中心座標

	double m_scale; // 取得フレームのスケール
	int    m_resizedWidth = 320; // リサイズ後の幅
	bool   m_foundFace = false; // 顔が見つかったかどうか。初期値はfalse
	bool   m_foundSmile = false; // 笑顔が見つかったかどうか。初期値はfalse

	vector<Rect> m_allFaces; // HaarLike検出により検出された複数個の顔を格納するvector配列
	vector<Rect> m_allSmiles; // HaarLike検出により検出された複数個の笑顔を格納するvector配列
	
	Rect m_trackedFace; // 検出された顔
	Rect m_trackedSmile; // 検出された笑顔
	Rect m_faceRoi; // 顔領域のROI
	Mat  m_faceTemplate; // 検出した顔の部分領域のテンプレート画像

	bool   m_templateMatchingRunning = false; // Template Matchingが動作しているか。初期値はfalse
	int64  m_templateMatchingStartTime = 0; // Template Matching開始時間(初期値は0秒)
	int64  m_templateMatchingCurrentTime = 0; // Template Matchingの動作時間
 	double m_templateMathingMaxDuration = 3; // Template Matchingの最大動作時間(任意に変更可)
	Mat    m_matchingResult; // テンプレマッチングの結果を格納する

	Point getFacePositionAndDetect(Mat & frame); // 顔の検出を行う
		void detectFaceAllSizes(const Mat & resizedFrame); // リサイズされたフレームを受け取り、HaarLike検出する
			Rect         biggestFace(vector<Rect> & allFaces) const; // HaarLikeで検出された顔領域の内、最も大きい顔領域を返す
			Mat          getFaceTemplate(const Mat & resizedFrame, Rect trackedFace); // 検出された顔の部分領域(テンプレート画像)を得る
			Rect         doubleRectSize(const Rect & trackedFace, const Rect & resizedFrame) const; // ROI計算
			inline Point centerOfRect(const Rect & trackedFace); // 検出された顔の中心計算する

		void detectFaceAroundRoi(const Mat & resizedFrame); // ROI内で顔を検出する
		void detectFacesTemplateMatching(Mat & resizedFrame); // Template Matchingで顔を検出する
		void detectSmileAroundRoi(const Mat & resizedFrame); // 笑顔検出用

	template<typename Function>
	auto measureProcessTime(Function func);
};
