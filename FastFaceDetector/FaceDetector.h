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
	FaceDetector(const string CASCADE_FILE_NAME, VideoCapture & cap); // �R���X�g���N�^
	FaceDetector(const string CASCADE_FILE_NAME, const string SMILE_CASCADE_FILE_NAME, VideoCapture &cap); // �R���X�g���N�^
	~FaceDetector(); // �f�X�g���N�^

	void setFaceCascade(const string CASCADE_FILE_NAME); // �J�X�P�[�h�t�@�C�����Z�b�g����
	void setSmileCascade(const string SMILE_CASCADE_FILE_NAME); // �X�}�C���J�X�P�[�h�t�@�C�����Z�b�g����
	void setVideoCapture(VideoCapture &cap); // �J�����f�����Z�b�g����

	bool isFaceFound(); // �炪�����������ǂ���
	bool isSmileFound(); // �Ί炪�����������ǂ���

	void operator>>(Mat & frame); // �I�u�W�F�N�g�������ɃJ�����f����cascade file�n��

	Rect getFaceRect(); // ��̈�����o����

	Point getFaceCenterPoint(); // ��̈�̒��S�_���擾
	Point getSmileCenterPoint(); // �Ί�̈�̒��S�_���擾

private:

	CascadeClassifier* m_faceCascade = nullptr; // ��̃J�X�P�[�h
	CascadeClassifier* m_smileCascade = nullptr; // �Ί�̃J�X�P�[�h

	VideoCapture* m_videoCapture = nullptr; // �r�f�I�L���v�`���[
	
	Point  m_facePosition; // ���o���ꂽ��̒��S���W
	Point  m_smilePosition; // ���o���ꂽ�Ί�̒��S���W

	double m_scale; // �擾�t���[���̃X�P�[��
	int    m_resizedWidth = 320; // ���T�C�Y��̕�
	bool   m_foundFace = false; // �炪�����������ǂ����B�����l��false
	bool   m_foundSmile = false; // �Ί炪�����������ǂ����B�����l��false

	vector<Rect> m_allFaces; // HaarLike���o�ɂ�茟�o���ꂽ�����̊���i�[����vector�z��
	vector<Rect> m_allSmiles; // HaarLike���o�ɂ�茟�o���ꂽ�����̏Ί���i�[����vector�z��
	
	Rect m_trackedFace; // ���o���ꂽ��
	Rect m_trackedSmile; // ���o���ꂽ�Ί�
	Rect m_faceRoi; // ��̈��ROI
	Mat  m_faceTemplate; // ���o������̕����̈�̃e���v���[�g�摜

	bool   m_templateMatchingRunning = false; // Template Matching�����삵�Ă��邩�B�����l��false
	int64  m_templateMatchingStartTime = 0; // Template Matching�J�n����(�����l��0�b)
	int64  m_templateMatchingCurrentTime = 0; // Template Matching�̓��쎞��
 	double m_templateMathingMaxDuration = 3; // Template Matching�̍ő哮�쎞��(�C�ӂɕύX��)
	Mat    m_matchingResult; // �e���v���}�b�`���O�̌��ʂ��i�[����

	Point getFacePositionAndDetect(Mat & frame); // ��̌��o���s��
		void detectFaceAllSizes(const Mat & resizedFrame); // ���T�C�Y���ꂽ�t���[�����󂯎��AHaarLike���o����
			Rect         biggestFace(vector<Rect> & allFaces) const; // HaarLike�Ō��o���ꂽ��̈�̓��A�ł��傫����̈��Ԃ�
			Mat          getFaceTemplate(const Mat & resizedFrame, Rect trackedFace); // ���o���ꂽ��̕����̈�(�e���v���[�g�摜)�𓾂�
			Rect         doubleRectSize(const Rect & trackedFace, const Rect & resizedFrame) const; // ROI�v�Z
			inline Point centerOfRect(const Rect & trackedFace); // ���o���ꂽ��̒��S�v�Z����

		void detectFaceAroundRoi(const Mat & resizedFrame); // ROI���Ŋ�����o����
		void detectFacesTemplateMatching(Mat & resizedFrame); // Template Matching�Ŋ�����o����
		void detectSmileAroundRoi(const Mat & resizedFrame); // �Ί猟�o�p

	template<typename Function>
	auto measureProcessTime(Function func);
};
