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

	const int WIDTH = 600, HEIGHT = 800; // �t���[�����A����
	Mat m_frame; // �\���p�t���[��
	Mat m_buffFrame; // �o�b�t�@�t���[��

	Point m_center; // �O���t�̒��S�_
	Point m_upperLeft; // ����
	Point m_upperRight; // �E��
	Point m_lowerRight; // �E��
	Point m_lowerLeft; // ����
	Point m_zero_pos; // �O���t���0�̈ʒu

	Point m_valuePoint; // �l���O���t�Ƀv���b�g���鎞�̓_

	vector<Point> m_valuePointVector; // �O���t�Ƀv���b�g�������W���i�[����
	vector<Point> m_XAxisPosVec; // x���W�̎��Ԏ��̓_���o���Ă����p
	int m_x_axis_length; // x���̒���(x���Ɏ��Ԃ𓙊Ԋu�ɕ`�悷��̂Ɏg��)
	const int INTERVAL = 10; // x���̎��ԃv���b�g�̊Ԋu

	const Point m_graph_title_point; // �O���t�^�C�g���̈ʒu
	
	Size m_size; // �O���t�̃T�C�Y
	Rect m_rect;// �O���t�̈�
	bool is_thread_start = false; // �X���b�h���J�n�������ǂ���

	time_point<system_clock> m_start, m_end; // �o�ߎ��ԑ���p
	
	long long m_total_time = 0LL; // ���v�v������

	const Scalar WHITE = Scalar(255, 255, 255); // ��
	const Scalar BLACK = Scalar(0, 0, 0); // ��
	const Scalar BLUE = Scalar(255, 0, 0); // ��
	const Scalar GREEN = Scalar(0, 255, 0); // ��
	const Scalar RED = Scalar(0, 0, 255); // ��

	bool m_isThread = false; // �X���b�h���ǂ����m�F�p
	const String GRAPH_NAME = "Graph";

	fstream m_fs; // �O���t�f�[�^�L�^�p�X�g���[��
	
	void initGraphProperty(Mat & frame);

public:
	KGraph(Mat & m_frame); // �R���X�g���N�^
	
	thread th1;
	
	void showGraph(Mat & graphFrame); // �O���t��\������
	void updateGraph(Mat & graphFrame); // �O���t���ĕ`�悷��
	void startThread(); // �X���b�h�X�^�[�g
	~KGraph();
};

