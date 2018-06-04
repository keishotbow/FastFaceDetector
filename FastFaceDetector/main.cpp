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
int main()
{
	VideoCapture cap(0); // Webカメラを取得
	if (!cap.isOpened()) {
		cout << "カメラが取得できませんでした。接続状況を確認してください。\n";
		getchar();
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
		cout << "ファイルをオープン出来ませんでした。" << endl;
		return 1;
	}
	else {
		cout << "ファイルをオープンしました。" << endl;
	}

	auto fps{ 0.0 }; // フレームレート
	auto time_per_frame{ 0.0 }; // 1フレーム毎の処理時間
	Mat graph(600, 800, CV_8UC3); // 640*480, 3チャンネルのMat画像を用意
	KGraph kgraph(graph);

	// 顔検出開始
	while (true)
	{
		//auto start = getCPUTickCount();
		auto start = system_clock::now();
		detector >> frame; // 演算子オーバーロード. 取得フレームを
		//auto end = getCPUTickCount();
		auto end = system_clock::now();

		//time_per_frame = (end - start) / getTickFrequency(); // 1フレーム毎の処理時間を計算
		auto duration = end - start; // 時間を計算
		auto nsec = duration_cast<nanoseconds>(duration).count(); // ミリ秒に変換
		//fps = (15 * fps + (1 / time_per_frame)) / 16; // フレームレート計算

		//cout << "time_per_frame: " << time_per_frame;
		cout << nsec << "ns passed..." << endl;
		//cout << ",  fps: " << fps << endl;

		// ファイル出力
		//fout << "time_per_frame: " << time_per_frame << ", fps: " << fps << endl;
		fout << "nsec," << nsec << endl;

		if (detector.isFaceFound() == true) {
			rectangle(frame, detector.getFaceRect(), Scalar(0, 255, 0), 3, LINE_4);
			//circle(frame, detector.getFaceCenterPoint(), 30, Scalar(255, 0, 0), 2, LINE_4);
		}

		//if (detector.isSmileFound() == true) {
		//	//cout << "笑顔を検出しました。" << endl;
		//	circle(frame, detector.getSmileCenterPoint(), 30, Scalar(0, 0, 255), 2, LINE_4);
		//}
		/*Point center_of_graph(graph.cols / 2, graph.rows / 2);
		circle(graph, center_of_graph, 5, Scalar(0, 100, 0), -1, 8, 0);
		line(graph, Point(0, 0), center_of_graph, Scalar(1, 1, 1), 2, LINE_AA, 0);
		imshow("graph", graph);*/

		kgraph.showGraph(graph);

		imshow(WINDOW_NAME, frame);
		
		if (cvWaitKey(1) >= 0) {
			break; // 1msキー入力待ち
		}
	}
	fout.close();
	cout << "ファイルをクローズしました。" << endl;

	ofstream func_out("test_function.csv");
	if (func_out.fail()) {
		std::cerr << "ファイルを開けませんでした(笑)" << endl;
	}

	double y = 0;
	int x = 0;
	const double a = 0.5;
	const double b = -2;

	for (int i = 0; i < 1000; i++) {
		x++;
		y = a * x + b;
		func_out << y << endl;
	}

	func_out.close();
    return 0;
}
// 処理終了