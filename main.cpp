#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

const char *IMAGE_DIR = "caltech101_10";
const char *OBJ_FILE = "object.txt";        // ����ID�i�[�t�@�C��
const char *DESC_FILE = "description.txt";  // �����ʊi�[�t�@�C��

const double SURF_PARAM = 400;  // SURF�̃p�����[�^
const int DIM = 128;            // SURF�����ʂ̎�����

/**
 * SURF�����ʂ𒊏o����
 *
 * @param[in]  filename         �摜�t�@�C����
 * @param[out] imageKeypoints   �L�[�|�C���g�i�o�͂̂��ߎQ�Ɠn���j
 * @param[out] imageDescriptors �e�L�[�|�C���g��SURF�����ʁi�o�͂̂��ߎQ�Ɠn���j
 * @param[out] storage          Memory Storage�i�o�͂̂��ߎQ�Ɠn���j
 *
 * @return �����Ȃ�0�A���s�Ȃ�1
 */
int extractSURF(char *filename, CvSeq* &imageKeypoints, CvSeq* &imageDescriptors, CvMemStorage* &storage) {
    // �O���[�X�P�[���ŉ摜�����[�h����
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
 * ���̃��f�����t�@�C���ɕۑ�����
 *
 * @param[in]   objId              �I�u�W�F�N�gID
 * @param[in]   filename           �摜�t�@�C����
 * @param[in]   imageKeypoints     �L�[�|�C���g
 * @param[in]   imageDescriptors   �e�L�[�|�C���g�̓�����
 * @param[in]   objFile            ����ID�t�@�C���̃n���h��
 * @param[in]   descFile           �����ʃt�@�C���̃n���h��
 *
 * @return �����Ȃ�0�A���s�Ȃ�1
 */
int saveFile(int objId, char *filename, CvSeq* imageKeypoints, CvSeq* imageDescriptors, ofstream& objFile, ofstream& descFile) {
    cout << objId << " " << filename << " " << imageDescriptors->total << endl;

    // ����ID�t�@�C���֓o�^
    objFile << objId << "\t" << filename << endl;

    // �I�u�W�F�N�gID, ���v���V�A��, 128�̐������^�u��؂�ŏo��
    for (int i = 0; i < imageDescriptors->total; i++) {  // �e�L�[�|�C���g�̓����ʂɑ΂�
        // �I�u�W�F�N�gID
        descFile << objId << "\t";

        // �����_�̃��v���V�A���iSURF�����ʂł̓x�N�g���̔�r���Ɏg�p�j
        const CvSURFPoint* kp = (const CvSURFPoint*)cvGetSeqElem(imageKeypoints, i);
        int laplacian = kp->laplacian;
        descFile << laplacian << "\t";

        // 128�����x�N�g��
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

    // ����ID�t�@�C�����J��
    ofstream objFile(OBJ_FILE);
    if (objFile.fail()) {
        cerr << "cannot open file: " << OBJ_FILE << endl;
        return 1;
    }

    // �����ʃt�@�C�����J��
    ofstream descFile(DESC_FILE);
    if (descFile.fail()) {
        cerr << "cannot open file: " << DESC_FILE << endl;
        return 1;
    }

    // IMAGE_DIR�̉摜�t�@�C�����𑖍�
    DIR *dp = opendir(IMAGE_DIR);
    if (dp == NULL) {
        cerr << "cannot open directory: " << IMAGE_DIR << endl;
        return 1;
    }

    int objId = 0;  // �I�u�W�F�N�gID
    struct dirent *entry;
    while (1) {
        entry = readdir(dp);

        if (entry == NULL) {
            break;
        }

        // .��..�͖�������
        if (strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0) {
            continue;
        }

        char *filename = entry->d_name;

        // SURF�𒊏o
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

        // �t�@�C���֏o��
        ret = saveFile(objId, filename, imageKeypoints, imageDescriptors, objFile, descFile);
        if (ret != 0) {
            cerr << "cannot save surf description" << endl;
            return 1;
        }

        // ��n��
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