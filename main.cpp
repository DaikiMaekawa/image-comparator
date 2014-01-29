#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

const char *IMAGE_DIR = "caltech101_10";
const char *OBJ_FILE = "object.txt";        // 物体ID格納ファイル
const char *DESC_FILE = "description.txt";  // 特徴量格納ファイル

const double SURF_PARAM = 400;  // SURFのパラメータ
const int DIM = 128;            // SURF特徴量の次元数

/**
 * SURF特徴量を抽出する
 *
 * @param[in]  filename         画像ファイル名
 * @param[out] imageKeypoints   キーポイント（出力のため参照渡し）
 * @param[out] imageDescriptors 各キーポイントのSURF特徴量（出力のため参照渡し）
 * @param[out] storage          Memory Storage（出力のため参照渡し）
 *
 * @return 成功なら0、失敗なら1
 */
int extractSURF(char *filename, CvSeq* &imageKeypoints, CvSeq* &imageDescriptors, CvMemStorage* &storage) {
    // グレースケールで画像をロードする
    IplImage *img = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if (img == NULL) {
        cerr << "cannot load image file: " << filename << endl;
        return 1;
    }

    storage = cvCreateMemStorage(0);
    CvSURFParams params = cvSURFParams(SURF_PARAM, 1);
    cvExtractSURF(img, 0, &imageKeypoints, &imageDescriptors, storage, params);

    return 0;
}

/**
 * 物体モデルをファイルに保存する
 *
 * @param[in]   objId              オブジェクトID
 * @param[in]   filename           画像ファイル名
 * @param[in]   imageKeypoints     キーポイント
 * @param[in]   imageDescriptors   各キーポイントの特徴量
 * @param[in]   objFile            物体IDファイルのハンドラ
 * @param[in]   descFile           特徴量ファイルのハンドラ
 *
 * @return 成功なら0、失敗なら1
 */
int saveFile(int objId, char *filename, CvSeq* imageKeypoints, CvSeq* imageDescriptors, ofstream& objFile, ofstream& descFile) {
    cout << objId << " " << filename << " " << imageDescriptors->total << endl;

    // 物体IDファイルへ登録
    objFile << objId << "\t" << filename << endl;

    // オブジェクトID, ラプラシアン, 128個の数字をタブ区切りで出力
    for (int i = 0; i < imageDescriptors->total; i++) {  // 各キーポイントの特徴量に対し
        // オブジェクトID
        descFile << objId << "\t";

        // 特徴点のラプラシアン（SURF特徴量ではベクトルの比較時に使用）
        const CvSURFPoint* kp = (const CvSURFPoint*)cvGetSeqElem(imageKeypoints, i);
        int laplacian = kp->laplacian;
        descFile << laplacian << "\t";

        // 128次元ベクトル
        const float *descriptor = (const float *)cvGetSeqElem(imageDescriptors, i);
        for (int d = 0; d < DIM; d++) {
            descFile << descriptor[d] << "\t";
        }

        descFile << endl;
    }

    return 0;
}

int main(int argc, char **argv) {
    int ret;

    // 物体IDファイルを開く
    ofstream objFile(OBJ_FILE);
    if (objFile.fail()) {
        cerr << "cannot open file: " << OBJ_FILE << endl;
        return 1;
    }

    // 特徴量ファイルを開く
    ofstream descFile(DESC_FILE);
    if (descFile.fail()) {
        cerr << "cannot open file: " << DESC_FILE << endl;
        return 1;
    }

    // IMAGE_DIRの画像ファイル名を走査
    DIR *dp = opendir(IMAGE_DIR);
    if (dp == NULL) {
        cerr << "cannot open directory: " << IMAGE_DIR << endl;
        return 1;
    }

    int objId = 0;  // オブジェクトID
    struct dirent *entry;
    while (1) {
        entry = readdir(dp);

        if (entry == NULL) {
            break;
        }

        // .と..は無視する
        if (strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0) {
            continue;
        }

        char *filename = entry->d_name;

        // SURFを抽出
        char buf[1024];
        sprintf(buf, "%s/%s", IMAGE_DIR, filename);
        CvSeq *imageKeypoints = 0;
        CvSeq *imageDescriptors = 0;
        CvMemStorage *storage = 0;
        ret = extractSURF(buf, imageKeypoints, imageDescriptors, storage);
        if (ret != 0) {
            cerr << "cannot extract surf description" << endl;
            return 1;
        }

        // ファイルへ出力
        ret = saveFile(objId, filename, imageKeypoints, imageDescriptors, objFile, descFile);
        if (ret != 0) {
            cerr << "cannot save surf description" << endl;
            return 1;
        }

        // 後始末
        cvClearSeq(imageKeypoints);
        cvClearSeq(imageDescriptors);
        cvReleaseMemStorage(&storage);

        objId++;
    }

    objFile.close();
    descFile.close();
    closedir(dp);

    return 0;
}