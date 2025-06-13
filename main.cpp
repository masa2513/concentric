// g++ -O3 main.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -I/opt/homebrew/include -L/opt/homebrew/lib -lalut -framework OpenAL -Wno-deprecated
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
int mButton, mState, mX, mY;  // マウス系の変数
double fr = 60.0;                                   // フレームレート
int winID[2];                                       // ウィンドウID
int winW[2], winH[2];                               // ウィンドウサイズ

time_t timeX;
tm *local;

// main関数
int main(int argc, char *argv[])
{

    srand((unsigned)time(NULL));

    glutInit(&argc, argv); // OpenGL初期化

    initGL(); // OpenGL初期設定

    gameReset();

    glutMainLoop(); // イベント待ち無限ループ

    return 0; // 戻り値
}

// ゲームリセット
void gameReset()
{
    // プレイヤーの（初期）位置
}

// シーン表示
void scene()
{
    // 光源の位置指定
    setKogen(0.0, 0.0, 0.0, 1.0);
}

// タイマーコールバック
void timer(int val1)
{
    system("clear");

    glutSetWindow(winID[0]);
    glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバックの再指定

    glutSetWindow(winID[1]);
    glutPostRedisplay();

    //----------ここに毎フレーム実施する内容を記述----------
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
    double eX = eDist * cos(M_PI * eDegX / 180.0) * sin(M_PI * eDegY / 180.0);
    double eY = eDist * sin(M_PI * eDegX / 180.0);
    double eZ = eDist * cos(M_PI * eDegX / 180.0) * cos(M_PI * eDegY / 180.0);
    gluLookAt(eX, eY, eZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 視点座標(x,y,z)，注視点座標(x,y,z)，上方向(x,y,z)

    // シーン表示
    scene();

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
}

// 光源配置1
void setKogen(double x, double y, double z, double d)
{
    GLfloat pos[] = {(float)x, (float)y, (float)z, (float)d};
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
}

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

    mX = x; mY = y; mButton = button; mState = state;
}

// マウスドラッグコールバック
void motion(int x, int y)
{
    if(mButton == GLUT_LEFT_BUTTON){
        eDegY += (mX - x)*0.5;
        eDegX += (y - mY)*0.5;
        //マウス座標をグローバル変数に保存
        mX = x; mY = y;
    }
    if(mButton == GLUT_RIGHT_BUTTON){
        eDist += (y - mY)*0.5;
        //マウス座標をグローバル変数に保存
        mX = x; mY = y;
    }
}

// キーボードコールバック
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {            // キーボードで入力した文字keyの値
    case 27:     //[esc]キー
        exit(0); // 終了
        break;

    case 'r':
        gameReset();
        break;

    default:
        break;
    }
}
