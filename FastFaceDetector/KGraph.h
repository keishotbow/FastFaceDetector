#pragma once
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

using namespace std;
using namespace cv;
using namespace chrono;

class KGraph
{
private:

	const int WIDTH = 600, HEIGHT = 800; // フレーム幅、高さ
	Mat m_frame; // 表示用フレーム
	Mat m_buffFrame; // バッファフレーム

	Point m_center; // グラフの中心点
	Point m_upperLeft; // 左上
	Point m_upperRight; // 右上
	Point m_lowerRight; // 右下
	Point m_lowerLeft; // 左下
	Point m_zero_pos; // グラフ上の0の位置

	Point m_valuePoint; // 値をグラフにプロットする時の点

	vector<Point> m_valuePointVector; // グラフにプロットした座標を格納する
	vector<Point> m_XAxisPosVec; // x座標の時間軸の点を覚えておく用
	int m_x_axis_length; // x軸の長さ(x軸に時間を等間隔に描画するのに使う)
	const int INTERVAL = 10; // x軸の時間プロットの間隔

	const Point m_graph_title_point; // グラフタイトルの位置
	
	Size m_size; // グラフのサイズ
	Rect m_rect;// グラフ領域
	bool is_thread_start = false; // スレッドが開始したかどうか

	time_point<system_clock> m_start, m_end; // 経過時間測定用
	
	long long m_total_time = 0LL; // 合計計測時間

	const Scalar WHITE = Scalar(255, 255, 255); // 白
	const Scalar BLACK = Scalar(0, 0, 0); // 黒
	const Scalar BLUE = Scalar(255, 0, 0); // 青
	const Scalar GREEN = Scalar(0, 255, 0); // 緑
	const Scalar RED = Scalar(0, 0, 255); // 赤

	bool m_isThread = false; // スレッドかどうか確認用
	const String GRAPH_NAME = "Graph";

	fstream m_fs; // グラフデータ記録用ストリーム
	
	void initGraphProperty(Mat & frame);

public:
	KGraph(Mat & m_frame); // コンストラクタ
	
	thread th1;
	
	void showGraph(Mat & graphFrame); // グラフを表示する
	void updateGraph(Mat & graphFrame); // グラフを再描画する
	void startThread(); // スレッドスタート
	~KGraph();
};

