#include "stdafx.h"
#include "KGraph.h"
#define _USE_MATH_DEFINES
#include <math.h>

// コンストラクタ
KGraph::KGraph(Mat & frame)
{
	this->initGraphProperty(frame); // グラフ初期化
	
	m_fs.open("data.dat", ios::out);
	if (m_fs.fail()) {
		cout << "ファイルをオープン出来ませんでした。" << endl;
		EXIT_FAILURE;
	}
	m_fs << 123 << endl;
	//startGnuPlot();
	//startFourie();
	
	string buff;
	while (m_fs && getline(m_fs, buff)) {
		cout << buff << endl;
	}
}

// グラフを初期化する
void KGraph::initGraphProperty(Mat & frame)
{
	//cout << "constructer is called" << endl;
	
	// 中心点設定
	this->m_center = Point(frame.cols / 2, frame.rows / 2);

	// 四隅の点設定
	this->m_upperLeft = Point(frame.cols / 10, frame.rows / 10);
	this->m_upperRight = Point(frame.cols * 9 / 10, frame.rows / 10);
	this->m_lowerRight = Point(frame.cols * 9 / 10, frame.rows * 9 / 10);
	this->m_lowerLeft = Point(frame.cols / 10, frame.rows * 9 / 10);

	this->m_valuePoint.x = m_upperLeft.x; // 最初は左端を入手

	// グラフ軸の作成
	rectangle(frame, this->m_upperLeft, this->m_lowerRight, WHITE, -1, 0); // グラフ領域白で塗りつぶし
	line(frame, this->m_upperLeft, this->m_upperRight, BLACK, 1, 8, 0); // グラフ上辺
	line(frame, this->m_upperRight, this->m_lowerRight, BLACK, 1, 8, 0); // グラフ右線
	line(frame, this->m_lowerRight, this->m_lowerLeft, BLACK, 1, 8, 0); // グラフ下線
	line(frame, this->m_lowerLeft, this->m_upperLeft, BLACK, 1, 8, 0); // グラフ左線

	// グラフ上の原点下の0位置
	m_zero_pos = m_lowerLeft - Point(5, -15);

	// x軸の長さを取得
	m_x_axis_length = (frame.cols * 9 / 10) - (frame.cols / 10);
	for (int i = 0; i <= this->INTERVAL; i++) {
		m_XAxisPosVec.push_back(m_zero_pos + Point(m_x_axis_length / 10 * i, 0));
		//putText(frame, to_string(i), m_XAxisPosVec.at(i), FONT_HERSHEY_SIMPLEX, 0.3, BLACK, 1, LINE_AA);
	} 

}
double radian = 0.0;
double fourie = 0.0;
// グラフを描画する
void KGraph::showGraph(Mat & graphFrame) {
	m_buffFrame = graphFrame.clone();

	if (this->is_thread_start == false) { // 一回だけ実行
		namedWindow(GRAPH_NAME, CV_WINDOW_AUTOSIZE); // ウィンドウ設定、表示
		this_thread::sleep_for(seconds(3));
		this->startThread();		
		this->is_thread_start = true; // 以降falseにならない
	}
	
	//this->storeXYToVector(m_buffFrame, 100, 50);
	radian += 0.05;
	auto stepFunction = 0.0;
	for (int i = 1; i < 30; i += 2) {
		stepFunction += sin(i*radian) / i;
	}
	//m_valuePoint.y = (int)(m_center.y + 100 * -(sin(radian) + sin(3*radian)/3 + sin(5*radian)/5)); // 上下方向の動き
	m_valuePoint.y = (int)(m_center.y + 100 * -(stepFunction)); // 上下方向の動き
	
	if (m_valuePoint.x < m_upperRight.x - 100) { // グラフ領域右端に到達してない場合
		m_valuePoint.x += 2; // 点を右にずらしていく(int型にキャストされる)
		circle(m_buffFrame, m_valuePoint, 2, GREEN, 1, LINE_AA, 0);
		m_valuePointVector.push_back(m_valuePoint);
		cout << m_valuePointVector.size() << ", ";
		if (m_valuePointVector.size() >= 2) {
			for (int i = 0; i < m_valuePointVector.size() - 1; i++) {
				line(m_buffFrame, m_valuePointVector.at(i), m_valuePointVector.at(i + 1), RED, 1, LINE_AA, 0);
			}
		}
	}
	else { // 到達した場合
		circle(m_buffFrame, m_valuePoint, 2, GREEN, 1, LINE_AA, 0);
		if (m_valuePointVector.size() >= 2) {
			for (int i = 0; i < m_valuePointVector.size() - 1; i++) {
				line(m_buffFrame, m_valuePointVector.at(i), m_valuePointVector.at(i + 1), RED, 1, LINE_AA, 0);
			}
		}
		cout << m_valuePointVector.size() << ", ";
	}

	this->updateGraph(m_buffFrame); // グラフを更新
	imshow(GRAPH_NAME, m_buffFrame);
}

// グラフを再描画する
void KGraph::updateGraph(Mat & graphFrame) {

	if (m_isThread == true) { // スレッドからの呼び出しの場合(スレッドの間隔分呼び出し)
		if ((int)m_total_time >= 3) {
			// 時間軸方向の時間位置
			int i = 0;
			for (i = 0; i <= this->INTERVAL; i++) {
				//m_XAxisPosVec.push_back(m_zero_pos + Point(m_x_axis_length / 10 * i, 0));
				//putText(m_buffFrame, to_string(i), m_XAxisPosVec.at(i), FONT_HERSHEY_SIMPLEX, 0.3, BLACK, 1, LINE_AA);
			}
			//this->storeXYToVector(m_buffFrame, 100 + i, 200 + i);
			//circle(m_buffFrame, m_XYPositions.at(0), 2, BLACK, 1, LINE_AA, 0);
			cout << "called" << endl;
		}
		m_isThread = false; // スレッドからの呼び出しをFalseに戻しておく
		return;
	}
	return;
}

// プログラムの実行時間を計測する(chrono利用)
void KGraph::startThread() {
	
	this->th1 = thread([this] { // ラムダ式	
		while (true) {
			auto start = high_resolution_clock::now(); //計測開始
			this_thread::sleep_for(seconds(1)); //テスト:1秒停止
			auto end = high_resolution_clock::now(); //計測終わり
			auto duration = duration_cast<seconds>(end - start).count();
			m_total_time += duration;
			cout << m_total_time << " dur";
			m_isThread = true;
			updateGraph(m_buffFrame);
		}
	});
}

// デストラクタ
KGraph::~KGraph()
{
	cout << "デストラクタが呼ばれたよ\n";
	m_fs.close();
	this->th1.detach();
}