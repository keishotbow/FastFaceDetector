#include "stdafx.h"
#include "FaceDetector.h"

// コンストラクタ
FaceDetector::FaceDetector(const string CASCADE_FILE_NAME, const string SMILE_CASCADE_FILE_NAME, VideoCapture &cap)
{
	setFaceCascade(CASCADE_FILE_NAME); // カスケードファイルをセット -> m_faceCascade
	setSmileCascade(SMILE_CASCADE_FILE_NAME); // 笑顔のカスケードファイルをセット -> m_smileCascade
	setVideoCapture(cap); // カメラ映像をセット -> m_videoCapture
	ofs.open("BGRave.csv");
	if (ofs.fail()) {
		exit(1);
	}
}

// コンストラクタ
FaceDetector::FaceDetector(const string CASCADE_FILE_NAME, VideoCapture &cap)
{
	ofs.open("BGRave.csv");
	setFaceCascade(CASCADE_FILE_NAME); // カスケードファイルをセット -> m_faceCascade
	setVideoCapture(cap); // カメラ映像をセット -> m_videoCapture
}

// デストラクタ
FaceDetector::~FaceDetector()
{
	if (m_faceCascade != nullptr) {
		delete m_faceCascade; // ポインタ解放
	}

	if (m_smileCascade != nullptr) {
		delete m_smileCascade; // ポインタ解放
	}
	ofs.close();
}

// Cascade Fileをセットする(コンストラクタからオブジェクト生成時に呼び出される)
void FaceDetector::setFaceCascade(const string CASCADE_FILE_NAME)
{
	if (m_faceCascade == nullptr) {
		//cout << "笑顔カスケード生成完了" << endl;
		m_faceCascade = new CascadeClassifier(CASCADE_FILE_NAME);
	}
	else {
		//cout << "笑顔カスケードロード完了" << endl;
		m_faceCascade->load(CASCADE_FILE_NAME);
	}

	if (m_faceCascade->empty()) {
		cerr << "カスケードファイルが見つかりません。ファイルパスは絶対パスで指定してください。\n";
		exit(1);
	}
}

// 笑顔のカスケードファイルをセットする(コンストラクからオブジェクト生成時に呼び出される)
void FaceDetector::setSmileCascade(const string SMILE_CASCADE_FILE_NAME)
{
	if (m_smileCascade == NULL) {
		m_smileCascade = new CascadeClassifier(SMILE_CASCADE_FILE_NAME);
	}
	else {
		m_smileCascade->load(SMILE_CASCADE_FILE_NAME);
	}

	if (m_smileCascade->empty()) {
		cerr << "笑顔検出のカスケードファイルが見つかりません。ファイルパスは絶対パスで指定してください。\n";
		exit(1);
	}
}

// カメラ映像をセットする(コンストラクタからオブジェクト生成時に呼び出される)
void FaceDetector::setVideoCapture(VideoCapture & cap)
{
	m_videoCapture = &cap;
}

// 顔が見つかればtrueを返す。それ以外はfalse
bool FaceDetector::isFaceFound() {
	return m_foundFace;
}

// 笑顔が検出されればTrue, それ以外はFalseを返す
bool FaceDetector::isSmileFound()
{
	return m_foundSmile;
}

// フレームを受け取り、顔を検出してから顔の位置を返す
Point FaceDetector::getFacePositionAndDetect(Mat &frame)
{
	*m_videoCapture >> frame;

	// 取得フレームをダウンスケールし、resizedFrameSizeに格納する
	m_scale = (double)min(m_resizedWidth, frame.cols) / frame.cols;
	Size resizedFrameSize = Size((int)(m_scale*frame.cols), (int)(m_scale*frame.rows));
	Mat resizedFrame(resizedFrameSize, CV_8UC3); // リサイズした画像
	resize(frame, resizedFrame, resizedFrameSize);

	if (m_foundFace == false) { // 顔が見つからなければフレーム全体に渡ってHaarLike検出
		cout << "Not Found. \n";
		detectFaceAllSizes(resizedFrame);
	}
	else { // 顔が見つかった場合、ROI内でのみHaarLike検出する
		cout << "\tFound! \n";
		detectFaceAroundRoi(resizedFrame); // ROI内で顔検出

		extractSkinColor(frame); // ROI放り込んで肌色抽出

		if (m_templateMatchingRunning == true) { // Template Matchingを動作させる
			detectFacesTemplateMatching(resizedFrame);
		}

	}

	return m_facePosition;
}

// リサイズされたフレームを受け取り、HaarLike検出する
void FaceDetector::detectFaceAllSizes(const Mat &resizedFrame)
{
	m_faceCascade->detectMultiScale(resizedFrame, m_allFaces, 1.1, 3, 0
		, Size(resizedFrame.rows / 5, resizedFrame.rows / 5)		// 検出の最小サイズ
		, Size(resizedFrame.rows * 2 / 3, resizedFrame.rows * 2 / 3));  // 検出の最大サイズ

	if (m_allFaces.size() == 0) return;

	m_foundFace = true; // detectMultiScaleにより顔が見つかった
	m_trackedFace = biggestFace(m_allFaces); // 検出した顔の内最大サイズの顔を返す
	m_faceTemplate = getFaceTemplate(resizedFrame, m_trackedFace); // 検出された顔の部分領域(テンプレート画像)を得る	
	m_faceRoi = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows)); // ROI計算
	m_facePosition = centerOfRect(m_trackedFace); // 検出された顔の中心を計算する
}

// HaarLikeで検出された顔領域の内、最も大きい顔領域を返す
Rect FaceDetector::biggestFace(vector<Rect> & allFaces) const
{
	assert(!allFaces.empty());
	Rect *biggest = &allFaces[0]; // 顔が収められている配列の先頭を指すポインタ
	
	for (auto &face : allFaces) {
		if (face.area() < biggest->area()) {
			biggest = &face;
		}
	}

	return *biggest;
}

// 検出された顔の部分領域(テンプレート画像)を得る
Mat FaceDetector::getFaceTemplate(const Mat & resizedFrame, Rect trackedFace)
{
	trackedFace.x = trackedFace.x + trackedFace.width / 4;
	trackedFace.y = trackedFace.y + trackedFace.height / 4;
	trackedFace.width = trackedFace.width / 2; // 幅2分の1
	trackedFace.height = trackedFace.height / 2; // 高さ2分の1

	Mat faceTemplate = resizedFrame(trackedFace);
	return faceTemplate;
}

// 検出された顔のROI計算
Rect FaceDetector::doubleRectSize(const Rect & trackedFace, const Rect & resizedFrame) const
{
	Rect roiRect;

	// サイズを２倍にする
	roiRect.width = trackedFace.width * 2;
	roiRect.height = trackedFace.height * 2;

	// 中心計算
	roiRect.x = trackedFace.x - trackedFace.width / 2;
	roiRect.y = trackedFace.y - trackedFace.height / 2;

	// フレーム端の処理
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

// 検出された顔の中心計算する
inline Point FaceDetector::centerOfRect(const Rect & trackedFace) {
	return Point(trackedFace.x + trackedFace.width / 2, trackedFace.y + trackedFace.height / 2);
}

// ROI内で顔を検出する
void FaceDetector::detectFaceAroundRoi(const Mat &resizedFrame)
{
	m_faceCascade->detectMultiScale(resizedFrame(m_faceRoi), m_allFaces, 1.1, 3, 0
		, Size(m_trackedFace.width * 8 / 10, m_trackedFace.height * 8 / 10)
		, Size(m_trackedFace.width * 12 / 10, m_trackedFace.height * 12 / 10));

	if (m_allFaces.empty()) {
		// テンプレートマッチングをスタートさせる
		m_templateMatchingRunning = true;
		if (m_templateMatchingStartTime == 0) {
			m_templateMatchingStartTime = getTickCount();
		}
		return;
	}

	// テンプレートマッチングが動作していれば停止し、開始時間もリセットする
	m_templateMatchingRunning = false;
	m_templateMatchingCurrentTime = m_templateMatchingStartTime = 0;

	// 顔領域を一つだけ取得(detectFaceAllSize()内と同様の処理)
	m_trackedFace = biggestFace(m_allFaces);

	// ROIオフセットを加算
	m_trackedFace.x = m_trackedFace.x + m_faceRoi.x;
	m_trackedFace.y = m_trackedFace.y + m_faceRoi.y;

	// 顔のテンプレ画像を取得
	m_faceTemplate = getFaceTemplate(resizedFrame, m_trackedFace);

	// ROI再計算
	m_faceRoi = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows));

	// 顔推定位置更新
	m_facePosition = centerOfRect(m_trackedFace);
}

// HaarLikeによりROI内で笑顔検出を行う。
void FaceDetector::detectSmileAroundRoi(const Mat & resizedFrame)
{
	// 笑顔検出
	m_smileCascade->detectMultiScale(resizedFrame(m_faceRoi), m_allSmiles, 1.1, 3, 0
		, Size(m_trackedFace.width / 2, m_trackedFace.height / 2));

	if (m_allSmiles.empty()) {
		cout << "笑顔が空です" << endl;
		m_foundSmile = false; // 笑顔が見つからなった
		return;
	}

	cout << "笑顔が見つかりました。" << endl;
	m_foundSmile = true; // 笑顔が見つかった

	// 笑顔領域を一つだけ取得
	m_trackedSmile = biggestFace(m_allSmiles);
	
	// Roiオフセット計算
	//m_trackedSmile.x = m_trackedSmile.x + m_faceRoi.width;
	//m_trackedSmile.y = m_trackedSmile.y + m_faceRoi.height;

	m_smilePosition = centerOfRect(m_trackedSmile);
}

// フレームの画素値の平均値を返す
Scalar FaceDetector::calcBGRAverage( Mat & roi_image, const Mat & mask_image){
	for (int i = 0; i < roi_image.total(); i++) {
		if (mask_image.data[i] == 255) {
			roi_image.data[i] = 0;
		}
	}
	return mean(mask_image);
}

// ROI画像を受け取って肌色抽出を行う
Scalar FaceDetector::extractSkinColor(const Mat & frame)
{
	Mat roi_image = frame(getFaceRect()).clone();
	Mat hsv_image, mask_image;
	
	cvtColor(roi_image, hsv_image, COLOR_BGR2HSV);

	Scalar scalar_min = Scalar(0, 58, 88); // 肌色最低値
	Scalar scalar_max = Scalar(25, 173, 229); // 肌色最高値

	inRange(hsv_image, scalar_min, scalar_max, mask_image); // 肌色抽出
	//cvtColor(mask_image, mask_image, CV_GRAY2BGR);

	unsigned int count = 0;
	auto start = high_resolution_clock::now();
	int blue = 0, green = 0, red = 0;
	double blue_ave, green_ave, red_ave;

	for (int y = 0; y < roi_image.rows; y++) {
		Vec3b *first_row_ptr = roi_image.ptr<Vec3b>(y); // 画像の行の先頭のポインタ取得
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

// Template Matchingで顔を検出する
void FaceDetector::detectFacesTemplateMatching(Mat &resizedFrame)
{
	m_templateMatchingCurrentTime = getTickCount();
	double duration = (double)(m_templateMatchingCurrentTime - m_templateMatchingStartTime) / getTickFrequency();

	if (duration > m_templateMathingMaxDuration) {
		m_foundFace = false; // 顔を見失った
		m_templateMatchingRunning = false; // テンプレマッチング停止
		m_templateMatchingStartTime = 0; // 開始時刻リセット
		m_templateMatchingCurrentTime = 0; // 動作時間リセット

		// 顔位置リセット
		m_facePosition.x = m_facePosition.y = 0;
		m_trackedFace.x = m_trackedFace.y = m_trackedFace.width = m_trackedFace.height = 0;
		return;
	}

	// テンプレート画像が極めて小さい場合テンプレートマッチング中断
	if (m_faceTemplate.rows * m_faceTemplate.cols == 0 ||
		m_faceTemplate.rows <= 10 || m_faceTemplate.cols <= 10)
	{
		m_foundFace = false; // 顔を見失った
		m_templateMatchingRunning = false; // テンプレマッチング停止
		m_templateMatchingStartTime = 0; // 開始時刻リセット
		m_templateMatchingCurrentTime = 0; // 動作時間リセット

		// 顔位置リセット
		m_facePosition.x = m_facePosition.y = 0;
		m_trackedFace.x = m_trackedFace.y = m_trackedFace.width = m_trackedFace.height = 0;
		return;
	}

	// テンプレートマッチング開始
	matchTemplate(resizedFrame(m_faceRoi), m_faceTemplate, m_matchingResult, CV_TM_SQDIFF);
	//matchTemplate(resizedFrame(m_faceRoi), m_faceTemplate, m_matchingResult, CV_TM_SQDIFF_NORMED);
	normalize(m_matchingResult, m_matchingResult, 0, 1, NORM_MINMAX, -1, Mat());
	double min, max;
	Point minLoc, maxLoc;
	minMaxLoc(m_matchingResult, &min, &max, &minLoc, &maxLoc);

	// roiオフセットを加算
	minLoc.x = minLoc.x + m_faceRoi.x;
	minLoc.y = minLoc.y + m_faceRoi.y;

	// 検出した顔を取得
	m_trackedFace = Rect(minLoc.x, minLoc.y, m_faceTemplate.cols, m_faceTemplate.rows);
	m_trackedFace = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows));

	// 新たに顔のテンプレート画像を取得
	m_faceTemplate = getFaceTemplate(resizedFrame, m_trackedFace);

	// ROI計算
	m_faceRoi = doubleRectSize(m_trackedFace, Rect(0, 0, resizedFrame.cols, resizedFrame.rows));

	// 顔位置更新
	m_facePosition = centerOfRect(m_trackedFace);
}

// 顔領域を格納する
Rect FaceDetector::getFaceRect()
{
	Rect faceRect = m_trackedFace;
	faceRect.x = faceRect.x / m_scale;
	faceRect.y = faceRect.y / m_scale;
	faceRect.width = faceRect.width / m_scale;
	faceRect.height = faceRect.height / m_scale;

	return faceRect;
}

// 顔領域の中の更に小さな肌領域を算出
Rect FaceDetector::getFaceROIRect() {
	Rect faceRoiRect = getFaceRect();
	faceRoiRect.x = faceRoiRect.x * 1.2;
	faceRoiRect.y = faceRoiRect.y * 1.2;
	faceRoiRect.width = faceRoiRect.width * 0.5;
	faceRoiRect.height = faceRoiRect.height * 0.5;
	return faceRoiRect;
}

// 顔領域の中心点を取得
Point FaceDetector::getFaceCenterPoint()
{
	Point facePos;
	facePos.x = (int)(m_facePosition.x / m_scale);
	facePos.y = (int)(m_facePosition.y / m_scale);
	return facePos;
}

// 笑顔領域の中心点を取得
Point FaceDetector::getSmileCenterPoint()
{
	Point smilePos;
	smilePos.x = (int)(m_smilePosition.x / m_scale);
	smilePos.y = (int)(m_smilePosition.y / m_scale);
	return smilePos;
}

// 関数を放り込んで処理時間を計測する
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

// 演算子のオーバーロード、オブジェクト生成時にカメラ映像とcascade fileを渡す
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