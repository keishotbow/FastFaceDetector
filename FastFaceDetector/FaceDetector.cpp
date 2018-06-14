#include "stdafx.h"
#include "FaceDetector.h"

// �R���X�g���N�^
FaceDetector::FaceDetector(const string CASCADE_FILE_NAME, const string SMILE_CASCADE_FILE_NAME, VideoCapture &cap)
{
	setFaceCascade(CASCADE_FILE_NAME); // �J�X�P�[�h�t�@�C�����Z�b�g -> m_faceCascade
	setSmileCascade(SMILE_CASCADE_FILE_NAME); // �Ί�̃J�X�P�[�h�t�@�C�����Z�b�g -> m_smileCascade
	setVideoCapture(cap); // �J�����f�����Z�b�g -> m_videoCapture
	ofs.open("BGRave.csv");
	if (ofs.fail()) {
		exit(1);
	}
}

// �R���X�g���N�^
FaceDetector::FaceDetector(const string CASCADE_FILE_NAME, VideoCapture &cap)
{
	ofs.open("BGRave.csv");
	setFaceCascade(CASCADE_FILE_NAME); // �J�X�P�[�h�t�@�C�����Z�b�g -> m_faceCascade
	setVideoCapture(cap); // �J�����f�����Z�b�g -> m_videoCapture
}

// �f�X�g���N�^
FaceDetector::~FaceDetector()
{
	if (m_faceCascade != nullptr) {
		delete m_faceCascade; // �|�C���^���
	}

	if (m_smileCascade != nullptr) {
		delete m_smileCascade; // �|�C���^���
	}
	ofs.close();
}

// Cascade File���Z�b�g����(�R���X�g���N�^����I�u�W�F�N�g�������ɌĂяo�����)
void FaceDetector::setFaceCascade(const string CASCADE_FILE_NAME)
{
	if (m_faceCascade == nullptr) {
		//cout << "�Ί�J�X�P�[�h��������" << endl;
		m_faceCascade = new CascadeClassifier(CASCADE_FILE_NAME);
	}
	else {
		//cout << "�Ί�J�X�P�[�h���[�h����" << endl;
		m_faceCascade->load(CASCADE_FILE_NAME);
	}

	if (m_faceCascade->empty()) {
		cerr << "�J�X�P�[�h�t�@�C����������܂���B�t�@�C���p�X�͐�΃p�X�Ŏw�肵�Ă��������B\n";
		exit(1);
	}
}

// �Ί�̃J�X�P�[�h�t�@�C�����Z�b�g����(�R���X�g���N����I�u�W�F�N�g�������ɌĂяo�����)
void FaceDetector::setSmileCascade(const string SMILE_CASCADE_FILE_NAME)
{
	if (m_smileCascade == NULL) {
		m_smileCascade = new CascadeClassifier(SMILE_CASCADE_FILE_NAME);
	}
	else {
		m_smileCascade->load(SMILE_CASCADE_FILE_NAME);
	}

	if (m_smileCascade->empty()) {
		cerr << "�Ί猟�o�̃J�X�P�[�h�t�@�C����������܂���B�t�@�C���p�X�͐�΃p�X�Ŏw�肵�Ă��������B\n";
		exit(1);
	}
}

// �J�����f�����Z�b�g����(�R���X�g���N�^����I�u�W�F�N�g�������ɌĂяo�����)
void FaceDetector::setVideoCapture(VideoCapture & cap)
{
	m_videoCapture = &cap;
}

// �炪�������true��Ԃ��B����ȊO��false
bool FaceDetector::isFaceFound() {
	return m_foundFace;
}

// �Ί炪���o������True, ����ȊO��False��Ԃ�
bool FaceDetector::isSmileFound()
{
	return m_foundSmile;
}

// �t���[�����󂯎��A������o���Ă����̈ʒu��Ԃ�
Point FaceDetector::getFacePositionAndDetect(Mat &frame)
{
	*m_videoCapture >> frame;

	// �擾�t���[�����_�E���X�P�[�����AresizedFrameSize�Ɋi�[����
	m_scale = (double)min(m_resizedWidth, frame.cols) / frame.cols;
	Size resizedFrameSize = Size((int)(m_scale*frame.cols), (int)(m_scale*frame.rows));
	Mat resizedFrame(resizedFrameSize, CV_8UC3); // ���T�C�Y�����摜
	resize(frame, resizedFrame, resizedFrameSize);

	if (m_foundFace == false) { // �炪������Ȃ���΃t���[���S�̂ɓn����HaarLike���o
		cout << "Not Found. \n";
		detectFaceAllSizes(resizedFrame);
	}
	else { // �炪���������ꍇ�AROI���ł̂�HaarLike���o����
		cout << "\tFound! \n";
		detectFaceAroundRoi(resizedFrame); // ROI���Ŋ猟�o

		extractSkinColor(frame); // ROI���荞��Ŕ��F���o

		if (m_templateMatchingRunning == true) { // Template Matching�𓮍삳����
			detectFacesTemplateMatching(resizedFrame);
		}

	}

	return m_facePosition;
}

// ���T�C�Y���ꂽ�t���[�����󂯎��AHaarLike���o����
void FaceDetector::detectFaceAllSizes(const Mat &resizedFrame)
{
	m_faceCascade->detectMultiScale(resizedFrame, m_allFaces, 1.1, 3, 0
		, Size(resizedFrame.rows / 5, resizedFrame.rows / 5)		// ���o�̍ŏ��T�C�Y
		, Size(resizedFrame.rows * 2 / 3, resizedFrame.rows * 2 / 3));  // ���o�̍ő�T�C�Y

	if (m_allFaces.size() == 0) return;

	m_foundFace = true; // detectMultiScale�ɂ��炪��������
	m_trackedFace = biggestFace(m_allFaces); // ���o������̓��ő�T�C�Y�̊��Ԃ�
	m_faceTemplate = getFaceTemplate(resizedFrame, m_trackedFace); // ���o���ꂽ��̕����̈�(�e���v���[�g�摜)�𓾂�	
	m_faceRoi = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows)); // ROI�v�Z
	m_facePosition = centerOfRect(m_trackedFace); // ���o���ꂽ��̒��S���v�Z����
}

// HaarLike�Ō��o���ꂽ��̈�̓��A�ł��傫����̈��Ԃ�
Rect FaceDetector::biggestFace(vector<Rect> & allFaces) const
{
	assert(!allFaces.empty());
	Rect *biggest = &allFaces[0]; // �炪���߂��Ă���z��̐擪���w���|�C���^
	
	for (auto &face : allFaces) {
		if (face.area() < biggest->area()) {
			biggest = &face;
		}
	}

	return *biggest;
}

// ���o���ꂽ��̕����̈�(�e���v���[�g�摜)�𓾂�
Mat FaceDetector::getFaceTemplate(const Mat & resizedFrame, Rect trackedFace)
{
	trackedFace.x = trackedFace.x + trackedFace.width / 4;
	trackedFace.y = trackedFace.y + trackedFace.height / 4;
	trackedFace.width = trackedFace.width / 2; // ��2����1
	trackedFace.height = trackedFace.height / 2; // ����2����1

	Mat faceTemplate = resizedFrame(trackedFace);
	return faceTemplate;
}

// ���o���ꂽ���ROI�v�Z
Rect FaceDetector::doubleRectSize(const Rect & trackedFace, const Rect & resizedFrame) const
{
	Rect roiRect;

	// �T�C�Y���Q�{�ɂ���
	roiRect.width = trackedFace.width * 2;
	roiRect.height = trackedFace.height * 2;

	// ���S�v�Z
	roiRect.x = trackedFace.x - trackedFace.width / 2;
	roiRect.y = trackedFace.y - trackedFace.height / 2;

	// �t���[���[�̏���
	if (roiRect.x < resizedFrame.x) {
		roiRect.width += roiRect.x;
		roiRect.x = resizedFrame.x;
	}

	if (roiRect.y < resizedFrame.y) {
		roiRect.height += roiRect.y;
		roiRect.y = resizedFrame.y;
	}

	if (roiRect.x + roiRect.width > resizedFrame.width) {
		roiRect.width = resizedFrame.width - roiRect.x;
	}
	if (roiRect.y + roiRect.height > resizedFrame.height) {
		roiRect.height = resizedFrame.height - roiRect.y;
	}

	return roiRect;
}

// ���o���ꂽ��̒��S�v�Z����
inline Point FaceDetector::centerOfRect(const Rect & trackedFace) {
	return Point(trackedFace.x + trackedFace.width / 2, trackedFace.y + trackedFace.height / 2);
}

// ROI���Ŋ�����o����
void FaceDetector::detectFaceAroundRoi(const Mat &resizedFrame)
{
	m_faceCascade->detectMultiScale(resizedFrame(m_faceRoi), m_allFaces, 1.1, 3, 0
		, Size(m_trackedFace.width * 8 / 10, m_trackedFace.height * 8 / 10)
		, Size(m_trackedFace.width * 12 / 10, m_trackedFace.height * 12 / 10));

	if (m_allFaces.empty()) {
		// �e���v���[�g�}�b�`���O���X�^�[�g������
		m_templateMatchingRunning = true;
		if (m_templateMatchingStartTime == 0) {
			m_templateMatchingStartTime = getTickCount();
		}
		return;
	}

	// �e���v���[�g�}�b�`���O�����삵�Ă���Β�~���A�J�n���Ԃ����Z�b�g����
	m_templateMatchingRunning = false;
	m_templateMatchingCurrentTime = m_templateMatchingStartTime = 0;

	// ��̈��������擾(detectFaceAllSize()���Ɠ��l�̏���)
	m_trackedFace = biggestFace(m_allFaces);

	// ROI�I�t�Z�b�g�����Z
	m_trackedFace.x = m_trackedFace.x + m_faceRoi.x;
	m_trackedFace.y = m_trackedFace.y + m_faceRoi.y;

	// ��̃e���v���摜���擾
	m_faceTemplate = getFaceTemplate(resizedFrame, m_trackedFace);

	// ROI�Čv�Z
	m_faceRoi = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows));

	// �琄��ʒu�X�V
	m_facePosition = centerOfRect(m_trackedFace);
}

// HaarLike�ɂ��ROI���ŏΊ猟�o���s���B
void FaceDetector::detectSmileAroundRoi(const Mat & resizedFrame)
{
	// �Ί猟�o
	m_smileCascade->detectMultiScale(resizedFrame(m_faceRoi), m_allSmiles, 1.1, 3, 0
		, Size(m_trackedFace.width / 2, m_trackedFace.height / 2));

	if (m_allSmiles.empty()) {
		cout << "�Ί炪��ł�" << endl;
		m_foundSmile = false; // �Ί炪������Ȃ���
		return;
	}

	cout << "�Ί炪������܂����B" << endl;
	m_foundSmile = true; // �Ί炪��������

	// �Ί�̈��������擾
	m_trackedSmile = biggestFace(m_allSmiles);
	
	// Roi�I�t�Z�b�g�v�Z
	//m_trackedSmile.x = m_trackedSmile.x + m_faceRoi.width;
	//m_trackedSmile.y = m_trackedSmile.y + m_faceRoi.height;

	m_smilePosition = centerOfRect(m_trackedSmile);
}

// �t���[���̉�f�l�̕��ϒl��Ԃ�
Scalar FaceDetector::calcBGRAverage( Mat & roi_image, const Mat & mask_image){
	for (int i = 0; i < roi_image.total(); i++) {
		if (mask_image.data[i] == 255) {
			roi_image.data[i] = 0;
		}
	}
	return mean(mask_image);
}

// ROI�摜���󂯎���Ĕ��F���o���s��
Scalar FaceDetector::extractSkinColor(const Mat & frame)
{
	Mat roi_image = frame(getFaceRect()).clone();
	Mat hsv_image, mask_image;
	
	cvtColor(roi_image, hsv_image, COLOR_BGR2HSV);

	Scalar scalar_min = Scalar(0, 58, 88); // ���F�Œ�l
	Scalar scalar_max = Scalar(25, 173, 229); // ���F�ō��l

	inRange(hsv_image, scalar_min, scalar_max, mask_image); // ���F���o
	//cvtColor(mask_image, mask_image, CV_GRAY2BGR);

	unsigned int count = 0;
	auto start = high_resolution_clock::now();
	int blue = 0, green = 0, red = 0;
	double blue_ave, green_ave, red_ave;

	for (int y = 0; y < roi_image.rows; y++) {
		Vec3b *first_row_ptr = roi_image.ptr<Vec3b>(y); // �摜�̍s�̐擪�̃|�C���^�擾
		for (int x = 0; x < roi_image.cols; x++) {
			Vec3b bgr = first_row_ptr[x];
			blue  += bgr[0];
			green += bgr[1];
			red   += bgr[2];
			//Vec3b bgr = move(first_row_ptr[x]);
			/*if (*mask_image.ptr<Vec3b>(x) == Vec3b(255, 255, 255)) {
				*mask_image.ptr<Vec3b>(x) = Vec3b(100, 100, 100);
			}*/
			//cout << bgr[0] << ", " << bgr[1] << ", " << bgr[2] << " ";
			count++;
		}
	}

	blue_ave  = blue / count;
	green_ave = green / count;
	red_ave   = red / count;

	cout << "B_ave: " << blue_ave  << ", ";
	cout << "G_ave: " << green_ave << ", ";
	cout << "R_ave: " << red_ave   << " ";

	ofs << blue_ave << "," << green_ave << "," << red_ave << endl;

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start).count();
	cout << duration << " ms ";
	
	imshow("mask", mask_image);
	imshow("roi", roi_image);
	return Scalar();
}

// Template Matching�Ŋ�����o����
void FaceDetector::detectFacesTemplateMatching(Mat &resizedFrame)
{
	m_templateMatchingCurrentTime = getTickCount();
	double duration = (double)(m_templateMatchingCurrentTime - m_templateMatchingStartTime) / getTickFrequency();

	if (duration > m_templateMathingMaxDuration) {
		m_foundFace = false; // �����������
		m_templateMatchingRunning = false; // �e���v���}�b�`���O��~
		m_templateMatchingStartTime = 0; // �J�n�������Z�b�g
		m_templateMatchingCurrentTime = 0; // ���쎞�ԃ��Z�b�g

		// ��ʒu���Z�b�g
		m_facePosition.x = m_facePosition.y = 0;
		m_trackedFace.x = m_trackedFace.y = m_trackedFace.width = m_trackedFace.height = 0;
		return;
	}

	// �e���v���[�g�摜���ɂ߂ď������ꍇ�e���v���[�g�}�b�`���O���f
	if (m_faceTemplate.rows * m_faceTemplate.cols == 0 ||
		m_faceTemplate.rows <= 10 || m_faceTemplate.cols <= 10)
	{
		m_foundFace = false; // �����������
		m_templateMatchingRunning = false; // �e���v���}�b�`���O��~
		m_templateMatchingStartTime = 0; // �J�n�������Z�b�g
		m_templateMatchingCurrentTime = 0; // ���쎞�ԃ��Z�b�g

		// ��ʒu���Z�b�g
		m_facePosition.x = m_facePosition.y = 0;
		m_trackedFace.x = m_trackedFace.y = m_trackedFace.width = m_trackedFace.height = 0;
		return;
	}

	// �e���v���[�g�}�b�`���O�J�n
	matchTemplate(resizedFrame(m_faceRoi), m_faceTemplate, m_matchingResult, CV_TM_SQDIFF);
	//matchTemplate(resizedFrame(m_faceRoi), m_faceTemplate, m_matchingResult, CV_TM_SQDIFF_NORMED);
	normalize(m_matchingResult, m_matchingResult, 0, 1, NORM_MINMAX, -1, Mat());
	double min, max;
	Point minLoc, maxLoc;
	minMaxLoc(m_matchingResult, &min, &max, &minLoc, &maxLoc);

	// roi�I�t�Z�b�g�����Z
	minLoc.x = minLoc.x + m_faceRoi.x;
	minLoc.y = minLoc.y + m_faceRoi.y;

	// ���o��������擾
	m_trackedFace = Rect(minLoc.x, minLoc.y, m_faceTemplate.cols, m_faceTemplate.rows);
	m_trackedFace = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows));

	// �V���Ɋ�̃e���v���[�g�摜���擾
	m_faceTemplate = getFaceTemplate(resizedFrame, m_trackedFace);

	// ROI�v�Z
	m_faceRoi = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows));

	// ��ʒu�X�V
	m_facePosition = centerOfRect(m_trackedFace);
}

// ��̈���i�[����
Rect FaceDetector::getFaceRect()
{
	Rect faceRect = m_trackedFace;
	faceRect.x = faceRect.x / m_scale;
	faceRect.y = faceRect.y / m_scale;
	faceRect.width = faceRect.width / m_scale;
	faceRect.height = faceRect.height / m_scale;

	return faceRect;
}

// ��̈�̒��̍X�ɏ����Ȕ��̈���Z�o
Rect FaceDetector::getFaceROIRect() {
	Rect faceRoiRect = getFaceRect();
	faceRoiRect.x = faceRoiRect.x * 1.2;
	faceRoiRect.y = faceRoiRect.y * 1.2;
	faceRoiRect.width = faceRoiRect.width * 0.5;
	faceRoiRect.height = faceRoiRect.height * 0.5;
	return faceRoiRect;
}

// ��̈�̒��S�_���擾
Point FaceDetector::getFaceCenterPoint()
{
	Point facePos;
	facePos.x = (int)(m_facePosition.x / m_scale);
	facePos.y = (int)(m_facePosition.y / m_scale);
	return facePos;
}

// �Ί�̈�̒��S�_���擾
Point FaceDetector::getSmileCenterPoint()
{
	Point smilePos;
	smilePos.x = (int)(m_smilePosition.x / m_scale);
	smilePos.y = (int)(m_smilePosition.y / m_scale);
	return smilePos;
}

// �֐�����荞��ŏ������Ԃ��v������
template<typename Function>
auto FaceDetector::measureProcessTime(Function func)
{
	auto start = high_resolution_clock::now();
	func();
	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start).count();
	cout << duration << " ms.\n";
	return duration;
}

// ���Z�q�̃I�[�o�[���[�h�A�I�u�W�F�N�g�������ɃJ�����f����cascade file��n��
void FaceDetector::operator>>(Mat &frame)
{
	/*measureProcessTime([this, &frame]() {
		this->getFacePositionAndDetect(frame);
	});*/
	this->getFacePositionAndDetect(frame);
}

Mat FaceDetector::getTrackedFace() {
	if (!m_trackedFace.empty()) {
	}
	Mat frame;
	return frame;
}