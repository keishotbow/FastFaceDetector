#include "stdafx.h"
#include "KGraph.h"
#define _USE_MATH_DEFINES
#include <math.h>

// �R���X�g���N�^
KGraph::KGraph(Mat & frame)
{
	this->initGraphProperty(frame); // �O���t������
	
	m_fs.open("data.dat", ios::out);
	if (m_fs.fail()) {
		cout << "�t�@�C�����I�[�v���o���܂���ł����B" << endl;
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

// �O���t������������
void KGraph::initGraphProperty(Mat & frame)
{
	//cout << "constructer is called" << endl;
	
	// ���S�_�ݒ�
	this->m_center = Point(frame.cols / 2, frame.rows / 2);

	// �l���̓_�ݒ�
	this->m_upperLeft = Point(frame.cols / 10, frame.rows / 10);
	this->m_upperRight = Point(frame.cols * 9 / 10, frame.rows / 10);
	this->m_lowerRight = Point(frame.cols * 9 / 10, frame.rows * 9 / 10);
	this->m_lowerLeft = Point(frame.cols / 10, frame.rows * 9 / 10);

	this->m_valuePoint.x = m_upperLeft.x; // �ŏ��͍��[�����

	// �O���t���̍쐬
	rectangle(frame, this->m_upperLeft, this->m_lowerRight, WHITE, -1, 0); // �O���t�̈攒�œh��Ԃ�
	line(frame, this->m_upperLeft, this->m_upperRight, BLACK, 1, 8, 0); // �O���t���
	line(frame, this->m_upperRight, this->m_lowerRight, BLACK, 1, 8, 0); // �O���t�E��
	line(frame, this->m_lowerRight, this->m_lowerLeft, BLACK, 1, 8, 0); // �O���t����
	line(frame, this->m_lowerLeft, this->m_upperLeft, BLACK, 1, 8, 0); // �O���t����

	// �O���t��̌��_����0�ʒu
	m_zero_pos = m_lowerLeft - Point(5, -15);

	// x���̒������擾
	m_x_axis_length = (frame.cols * 9 / 10) - (frame.cols / 10);
	for (int i = 0; i <= this->INTERVAL; i++) {
		m_XAxisPosVec.push_back(m_zero_pos + Point(m_x_axis_length / 10 * i, 0));
		//putText(frame, to_string(i), m_XAxisPosVec.at(i), FONT_HERSHEY_SIMPLEX, 0.3, BLACK, 1, LINE_AA);
	} 

}
double radian = 0.0;
double fourie = 0.0;
// �O���t��`�悷��
void KGraph::showGraph(Mat & graphFrame) {
	m_buffFrame = graphFrame.clone();

	if (this->is_thread_start == false) { // ��񂾂����s
		namedWindow(GRAPH_NAME, CV_WINDOW_AUTOSIZE); // �E�B���h�E�ݒ�A�\��
		this_thread::sleep_for(seconds(3));
		this->startThread();		
		this->is_thread_start = true; // �ȍ~false�ɂȂ�Ȃ�
	}
	
	//this->storeXYToVector(m_buffFrame, 100, 50);
	radian += 0.05;
	auto stepFunction = 0.0;
	for (int i = 1; i < 30; i += 2) {
		stepFunction += sin(i*radian) / i;
	}
	//m_valuePoint.y = (int)(m_center.y + 100 * -(sin(radian) + sin(3*radian)/3 + sin(5*radian)/5)); // �㉺�����̓���
	m_valuePoint.y = (int)(m_center.y + 100 * -(stepFunction)); // �㉺�����̓���
	
	if (m_valuePoint.x < m_upperRight.x - 100) { // �O���t�̈�E�[�ɓ��B���ĂȂ��ꍇ
		m_valuePoint.x += 2; // �_���E�ɂ��炵�Ă���(int�^�ɃL���X�g�����)
		circle(m_buffFrame, m_valuePoint, 2, GREEN, 1, LINE_AA, 0);
		m_valuePointVector.push_back(m_valuePoint);
		cout << m_valuePointVector.size() << ", ";
		if (m_valuePointVector.size() >= 2) {
			for (int i = 0; i < m_valuePointVector.size() - 1; i++) {
				line(m_buffFrame, m_valuePointVector.at(i), m_valuePointVector.at(i + 1), RED, 1, LINE_AA, 0);
			}
		}
	}
	else { // ���B�����ꍇ
		circle(m_buffFrame, m_valuePoint, 2, GREEN, 1, LINE_AA, 0);
		if (m_valuePointVector.size() >= 2) {
			for (int i = 0; i < m_valuePointVector.size() - 1; i++) {
				line(m_buffFrame, m_valuePointVector.at(i), m_valuePointVector.at(i + 1), RED, 1, LINE_AA, 0);
			}
		}
		cout << m_valuePointVector.size() << ", ";
	}

	this->updateGraph(m_buffFrame); // �O���t���X�V
	imshow(GRAPH_NAME, m_buffFrame);
}

// �O���t���ĕ`�悷��
void KGraph::updateGraph(Mat & graphFrame) {

	if (m_isThread == true) { // �X���b�h����̌Ăяo���̏ꍇ(�X���b�h�̊Ԋu���Ăяo��)
		if ((int)m_total_time >= 3) {
			// ���Ԏ������̎��Ԉʒu
			int i = 0;
			for (i = 0; i <= this->INTERVAL; i++) {
				//m_XAxisPosVec.push_back(m_zero_pos + Point(m_x_axis_length / 10 * i, 0));
				//putText(m_buffFrame, to_string(i), m_XAxisPosVec.at(i), FONT_HERSHEY_SIMPLEX, 0.3, BLACK, 1, LINE_AA);
			}
			//this->storeXYToVector(m_buffFrame, 100 + i, 200 + i);
			//circle(m_buffFrame, m_XYPositions.at(0), 2, BLACK, 1, LINE_AA, 0);
			cout << "called" << endl;
		}
		m_isThread = false; // �X���b�h����̌Ăяo����False�ɖ߂��Ă���
		return;
	}
	return;
}

// �v���O�����̎��s���Ԃ��v������(chrono���p)
void KGraph::startThread() {
	
	this->th1 = thread([this] { // �����_��	
		while (true) {
			auto start = high_resolution_clock::now(); //�v���J�n
			this_thread::sleep_for(seconds(1)); //�e�X�g:1�b��~
			auto end = high_resolution_clock::now(); //�v���I���
			auto duration = duration_cast<seconds>(end - start).count();
			m_total_time += duration;
			cout << m_total_time << " dur";
			m_isThread = true;
			updateGraph(m_buffFrame);
		}
	});
}

// �f�X�g���N�^
KGraph::~KGraph()
{
	cout << "�f�X�g���N�^���Ă΂ꂽ��\n";
	m_fs.close();
	this->th1.detach();
}