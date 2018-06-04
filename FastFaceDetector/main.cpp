// FastFaceDetector.cpp : アプリケーションのエントリ ポイントを定義します。
/*************************************************************************
Author : Kei Nakahama
Usage  : Stand front of your camera and execute this program.
Comment: if you have a question, let me know. I wish I'll be your help.
		 keishotbow@gmail.com
**************************************************************************/

#include "stdafx.h"
#include "FaceDetector.h"
#include "KGraph.h"
#include <chrono>

using namespace cv;
using namespace chrono;
using namespace std;

const string WINDOW_NAME("camera");
const string CASCADE_FILE_NAME("C:/opencv/sources/data/haarcascades_cuda/haarcascade_frontalface_alt2.xml");
const string SMILE_CASCADE_FILE_NAME("C:/opencv/sources/data/haarcascades_cuda/haarcascade_smile.xml");

// メイン
int main() {
	VideoCapture cap(0); // Webカメラを取得
	if (!cap.isOpened()) {
		cout << "カメラが取得できませんでした。接続状況を確認してください。\n";
		exit(1);
	}

	namedWindow(WINDOW_NAME, WINDOW_AUTOSIZE);

	// カメラのフレームサイズ取得
	Size frame_size((int)cap.get(CAP_PROP_FRAME_WIDTH), (int)cap.get(CAP_PROP_FRAME_HEIGHT));

	Mat frame(frame_size, CV_8UC3); // 表示用フレーム(カメラフレームのサイズと8bit3チャンネルで初期化)

	// FaceDetector型オブジェクト生成, コンストラクタにCascade FileとVideoCapture(cap)渡す
	FaceDetector detector(CASCADE_FILE_NAME, SMILE_CASCADE_FILE_NAME, cap);

	// ファイル入出力準備とエラー処理
	ofstream fout("ResultProcessingTime2.csv");
	if (fout.fail()) {
		cerr << "ファイルをオープン出来ませんでした。" << endl;
		exit(1);
	}

	//Mat graph(600, 800, CV_8UC3); // 640*480, 3チャンネルのMat画像を用意
	//KGraph kgraph(graph);

	int frameCount = 0;
	// 顔検出開始
	auto start = high_resolution_clock::now();
	while (frameCount < 30)
	{
		frameCount++;
		detector >> frame; // 演算子オーバーロード. 取得フレームを顔検出にかける
	
		if (detector.isFaceFound() == true) {
			rectangle(frame, detector.getFaceRect(), Scalar(0, 255, 0), 3, LINE_AA);
			circle(frame, detector.getFaceCenterPoint(), 30, Scalar(255, 0, 0), 2, LINE_AA);
		}

		//kgraph.showGraph(graph);

		imshow(WINDOW_NAME, frame);
		
		if (cvWaitKey(1) >= 0) break; // キー入力待ち
	}
	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start).count();
	fout.close();
	cout << "ファイルをクローズしました。" << endl;
	cout << frameCount << ", " << duration << " ms, " << endl;

    return 0;
}