// g++ -O3 filmfestival01_2_2.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -I/opt/homebrew/include -L/opt/homebrew/lib -lalut -framework OpenAL -Wno-deprecated
#include <iostream>    //標準入出力関数のインクルード
#include <GLUT/glut.h> //OpenGL/GLUT関数のインクルード
#include <math.h>      //算術関数のインクルード
#include <time.h>      //時間を使えるようにするインクルード
#include <unistd.h>    //シリアル通信用
#include <fcntl.h>     //シリアル通信用
#include <termios.h>   //シリアル通信用
#include <sys/time.h>
#include <sys/stat.h>
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ
#include <AL/alut.h>          //OpenAL

// 関数名の宣言　　　（voidは数値を出さずCGの場合映像を出す）
void display0();
void display1();
void reshape0(int w, int h);
void reshape1(int w, int h);
void timer(int val);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void initGL();
void initAL();
void setKogen(double x, double y, double z, double d);
void putBlock(double x, double y, double z, double size, double r, double g, double b);  // ブロック配置
void putSphere(double x, double y, double z, double size, double r, double g, double b); // 球配置
// 板を配置する関数（座標(x,y,z)，サイズ(sizeX,sizeY)(横、縦)，回転角度(rotateX,rotateY,rotateZ)*なにもしなかったら立った状態，色(r,g,b)）
void putPlane(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b);
void putPlaneA(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b, double a);
void putPlaneT(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b, int texID);
void putPlaneTA(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b, double a, int texID);
void putPlane2(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ);
void scene(); // シーン表示
void gameReset();

// グローバル変数の宣言
double eDegY = 170.0, eDegX = 5.0, eDist = 15000.0; // 視点の水平角，垂直角，距離
int mX, mY;                                         // マウスクリック位置格納用
double fr = 60.0;                                   // フレームレート
int winID[2];                                       // ウィンドウID
int winW[2], winH[2];                               // ウィンドウサイズ
double floorSizeX = 5000.0, floorSizeZ = 20000.0;   // フロアの横幅，長さ
double aisleWidth = 750.0;                          // 通路の幅
double playerPos[3];                                // プレイヤーの（初期）位置
double playerSpd = 200.0;                           // プレイヤーの移動速度
int playerMoveFlg = 0;                              // プレイヤーの移動ON/OFF
double audienceX[AUDIENCENUM], audienceY[AUDIENCENUM], audienceZ[AUDIENCENUM];
double audienceR[AUDIENCENUM], audienceG[AUDIENCENUM], audienceB[AUDIENCENUM], audienceD[AUDIENCENUM], audienceD2[AUDIENCENUM];
double itemX[ITEMNUM], itemY[ITEMNUM], itemZ[ITEMNUM], itemStopX[ITEMNUM], itemSpd[ITEMNUM];
int audienceType[AUDIENCENUM];
int itemType[ITEMNUM];   // アイテムの種類
int itemStatus[ITEMNUM]; // アイテムの状態(1:有効、0:無効)
int itemTotalScore = 0;  // アイテム得点
int scrType[SCRTYPENUM];
double theta = 0.0;
int itemScore[] = {-10, 20, 10, -50, 50}; // 各アイテムの得点
int gameMode = 0;                         // ゴール到着フラグ
// シリアル通信関係
int fd;                    // シリアルポート
char bufferAll[BUFF_SIZE]; // 蓄積バッファデータ
int bufferPoint = 0;       // 蓄積バッファデータサイズ
double val[6];             // 受信データ
double hosei = 0.0;
double hoseiSum;
int hoseiCnt;
int hoseiFlg = 0;
double theta2 = 0.0;
double theta1 = 0.0;
double thetaLookUp = 0.0;
long tempTimeSec1;
double efctX, efctY, efctZ, efctSize = 50.0, efctAlpha = 0.0;
double cdX, cdY, cdZ, cdSize = 50.0, cdAlpha = 0.0;
double rotateX, rotateY, rotateZ;
double nmbrX = 0.0, nmbrY = 0.0, nmbrZ = 0.1, efctAlpha1[3];
int nmbrType[NMBRTYPENUM];
double right = 0.0;
cv::VideoCapture capture;
cv::Mat photoImage, photoImageR;
int photoSaveFlg = 0;
int pachaCnt;
int pachaItemID;

// OpenAL
ALuint sourceItem[5]; // アイテムゲット効果音
ALuint sourceBGM;     // BGM
ALuint sourcePanel;   // パネル登場音

time_t timeX;
tm *local;

// main関数
int main(int argc, char *argv[])
{
    initSerial(); // シリアルポート初期化

    srand((unsigned)time(NULL));

    alutInit(&argc, argv); // OpenAL初期化
    glutInit(&argc, argv); // OpenGL初期化

    capture = cv::VideoCapture(0);
    photoImageR = cv::Mat(cv::Size(512, 512), CV_8UC3);

    initGL(); // OpenGL初期設定
    initAL(); // OpenAL初期設定

    // itemX = -800.0; itemY = 100.0; itemZ = -8000.0;

    double w = 0.8 * floorSizeZ / (AUDIENCENUM / 2 - 1);
    for (int i = 0; i < AUDIENCENUM / 2; i++)
    {
        //-------左側-------
        audienceR[i] = 1.0 * rand() / RAND_MAX;
        audienceG[i] = 1.0 * rand() / RAND_MAX; // 1.0がないと０になってしまう（randは整数）
        audienceB[i] = 1.0 * rand() / RAND_MAX;
        // 座標
        audienceX[i] = aisleWidth / 2 + 50.0 * rand() / RAND_MAX;
        audienceY[i] = 100.0;
        audienceZ[i] = -0.4 * floorSizeZ + w * i;
        // 角度
        audienceD[i] = 60.0 + 10.0 * rand() / RAND_MAX;

        //----右側----
        audienceR[i + AUDIENCENUM / 2] = audienceR[i];
        audienceG[i + AUDIENCENUM / 2] = audienceG[i];
        audienceB[i + AUDIENCENUM / 2] = audienceB[i];
        // 座標
        audienceX[i + AUDIENCENUM / 2] = -audienceX[i];
        audienceY[i + AUDIENCENUM / 2] = audienceY[i];
        audienceZ[i + AUDIENCENUM / 2] = audienceZ[i];
        // 角度
        audienceD[i + AUDIENCENUM / 2] = -audienceD[i];
    }
    for (int i = 0; i < AUDIENCENUM; i++)
    {
        audienceType[i] = rand() % PERSONTYPENUM;
        audienceD2[i] = 2.0 * M_PI * rand() / RAND_MAX;
    }
    gameReset();

    glutMainLoop(); // イベント待ち無限ループ

    return 0; // 戻り値
}

// ゲームリセット
void gameReset()
{
    // プレイヤーの（初期）位置
    playerPos[0] = 0.0,
    playerPos[1] = 150.0,
    playerPos[2] = -floorSizeZ * 0.4; // プレイヤーの（初期）位置

    // ドアの角度
    rotateX = 0.0, rotateY = 0.0, rotateZ = 0.0;

    // カメラ撮影カウントダウン
    pachaCnt = 90;

    // アイテム設定
    for (int i = 0; i < ITEMNUM; i++)
    {
        int id = rand() % 100; // idは0~99の乱数
        // itemX[i] = audienceX [id]; itemY[i]= 100.0; itemZ[i] = audienceZ [id];
        itemX[i] = ((rand() % 2) * 2 - 1) * aisleWidth / 2 * 1.1;
        itemY[i] = 125.0;
        itemZ[i] = 0.7 * floorSizeZ * rand() / RAND_MAX - 0.35 * floorSizeZ;

        itemStopX[i] = 300.0 * rand() / RAND_MAX - 150.0;
        if (itemX[i] > 0)
        {
            itemSpd[i] = -500.0 - 500.0 * rand() / RAND_MAX;
        }
        else
        {
            itemSpd[i] = 500.0 + 500.0 * rand() / RAND_MAX;
        }
        itemType[i] = rand() % (ITEMTYPENUM - 2); // 0~ITEMTYPENUM-3
        itemStatus[i] = 1;                        // 初期状態は有効
    }
    // レアアイテム設定
    itemType[rand() % ITEMNUM] = 3; // 0~ITEMNUM-1のいずれかのitemTypeを”３”に設定
    // putPlaneT(itemStopX, 125.0, itemZ, 10.0, 10.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 9);
    int flg = 0;
    while (1)
    {                                 // 無限ループ
        int temID = rand() % ITEMNUM; // 0~ITEMNUM-1のいずれかの値
        if (itemType[temID] != 3)
        {
            itemType[temID] = 4; // itemTypeを”4”に設定
            pachaItemID = temID;
            break;
        }
    }
    // スコアリセット
    itemTotalScore = 0;
}
// OpenAl初期設定
void initAL()
{
    // バッファ（サウンドファイルの一時保存）の生成
    ALuint buffer;                                    // バッファ
    alGenBuffers(1, &buffer);                         // バッファ生成
    buffer = alutCreateBufferFromFile("wav/kya.wav"); // ファイル
    // 音源
    buffer = alutCreateBufferFromFile("wav/kya.wav");      // ファイル
    alGenSources(1, &sourceItem[0]);                       // 音源生成
    alSourcei(sourceItem[0], AL_BUFFER, buffer);           // 音源にバッファを結び付け
    alSourcei(sourceItem[0], AL_LOOPING, AL_FALSE);        // ループ再生設定
    alSourcei(sourceItem[0], AL_REFERENCE_DISTANCE, 1.0);  // 距離感設定
    alSourcef(sourceItem[0], AL_PITCH, 1.0);               // ピッチ設定
    alSourcef(sourceItem[0], AL_GAIN, 1.0);                // 音量設定 1.0
    alSource3f(sourceItem[0], AL_POSITION, 0.0, 0.0, 0.0); // 位置設定

    // 音源
    buffer = alutCreateBufferFromFile("wav/born.wav");     // ファイル
    alGenSources(1, &sourceItem[1]);                       // 音源生成
    alSourcei(sourceItem[1], AL_BUFFER, buffer);           // 音源にバッファを結び付け
    alSourcei(sourceItem[1], AL_LOOPING, AL_FALSE);        // ループ再生設定
    alSourcei(sourceItem[1], AL_REFERENCE_DISTANCE, 1.0);  // 距離感設定
    alSourcef(sourceItem[1], AL_PITCH, 1.0);               // ピッチ設定
    alSourcef(sourceItem[1], AL_GAIN, 0.8);                // 音量設定 0.8
    alSource3f(sourceItem[1], AL_POSITION, 0.0, 0.0, 0.0); // 位置設定

    // 音源
    buffer = alutCreateBufferFromFile("wav/banana.wav");   // ファイル
    alGenSources(1, &sourceItem[2]);                       // 音源生成
    alSourcei(sourceItem[2], AL_BUFFER, buffer);           // 音源にバッファを結び付け
    alSourcei(sourceItem[2], AL_LOOPING, AL_FALSE);        // ループ再生設定
    alSourcei(sourceItem[2], AL_REFERENCE_DISTANCE, 1.0);  // 距離感設定
    alSourcef(sourceItem[2], AL_PITCH, 1.0);               // ピッチ設定
    alSourcef(sourceItem[2], AL_GAIN, 1.0);                // 音量設定 1.0
    alSource3f(sourceItem[2], AL_POSITION, 0.0, 0.0, 0.0); // 位置設定

    // 音源
    buffer = alutCreateBufferFromFile("wav/sain.wav");     // ファイル
    alGenSources(1, &sourceItem[3]);                       // 音源生成
    alSourcei(sourceItem[3], AL_BUFFER, buffer);           // 音源にバッファを結び付け
    alSourcei(sourceItem[3], AL_LOOPING, AL_FALSE);        // ループ再生設定
    alSourcei(sourceItem[3], AL_REFERENCE_DISTANCE, 1.0);  // 距離感設定
    alSourcef(sourceItem[3], AL_PITCH, 1.0);               // ピッチ設定
    alSourcef(sourceItem[3], AL_GAIN, 2.0);                // 音量設定 2.0
    alSource3f(sourceItem[3], AL_POSITION, 0.0, 0.0, 0.0); // 位置設定

    // 音源
    buffer = alutCreateBufferFromFile("wav/pon.wav");      // ファイル
    alGenSources(1, &sourceItem[4]);                       // 音源生成
    alSourcei(sourceItem[4], AL_BUFFER, buffer);           // 音源にバッファを結び付け
    alSourcei(sourceItem[4], AL_LOOPING, AL_FALSE);        // ループ再生設定
    alSourcei(sourceItem[4], AL_REFERENCE_DISTANCE, 1.0);  // 距離感設定
    alSourcef(sourceItem[4], AL_PITCH, 1.0);               // ピッチ設定
    alSourcef(sourceItem[4], AL_GAIN, 1.0);                // 音量設定 1.0
    alSource3f(sourceItem[4], AL_POSITION, 0.0, 0.0, 0.0); // 位置設定

    // 音源
    buffer = alutCreateBufferFromFile("wav/koneko.wav"); // ファイル
    alGenSources(1, &sourceBGM);                         // 音源生成
    alSourcei(sourceBGM, AL_BUFFER, buffer);             // 音源にバッファを結び付け
    alSourcei(sourceBGM, AL_LOOPING, AL_TRUE);           // ループ再生設定
    alSourcei(sourceBGM, AL_REFERENCE_DISTANCE, 1.0);    // 距離感設定
    alSourcef(sourceBGM, AL_PITCH, 1.0);                 // ピッチ設定
    alSourcef(sourceBGM, AL_GAIN, 1.0);                  // 音量設定 1.0
    alSource3f(sourceBGM, AL_POSITION, 0.0, 0.0, 0.0);   // 位置設定
    alSourcePlay(sourceBGM);                             // 音の再生

    // 音源
    buffer = alutCreateBufferFromFile("wav/jajan.wav");  // ファイル
    alGenSources(1, &sourcePanel);                       // 音源生成
    alSourcei(sourcePanel, AL_BUFFER, buffer);           // 音源にバッファを結び付け
    alSourcei(sourcePanel, AL_LOOPING, AL_FALSE);        // ループ再生設定
    alSourcei(sourcePanel, AL_REFERENCE_DISTANCE, 1.0);  // 距離感設定
    alSourcef(sourcePanel, AL_PITCH, 1.0);               // ピッチ設定
    alSourcef(sourcePanel, AL_GAIN, 1.0);                // 音量設定 1.0
    alSource3f(sourcePanel, AL_POSITION, 0.0, 0.0, 0.0); // 位置設定

    // リスナー
    ALfloat orient[] = {0.0, 0.0, 1.0, 0.0, 1.0, 0.0}; // 方向+上方向
    alListenerfv(AL_ORIENTATION, orient);              // 方向
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);          // 位置

    // alSourcePlay(sourceItem);  //音の再生
}
// シーン表示
void scene()
{
    // 光源の位置指定
    setKogen(playerPos[0], playerPos[1] + 100, playerPos[2], 1.0);

    // 床の表示
    putPlane2(0.0, 0.0, 0.0, floorSizeX, floorSizeZ, 90.0, 0.0, 0.0); // 床はputplane2は格子にしかならないようになってる

    //----------ここに表示物体を記述----------
    // 観客
    for (int i = 0; i < AUDIENCENUM; i++)
    {
        putPlaneT(audienceX[i], audienceY[i], audienceZ[i], 150.0, 200.0, 0.0, audienceD[i], 10.0 * sin(audienceD2[i] + theta2), 1.0, 1.0, 1.0, audienceType[i] + 100);
    }

    // ロープ
    for (int i = 0; i < 100; i++)
    {
        putPlaneT(aisleWidth * 0.45, 50.0, -0.5 * floorSizeZ + 100.0 + 180.0 * i, 200.0, 100.0, 0.0, 90.0, 0.0, 1.0, 1.0, 1.0, 2);
        putPlaneT(-aisleWidth * 0.45, 50.0, -0.5 * floorSizeZ + 100.0 + 180.0 * i, 200.0, 100.0, 0.0, -90.0, 0.0, 1.0, 1.0, 1.0, 2);
    }

    // アイテム
    for (int i = 0; i < ITEMNUM; i++)
    {
        if (itemStatus[i] == 0)
            continue;

        putPlaneT(itemX[i], itemY[i], itemZ[i], 50.0, 50.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, itemType[i] + 200);
    }

    // ドア
    glPushMatrix();                                          // 行列の一時保存
    glTranslated(162.0, 0.0, 0.42 * floorSizeZ - 100.0);     // 3,元の位置に戻す
    glRotated(rotateY, 0.0, 1.0, 0.0);                       // 2,Y軸周りにrotateY度回転
    glTranslated(-162.0, 0.0, -(0.42 * floorSizeZ - 100.0)); // 1,原点付近に移動

    putPlaneT(81.0, 140, 0.42 * floorSizeZ - 100.0, 162.0, 280.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 11); // 左
    glPopMatrix();                                                                                   // 一時保存した行列の復帰

    glPushMatrix();                                         // 行列の一時保存
    glTranslated(-162.0, 0.0, 0.42 * floorSizeZ - 100.0);   // 3,元の位置に戻す
    glRotated(-rotateY, 0.0, 1.0, 0.0);                     // 2,Y軸周りにrotateY度回転
    glTranslated(162.0, 0.0, -(0.42 * floorSizeZ - 100.0)); // 1,原点付近に移動

    putPlaneT(-81.0, 140, 0.42 * floorSizeZ - 100.0, 162.0, 280.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0); // 右
    glPopMatrix();                                                                                   // 一時保存した行列の復帰

    // setKogen(0.0, 0.0, 0.42*floorSizeZ-120.0, right);
    if (playerPos[2] > floorSizeZ * 0.4 - 450.0)
    {
        right += 0.1;
        rotateY += 0.5;
        rotateX += 0.5;
        rotateZ += 0.5;
    }

    if (rotateY > 90)
        rotateY = 90;

    // ドア横
    putPlaneT(405.0, 140, 0.42 * floorSizeZ - 100.0, 500.0, 500.0 * 9 / 16, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 12);
    putPlaneT(-410.0, 140, 0.42 * floorSizeZ - 100.0, 500.0, 500.0 * 9 / 16, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 12);

    // ドア奥のビル
    putPlaneT(0.0, 800, 0.42 * floorSizeZ + 100.0, 1600.0, 1600.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1);

    // ビル
    for (int i = 0; i < 15; i++)
    {
        putPlaneT(aisleWidth * 0.8, 800, -0.5 * floorSizeZ + 800.0 + 1600.0 * i, 1600.0, 1600.0, 0.0, 90.0, 0.0, 1.0, 1.0, 1.0, 1);
        putPlaneT(-aisleWidth * 0.8, 800, -0.5 * floorSizeZ + 800.0 + 1600.0 * i, 1600.0, 1600.0, 0.0, -90.0, 0.0, 1.0, 1.0, 1.0, 1);
    }

    // ドアの奥の白板
    putPlaneA(0.0, 140, 0.42 * floorSizeZ + 80.0, 1000.0, 280.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, rotateY / 90);

    if (rotateY > 0)
    {
        // 洩れる光
        double maxRotateY = atan(180.0 / 162.0) * 180.0 / M_PI;
        double doorX = 162.0 - 162.0 * cos(MIN(rotateY, maxRotateY) * M_PI / 180.0);
        double lightX = doorX * 5.0;
        double lightY = (280.0 - 150.0) * 5.0 + 150.0;
        double doorZ = 162.0 * sin(MIN(rotateY, maxRotateY) * M_PI / 180.0) + 0.42 * floorSizeZ - 100.0;
        double lightZ = (doorZ - (0.42 * floorSizeZ + 80.0)) * 5.0 + 0.42 * floorSizeZ + 80.0;

        glDisable(GL_LIGHTING); // ライティング有効化
        glBegin(GL_QUADS);      // 図形(四角形)開始
        // 左側の光
        glColor4d(1.0, 1.0, 1.0, 1.0);
        glVertex3d(0.0, 150.0, 0.42 * floorSizeZ + 80.0); // 頂点
        glVertex3d(0.0, 5.0, 0.42 * floorSizeZ + 80.0);   // 頂点
        glColor4d(1.0, 1.0, 1.0, 0.0);
        glVertex3d(lightX, 5.0, lightZ);    // 頂点
        glVertex3d(lightX, lightY, lightZ); // 頂点
        // 右側の光
        glColor4d(1.0, 1.0, 1.0, 1.0);
        glVertex3d(0.0, 150.0, 0.42 * floorSizeZ + 80.0); // 頂点
        glVertex3d(0.0, 5.0, 0.42 * floorSizeZ + 80.0);   // 頂点
        glColor4d(1.0, 1.0, 1.0, 0.0);
        glVertex3d(-lightX, 5.0, lightZ);    // 頂点
        glVertex3d(-lightX, lightY, lightZ); // 頂点
        // 下側の光
        glColor4d(1.0, 1.0, 1.0, 1.0);
        glVertex3d(0.0, 5.0, 0.42 * floorSizeZ + 80.0); // 頂点
        glVertex3d(0.0, 5.0, 0.42 * floorSizeZ + 80.0); // 頂点
        glColor4d(1.0, 1.0, 1.0, 0.0);
        glVertex3d(lightX, 5.0, lightZ);  // 頂点
        glVertex3d(-lightX, 5.0, lightZ); // 頂点
        // 上側の光
        glColor4d(1.0, 1.0, 1.0, 1.0);
        glVertex3d(0.0, 150.0, 0.42 * floorSizeZ + 80.0); // 頂点
        glVertex3d(0.0, 150.0, 0.42 * floorSizeZ + 80.0); // 頂点
        glColor4d(1.0, 1.0, 1.0, 0.0);
        glVertex3d(lightX, lightY, lightZ);  // 頂点
        glVertex3d(-lightX, lightY, lightZ); // 頂点
        glEnd();                             // 図形終了
        glEnable(GL_LIGHTING);               // ライティング無効化
    }

    // フラッシュ
    if (pachaCnt == 0)
    {
        putPlaneTA(cdX, cdY, cdZ, cdSize, cdSize, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, cdAlpha, 13);
        if (cdSize < 500)
            cdSize *= 1.05;
        cdAlpha *= 0.95;
    }

    // 煙
    putPlaneTA(efctX, efctY, efctZ, efctSize, efctSize, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, efctAlpha, 9);

    //    //ボンネット
    //    if(gameMode==3)
    //        putPlaneT(playerPos[0], playerPos[1]+60.0, playerPos[2]+90.0, 200.0, 200.0, 80.0, 0.0, 0.0, 1.0, 1.0, 1.0, 8);
}

// タイマーコールバック
void timer(int val1)
{
    system("clear");

    // cv::Mat captureImage;
    // capture >> captureImage;

    glutSetWindow(winID[0]);
    glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバックの再指定

    getSerialData(); // ※加速度：val[0],val[1],val[2]，角速度val[3], val[4], val[5]を入手
    glutPostRedisplay();

    glutSetWindow(winID[1]);
    glutPostRedisplay();

    //    if(captureImage.data != NULL)
    //        cv::imshow("CAM", captureImage);

    struct timeval tempTimeVal;
    gettimeofday(&tempTimeVal, NULL);
    long tempTimeSec = tempTimeVal.tv_sec * 10000 + tempTimeVal.tv_usec / 100;
    long intervalSec = tempTimeSec - tempTimeSec1;
    tempTimeSec1 = tempTimeSec;
    printf("(%f, %f, %f) (%f, %f, %f)\n", val[0], val[1], val[2], val[3], val[4], val[5] - hosei);
    printf("%ld\n", intervalSec);
    if (hoseiFlg == 1)
    {
        hoseiSum += val[5];
        hoseiCnt += 1;

        if (hoseiCnt == 300)
        {
            hosei = hoseiSum / 300;
            hoseiSum = 0;
            hoseiCnt = 0;
            hoseiFlg = 0;
            // printf("hosei = %f\n", hosei);
        }
    }
    // theta = theta+((val[5]-hosei)/fr);
    theta = theta + (val[5] - hosei) * intervalSec / 10000.0;
    theta2 += 0.1;
    if (theta2 > 2.0 * M_PI)
        theta2 -= 2.0 * M_PI;

    //----------ここに毎フレーム実施する内容を記述----------
    theta1 = theta * 2;
    if (theta1 > 60)
        theta1 = 60;
    else if (theta1 < -60)
        theta1 = -60;
    if (playerMoveFlg)
    {
        playerPos[2] += playerSpd * cos(theta1 * M_PI / 180.0) / fr; // プレイヤーの位置のz座標を 速度/フレームレート 分だけ移動
        playerPos[0] += playerSpd * sin(theta1 * M_PI / 180.0) / fr; // プレイヤーの位置のx座標を 速度/フレームレート 分だけ移動

        // 観客との衝突防止
        if (playerPos[0] > aisleWidth / 2 * 0.8)
        {
            playerPos[0] = aisleWidth / 2 * 0.8;
        }
        else if (playerPos[0] < -aisleWidth / 2 * 0.8)
        {
            playerPos[0] = -aisleWidth / 2 * 0.8;
        }
        // ゴールに到着したらストップ
        if (playerPos[2] > floorSizeZ * 0.4)
        {
            playerMoveFlg = 0;
            gameMode = 4; // ゴール到着
            alSourcePlay(sourcePanel);
        }
    }

    // プレイヤーの座標：(playerPos[0], playerPos[1], playerPos[2])
    // アイテムの座標：(itemX, itemY, itemZ)
    // プレイヤーとアイテムとの距離    (pow(x1-x,2)は(x1-x)の2乗)（sqrt()はルート）(fabs()は絶対値)
    for (int i = 0; i < ITEMNUM; i++)
    {
        double dist = sqrt(pow(playerPos[0] - itemX[i], 2) + pow(playerPos[1] - itemY[i], 2) + pow(playerPos[2] - itemZ[i], 2));

        if (dist < 1200.0 && fabs(itemX[i] - itemStopX[i]) > 50.0)
        {
            itemX[i] += itemSpd[i] / fr;
        }
        // アイテムゲット
        if (dist < 90.0 && playerPos[2] < itemZ[i] && itemStatus[i] == 1)
        {
            itemStatus[i] = 0;

            if (itemType[i] == 0)
            {
                thetaLookUp -= 180.0;
                alSourcePlay(sourceItem[2]); // 音の再生
            }
            else if (itemType[i] == 3)
            {
                efctSize = 50.0;
                efctAlpha = 1.5;
                efctX = itemX[i];
                efctY = itemY[i];
                efctZ = itemZ[i] + 30.0;
                alSourcePlay(sourceItem[1]); // 音の再生
                playerMoveFlg = 0;
                gameMode = 5;
            }
            else if (itemType[i] == 1)
                alSourcePlay(sourceItem[3]); // 音の再生
            else if (itemType[i] == 2)
                alSourcePlay(sourceItem[4]); // 音の再生
            else
            {
                alSourcePlay(sourceItem[0]); // 音の再生

                //                    cv::Mat captureImage;
                //                    capture >> captureImage;
                //                    photoImage = captureImage.clone();
                // cv::imshow("photo", photoImage);
            }

            // itemScore[0],itemScore[1],itemScore[2],itemScore[3],,itemScore[4],itemScore[5]
            // itemType[i]:0~5
            itemTotalScore += itemScore[itemType[i]];
        }
    }

    printf("Score = %04d\n", itemTotalScore);

    if (thetaLookUp < 0)
        thetaLookUp += 5.0;
    else
        thetaLookUp = 0.0;

    // efct
    efctSize *= 1.2;
    if (efctSize > 700)
        efctSize = 700;
    efctAlpha *= 0.7;

    if (itemStatus[pachaItemID] == 0)
    { // トロフィーを取っていたら
        pachaCnt--;
        if (pachaCnt == 0)
        {
            cv::Mat captureImage;
            capture >> captureImage;
            photoImage = captureImage.clone();

            cdSize = 50.0;
            cdAlpha = 2.0;
            cdX = playerPos[0];
            cdY = playerPos[1];
            cdZ = playerPos[2] + 200.0;
        }
        if (pachaCnt < 0)
            pachaCnt = 0;
    }
}

// ディスプレイコールバック
void display0()
{
    // 画面消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 行列初期化
    glLoadIdentity();

    // 視点と視線の設定
    double eX = eDist * cos(M_PI * eDegX / 180.0) * sin(M_PI * eDegY / 180.0);
    double eY = eDist * sin(M_PI * eDegX / 180.0);
    double eZ = eDist * cos(M_PI * eDegX / 180.0) * cos(M_PI * eDegY / 180.0);
    gluLookAt(eX, eY, eZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 視点座標(x,y,z)，注視点座標(x,y,z)，上方向(x,y,z)

    // シーン表示
    scene();

    // プレイヤー表示
    putBlock(playerPos[0], playerPos[1], playerPos[2], 150.0, 1.0, 1.0, 0.0);

    glFlush(); // 描画実行
}

// ディスプレイコールバック
void display1()
{
    // 画面消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);                                              // 投影変換行列を計算対象に設定
    glLoadIdentity();                                                         // 行列初期化
    gluPerspective(40.0, (double)winW[1] / (double)winH[1], 100.0, 100000.0); // 変換行列に透視投影を乗算
    glMatrixMode(GL_MODELVIEW);                                               // モデルビュー変換行列を計算対象に設定

    // 行列初期化
    glLoadIdentity();

    // 視点と視線の設定
    gluLookAt(playerPos[0], playerPos[1], playerPos[2], playerPos[0] + sin(theta1 * 0.1 * M_PI / 180.0), playerPos[1] + sin(thetaLookUp * M_PI / 180.0), playerPos[2] + cos(thetaLookUp * M_PI / 180.0), 0.0, cos(thetaLookUp * M_PI / 180.0), 0.0); // 視点座標(x,y,z)，注視点座標(x,y,z)，上方向(x,y,z)
    //    if(itemType[i]==0){
    //        for()
    //        gluLookAt(playerPos[0], playerPos[1], playerPos[2], playerPos[0]+sin(theta1*0.1*M_PI/180.0), playerPos[1], playerPos[2]+cos(theta1*0.1*M_PI/180.0), 0.0, 1.0, 0.0);
    //    }

    // シーン表示
    scene();

    // パネル
    glMatrixMode(GL_PROJECTION); // 投影変換行列を計算対象に設定
    glLoadIdentity();            // 行列初期化
    glMatrixMode(GL_MODELVIEW);  // モデルビュー変換行列を計算対象に設定
    glLoadIdentity();            // 行列初期化
    setKogen(0.0, 0.0, 3.0, 1.0);
    if (gameMode == 0)
        putPlaneT(0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 3);
    else if (gameMode == 1)
        putPlaneT(0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 4);
    else if (gameMode == 2)
        putPlaneT(0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 10);
    else if (gameMode == 4)
    {
        if (itemTotalScore >= 400)
            putPlaneT(0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 5);
        else if (itemTotalScore >= 250)
            putPlaneT(0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 6);
        else
            putPlaneT(0.0, 0.0, 0.5, 2.0, 2.0, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 7);

        if (photoImage.data != NULL)
        {
            if (photoSaveFlg == 0)
            {
                timeX = time(NULL);
                local = localtime(&timeX);
                char fileName[100];
                sprintf(fileName, "photo/%02d%02d%02d%02d.jpg", local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min);
                cv::imwrite(fileName, photoImage);
                photoSaveFlg = 1;
            }
            cv::resize(photoImage, photoImageR, photoImageR.size());
            glBindTexture(GL_TEXTURE_2D, 400);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, photoImageR.cols, photoImageR.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, photoImageR.data);
            putPlaneT(0.544, -0.305, -0.1, 1.6 * 0.5, 0.9 * 16 / 9 * 0.5, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 400);
            // photoImage.release();
        }
    }
    else if (gameMode == 5)
        putPlaneT(0.0, 0.0, .0, 2.0, 2.0, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 14);

    // スコア表示

    int score = fabs(itemTotalScore); // fabsは絶対値
    int scrnum1, scrnum2, scrnum, score1, score2;
    putPlaneT(0 - 0.6, 0 + 0.8, 0.1, 0.15, 0.15 * 16 / 9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 0 + 300); // 一の位
    score1 = score / 10;

    scrnum1 = score1 % 10;
    putPlaneT(-0.1 - 0.6, 0 + 0.8, 0.1, 0.15, 0.15 * 16 / 9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, scrnum1 + 300); // 十の位

    score2 = score / 100;
    scrnum2 = score2 % 10;
    putPlaneT(-0.2 - 0.6, 0 + 0.8, 0.1, 0.15, 0.15 * 16 / 9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, scrnum2 + 300); // 百の位

    if (itemTotalScore < 0)
        putPlaneT(-0.3 - 0.6, 0 + 0.8, 0.1, 0.15, 0.15 * 16 / 9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 10 + 300); // マイナス

    // カウントダウン
    if (pachaCnt < 90 && pachaCnt > 0)
    {
        int texID = pachaCnt / 30;
        putPlaneTA(0.0, 0.0, 0.0, 0.6, 0.6 * 16 / 9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, 1.0, texID + 500); //  3
    }
    //    putPlaneTA(nmbrX, nmbrY, nmbrZ, 0.6, 0.6*16/9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, efctAlpha1[2], 3+500);  //  3
    //    putPlaneTA(nmbrX, nmbrY, nmbrZ, 0.6, 0.6*16/9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, efctAlpha1[1], 2+500);  //  2
    //    putPlaneTA(nmbrX, nmbrY, nmbrZ, 0.6, 0.6*16/9, 0.0, 180.0, 0.0, 1.0, 1.0, 1.0, efctAlpha1[0], 0+500);  //  1

    glFlush(); // 描画実行
}

// リサイズコールバック
void reshape0(int w, int h)
{
    winW[0] = w;
    winH[0] = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);                                  // 投影変換行列を計算対象に設定
    glLoadIdentity();                                             // 行列初期化
    gluPerspective(40.0, (double)w / (double)h, 100.0, 100000.0); // 変換行列に透視投影を乗算
    glMatrixMode(GL_MODELVIEW);                                   // モデルビュー変換行列を計算対象に設定
}

// リサイズコールバック
void reshape1(int w, int h)
{
    winW[1] = w;
    winH[1] = h;
    glViewport(0, 0, w, h);
}

// 初期設定
void initGL()
{
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH); // ディスプレイモード設定(RGBA, デプスバッファ)
    glutInitWindowSize(800, 450);                // ウィンドウサイズ指定
    glutInitWindowPosition(0, 0);                // ウィンドウ位置指定
    winID[0] = glutCreateWindow("CG0");          // ウィンドウ生成
    glutDisplayFunc(display0);                   // ディスプレイコールバックの指定
    glutReshapeFunc(reshape0);                   // リシェイプコールバックの指定
    glutTimerFunc(1000 / fr, timer, 0);          // タイマーコールバックの指定

    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH); // ディスプレイモード設定(RGBA, デプスバッファ)
    glutInitWindowSize(800, 450);                // ウィンドウサイズ指定
    glutInitWindowPosition(800, 0);              // ウィンドウ位置指定
    winID[1] = glutCreateWindow("CG1");          // ウィンドウ生成
    glutDisplayFunc(display1);                   // ディスプレイコールバックの指定
    glutReshapeFunc(reshape1);                   // リシェイプコールバックの指定

    // テクスチャ
    char fileName[100];
    cv::Mat textureImage[PERSONTYPENUM];
    for (int i = 0; i < PERSONTYPENUM; i++)
    {
        sprintf(fileName, "png/person%02d.png", i);
        textureImage[i] = cv::imread(fileName, cv::IMREAD_UNCHANGED);
    }

    // アイテム
    cv::Mat itemImage[ITEMTYPENUM];
    for (int i = 0; i < ITEMTYPENUM; i++)
    {
        sprintf(fileName, "png/item%02d.png", i); //%02dはiの数を数字二桁で表すようにする、printfの前のsは
        itemImage[i] = cv::imread(fileName, cv::IMREAD_UNCHANGED);
    }
    // cv::imshow("Texture", textureImage);

    // スコア表示
    cv::Mat scrImage[SCRTYPENUM];
    for (int i = 0; i < SCRTYPENUM; i++)
    {
        sprintf(fileName, "png/scr%02d.png", i); //%02dはiの数を数字二桁で表すようにする、printfの前のsは
        scrImage[i] = cv::imread(fileName, cv::IMREAD_UNCHANGED);
    }

    // ナンバー表示
    cv::Mat nmbrImage[NMBRTYPENUM];
    for (int i = 0; i < NMBRTYPENUM; i++)
    {
        sprintf(fileName, "png/count%02d.png", i); //%02dはiの数を数字二桁で表すようにする、printfの前のsは
        nmbrImage[i] = cv::imread(fileName, cv::IMREAD_UNCHANGED);
    }

    cv::Mat tempImage[20];
    tempImage[0] = cv::imread("png/door00.png", cv::IMREAD_UNCHANGED);
    tempImage[1] = cv::imread("png/building.png", cv::IMREAD_UNCHANGED);
    tempImage[2] = cv::imread("png/rope.png", cv::IMREAD_UNCHANGED);
    tempImage[3] = cv::imread("png/title.png", cv::IMREAD_UNCHANGED);
    tempImage[4] = cv::imread("png/start00.png", cv::IMREAD_UNCHANGED);
    tempImage[5] = cv::imread("png/end00.png", cv::IMREAD_UNCHANGED);
    tempImage[6] = cv::imread("png/end01.png", cv::IMREAD_UNCHANGED);
    tempImage[7] = cv::imread("png/end02.png", cv::IMREAD_UNCHANGED);
    tempImage[8] = cv::imread("png/carfront.png", cv::IMREAD_UNCHANGED);
    tempImage[9] = cv::imread("png/efct00.png", cv::IMREAD_UNCHANGED);
    tempImage[10] = cv::imread("png/start01.png", cv::IMREAD_UNCHANGED);
    tempImage[11] = cv::imread("png/door01.png", cv::IMREAD_UNCHANGED);
    tempImage[12] = cv::imread("png/side.png", cv::IMREAD_UNCHANGED);
    tempImage[13] = cv::imread("png/efctFlash.png", cv::IMREAD_UNCHANGED);
    tempImage[14] = cv::imread("png/gameover.png", cv::IMREAD_UNCHANGED);

    for (int i = 0; i < 2; i++)
    {
        glutSetWindow(winID[i]);

        glutMouseFunc(mouse);       // マウスクリックコールバックの指定
        glutMotionFunc(motion);     // マウスドラッグコールバックの指定
        glutKeyboardFunc(keyboard); // キーボードコールバックの指定

        glClearColor(0.0, 0.0, 0.0, 1.0);                  // ウィンドウを消去する色の指定(R:0.0, G:0.0, B:0.2)
        glEnable(GL_LIGHTING);                             // 陰影付けの有効化
        glEnable(GL_LIGHT0);                               // 光源0の有効化
        glEnable(GL_DEPTH_TEST);                           // デプスバッファの有効化
        glEnable(GL_NORMALIZE);                            // 法線正規化の有効化
        glEnable(GL_BLEND);                                // アルファチャンネル有効化
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // アルファ関数の設定
        glEnable(GL_ALPHA_TEST);                           // アルファテスト有効化
        glAlphaFunc(GL_GREATER, 0.01);                     // アルファ値比較関数の設定
        GLfloat col[4];                                    // 色指定用配列
        col[0] = 5.0;
        col[1] = 5.0;
        col[2] = 5.0;
        col[3] = 1.0;
        glLightfv(GL_LIGHT0, GL_DIFFUSE, col); // 拡散反射光
        col[0] = 0.0;
        col[1] = 0.0;
        col[2] = 0.0;
        col[3] = 1.0;
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, col); // グローバル環境光の設定
        glLightfv(GL_LIGHT0, GL_AMBIENT, col);       // 環境光
        col[0] = 1.0;
        col[1] = 1.0;
        col[2] = 1.0;
        col[3] = 1.0;
        glLightfv(GL_LIGHT0, GL_SPECULAR, col); // 鏡面反射光
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0000001);

        // 観客
        for (int j = 0; j < PERSONTYPENUM; j++)
        {
            glBindTexture(GL_TEXTURE_2D, j + 100);                             // テクスチャオブジェクト生成
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // パラメータ設定
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // パラメータ設定
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage[j].cols, textureImage[j].rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, textureImage[j].data);
        }

        // アイテム
        for (int j = 0; j < ITEMTYPENUM; j++)
        {
            glBindTexture(GL_TEXTURE_2D, 200 + j);                             // テクスチャオブジェクト生成
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // パラメータ設定
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // パラメータ設定
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, itemImage[j].cols, itemImage[j].rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, itemImage[j].data);
        }
        // スコア表示
        for (int j = 0; j < SCRTYPENUM; j++)
        {
            glBindTexture(GL_TEXTURE_2D, j + 300);                             // テクスチャオブジェクト生成
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // パラメータ設定
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // パラメータ設定
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrImage[j].cols, scrImage[j].rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, scrImage[j].data);
        }

        // ナンバー
        for (int j = 0; j < NMBRTYPENUM; j++)
        {
            glBindTexture(GL_TEXTURE_2D, j + 500);                             // テクスチャオブジェクト生成
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // パラメータ設定
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // パラメータ設定
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nmbrImage[j].cols, nmbrImage[j].rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, nmbrImage[j].data);
        }

        // その他諸々
        for (int j = 0; j < 15; j++)
        {
            glBindTexture(GL_TEXTURE_2D, j);                                   // テクスチャオブジェクト生成
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // パラメータ設定
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // パラメータ設定
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tempImage[j].cols, tempImage[j].rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, tempImage[j].data);
        }

        glBindTexture(GL_TEXTURE_2D, 400);                                 // テクスチャオブジェクト生成
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // パラメータ設定
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // パラメータ設定
    }
}

// 光源配置1
void setKogen(double x, double y, double z, double d)
{
    GLfloat pos[] = {(float)x, (float)y, (float)z, (float)d};
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
}

////光源配置2
// void setKogen(double x, double y, double z, double d)
//{
//     GLfloat pos[] = {(float)x,(float) y,(float) z, (float)d};
//     glLightfv(GL_LIGHT0, GL_POSITION, pos);
// }

// 立方体配置
void putBlock(double x, double y, double z, double size, double r, double g, double b)
{
    // 色設定
    GLfloat col[] = {(float)r, (float)g, (float)b, 1.0}; // 色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
    glPushMatrix();
    glTranslated(x, y, z); // 座標
    glutSolidCube(size);   // 立方体
    glPopMatrix();
}

// 球配置
void putSphere(double x, double y, double z, double size, double r, double g, double b)
{
    // 色設定
    GLfloat col[] = {(float)r, (float)g, (float)b, 1.0}; // 色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
    glPushMatrix();
    glTranslated(x, y, z);               // 座標
    glutSolidSphere(size * 0.5, 36, 18); // 球
    glPopMatrix();
}

// 板を配置する関数
void putPlane(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b)
{
    // 色設定
    GLfloat col[] = {(float)r, (float)g, (float)b, 1.0}; // 色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);

    // 分割数
    int dX = sizeX / 100 + 1, dY = sizeY / 100 + 1;
    // 格子サイズ
    double lX = sizeX / dX, lY = sizeY / dY;

    // 配置
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotateZ, 0.0, 0.0, 1.0);
    glRotated(rotateY, 0.0, 1.0, 0.0);
    glRotated(rotateX, 1.0, 0.0, 0.0);
    glTranslated(-sizeX * 0.5, -sizeY * 0.5, 0.0);
    glScaled(sizeX / dX, sizeY / dY, 1.0);
    glNormal3d(0.0, 0.0, -1.0);
    glBegin(GL_QUADS);
    for (int i = 0; i < dX; i++)
    {
        for (int j = 0; j < dY; j++)
        {
            glVertex3d(i, j, 0.0);
            glVertex3d(i + 1, j, 0.0);
            glVertex3d(i + 1, j + 1, 0.0);
            glVertex3d(i, j + 1, 0.0);
        }
    }
    glEnd();
    glPopMatrix();
}

// 板を配置する関数
void putPlaneA(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b, double a)
{
    // 色設定
    GLfloat col[] = {(float)r, (float)g, (float)b, (float)a}; // 色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);

    // 分割数
    int dX = sizeX / 100 + 1, dY = sizeY / 100 + 1;
    // 格子サイズ
    double lX = sizeX / dX, lY = sizeY / dY;

    // 配置
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotateZ, 0.0, 0.0, 1.0);
    glRotated(rotateY, 0.0, 1.0, 0.0);
    glRotated(rotateX, 1.0, 0.0, 0.0);
    glTranslated(-sizeX * 0.5, -sizeY * 0.5, 0.0);
    glScaled(sizeX / dX, sizeY / dY, 1.0);
    glNormal3d(0.0, 0.0, -1.0);
    glBegin(GL_QUADS);
    for (int i = 0; i < dX; i++)
    {
        for (int j = 0; j < dY; j++)
        {
            glVertex3d(i, j, 0.0);
            glVertex3d(i + 1, j, 0.0);
            glVertex3d(i + 1, j + 1, 0.0);
            glVertex3d(i, j + 1, 0.0);
        }
    }
    glEnd();
    glPopMatrix();
}

// 板を配置する関数
void putPlaneT(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b, int texID)
{
    glEnable(GL_TEXTURE_2D);             // テクスチャ有効化
    glBindTexture(GL_TEXTURE_2D, texID); // テクスチャ"texID"番の呼び出し
    // 色設定
    GLfloat col[] = {(float)r, (float)g, (float)b, 1.0}; // 色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);

    // 分割数
    int dX = sizeX / 100 + 1, dY = sizeY / 100 + 1;
    // 格子サイズ
    double lX = sizeX / dX, lY = sizeY / dY;

    // 配置
    glPushMatrix();
    glTranslated(x, y, z);
    glTranslated(0.0, -sizeY * 0.5, 0.0);
    glRotated(rotateZ, 0.0, 0.0, 1.0);
    glRotated(rotateY, 0.0, 1.0, 0.0);
    glRotated(rotateX, 1.0, 0.0, 0.0);
    glTranslated(-sizeX * 0.5, 0.0, 0.0);
    glScaled(sizeX / dX, sizeY / dY, 1.0);
    glNormal3d(0.0, 0.0, -1.0);
    glBegin(GL_QUADS);
    for (int i = 0; i < dX; i++)
    {
        for (int j = 0; j < dY; j++)
        {
            glTexCoord2d(1.0 * (dX - i) / dX, 1.0 * (dY - j) / dY);
            glVertex3d(i, j, 0.0);
            glTexCoord2d(1.0 * (dX - i - 1) / dX, 1.0 * (dY - j) / dY);
            glVertex3d(i + 1, j, 0.0);
            glTexCoord2d(1.0 * (dX - i - 1) / dX, 1.0 * (dY - j - 1) / dY);
            glVertex3d(i + 1, j + 1, 0.0);
            glTexCoord2d(1.0 * (dX - i) / dX, 1.0 * (dY - j - 1) / dY);
            glVertex3d(i, j + 1, 0.0);
        }
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D); // テクスチャ無効化
}

// 板を配置する関数A
void putPlaneTA(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ, double r, double g, double b, double a, int texID)
{
    glEnable(GL_TEXTURE_2D);             // テクスチャ有効化
    glBindTexture(GL_TEXTURE_2D, texID); // テクスチャ"texID"番の呼び出し
    // 色設定
    GLfloat col[] = {(float)r, (float)g, (float)b, (float)a}; // 色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);

    // 分割数
    int dX = sizeX / 100 + 1, dY = sizeY / 100 + 1;
    // 格子サイズ
    double lX = sizeX / dX, lY = sizeY / dY;

    // 配置
    glPushMatrix();
    glTranslated(x, y, z);
    glTranslated(0.0, -sizeY * 0.5, 0.0);
    glRotated(rotateZ, 0.0, 0.0, 1.0);
    glRotated(rotateY, 0.0, 1.0, 0.0);
    glRotated(rotateX, 1.0, 0.0, 0.0);
    glTranslated(-sizeX * 0.5, 0.0, 0.0);
    glScaled(sizeX / dX, sizeY / dY, 1.0);
    glNormal3d(0.0, 0.0, -1.0);
    glBegin(GL_QUADS);
    for (int i = 0; i < dX; i++)
    {
        for (int j = 0; j < dY; j++)
        {
            glTexCoord2d(1.0 * (dX - i) / dX, 1.0 * (dY - j) / dY);
            glVertex3d(i, j, 0.0);
            glTexCoord2d(1.0 * (dX - i - 1) / dX, 1.0 * (dY - j) / dY);
            glVertex3d(i + 1, j, 0.0);
            glTexCoord2d(1.0 * (dX - i - 1) / dX, 1.0 * (dY - j - 1) / dY);
            glVertex3d(i + 1, j + 1, 0.0);
            glTexCoord2d(1.0 * (dX - i) / dX, 1.0 * (dY - j - 1) / dY);
            glVertex3d(i, j + 1, 0.0);
        }
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D); // テクスチャ無効化
}

// 板を配置する関数（格子模様）
void putPlane2(double x, double y, double z, double sizeX, double sizeY, double rotateX, double rotateY, double rotateZ)
{
    // 色設定
    GLfloat col1[] = {1.0, 0.2, 0.2, 1.0}; // 色
    GLfloat col2[] = {0.8, 0.8, 0.8, 1.0}; // 色

    // 分割数
    int dX = sizeX / 100 + 1, dY = sizeY / 100 + 1;
    double centerX = (dX - 1) / 2.0;
    // 格子サイズ
    double lX = sizeX / dX, lY = sizeY / dY;

    // 配置
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotateZ, 0.0, 0.0, 1.0);
    glRotated(rotateY, 0.0, 1.0, 0.0);
    glRotated(rotateX, 1.0, 0.0, 0.0);
    glTranslated(-sizeX * 0.5, -sizeY * 0.5, 0.0);
    glScaled(sizeX / dX, sizeY / dY, 1.0);
    glNormal3d(0.0, 0.0, -1.0);
    glBegin(GL_QUADS);
    for (int i = 0; i < dX; i++)
    {
        for (int j = 0; j < dY; j++)
        {
            /*
             if ((i+j)%2==0)
             glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col1);
             else
             glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col2);
             */
            if (fabs(i - centerX) < 3)
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col1);
            else
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col2);
            glVertex3d(i, j, 0.0);
            glVertex3d(i + 1, j, 0.0);
            glVertex3d(i + 1, j + 1, 0.0);
            glVertex3d(i, j + 1, 0.0);
        }
    }
    glEnd();
    glPopMatrix();
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        // クリックしたマウス座標を(mX, mY)に格納
        mX = x;
        mY = y;
    }
}

// マウスドラッグコールバック
void motion(int x, int y)
{
    // マウスのx方向の移動(mX-x)：水平角の変化
    eDegY = eDegY + (mX - x) * 0.5;
    if (eDegY > 360)
        eDegY -= 360;
    if (eDegY < -0)
        eDegY += 360;

    // マウスのy方向の移動(y-mY)：垂直角の変化
    eDegX = eDegX + (y - mY) * 0.5;
    if (eDegX > 80)
        eDegX = 80;
    if (eDegX < -80)
        eDegX = -80;

    // 現在のマウス座標を(mX, mY)に格納
    mX = x;
    mY = y;
}

// キーボードコールバック
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {            // キーボードで入力した文字keyの値
    case 27:     //[esc]キー
        exit(0); // 終了
        break;

    case 'm':
        playerMoveFlg = 1 - playerMoveFlg; // playerMoveFlgを0⇄1で変化させる
        break;

    case '<':
        theta += 10.0;
        if (theta > 80)
            theta = 80;
        break;

    case '>':
        theta -= 10.0;
        if (theta < -80)
            theta = -80;
        break;

    case 'h':
        // hosei = val[5];
        hoseiFlg = 1;
        hoseiCnt = 0;
        hoseiSum = 0;
        break;

    case 'r':
        gameReset();
        break;

    case 'g':
        photoImage.release();
        photoSaveFlg = 0;
        if (gameMode == 4 || gameMode == 5)
            gameMode = 0;
        else
            gameMode++;

        if (gameMode == 3)
        {
            playerMoveFlg = 1;
            alSourcef(sourceBGM, AL_GAIN, 0.4); // 音量設定 0.4
        }
        else
            playerMoveFlg = 0;

        if (gameMode == 0)
        {
            gameReset();
            alSourcef(sourceBGM, AL_GAIN, 0.0); // 音量設定
        }
        if (gameMode == 4)
            alSourcePlay(sourcePanel);

        // hoseiFlg = 0;
        theta = 0.0;

        break;

    default:
        break;
    }
}
