/**************************************--仕様--***************************************
	■説明
	マーカ「Hiro」「Sample1」「Kanji」を認識して別のオブジェクトを表示させるプログラム
	マーカ間の距離・角度の差をコンソールで見ることができる。
*************************************************************************************/

#pragma warning(disable:4819)

#include <windows.h>
#include <cstdio>
#include <cstdlib>

#define _USE_MATH_DEFINES	// math.hのM_PIを使うため
#include <math.h>			// 角度計算用

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <AR/ar.h>
#include <AR/param.h>
#include <AR/video.h>
#include <AR/gsub.h>
#include <mmsystem.h>
#include "GLMetaseq.h"

#include <iostream>
#include <algorithm>

#pragma comment(lib,"winmm.lib")

// グローバル変数
/* カメラ構成 */
char *vconf_name  = "Data/WDM_camera_flipV.xml";	// ビデオデバイスの設定ファイル
int  xsize;											// ウィンドウサイズ
int  ysize;											// ウィンドウサイズ
int  thresh = 80;									// 2値化の閾値
int  count = 0;										// 処理フレーム数

MQO_MODEL	model[8];		// モデル
MQO_MODEL   rmodel;

/* カメラパラメータ */
char *cparam_name = "Data/camera_para.dat";			// カメラパラメータファイル
ARParam cparam;	// カメラパラメータ

HMIDIOUT g_hMidi;

char	*mqo_name[8]		={ "Data/mqo_model/onpumurasaki","Data/mqo_model/onpuai.mqo","Data/mqo_model/onpuao.mqo","Data/mqo_model/onpumidori.mqo","Data/mqo_model/onpuki.mqo","Data/mqo_model/onpudaidai.mqo","Data/mqo_model/onpuaka.mqo","Data/mqo_model/onpukuro.mqo"};
char    *mqo_rnm={"Data/mqo_model/hunassi.mqo"};
// MQOファイル

/* オブジェクトの回転角度 */
GLfloat		exRz = 0;		// z軸まわりの回転角度
GLfloat     rm=0;       //ランダム角度

// オブジェクトの移動距離
GLfloat		movex[8] = {0};
GLfloat		movey[8] = {0};
GLfloat		movez[8] = {0};
GLfloat		rmove={0};



/* パターンファイル */
#define MARK_NUM		8						// 使用するマーカーの個数
//-----
#define MARK1_MARK_ID	1						// マーカーID
#define MARK1_PATT_NAME	"Data/marker/patt.do2"		// パターンファイル名
#define MARK1_SIZE		70.0					// パターンの幅（80mm）
//-----
#define MARK2_MARK_ID	2						// マーカーID
#define MARK2_PATT_NAME	"Data/marker/patt.re"	// パターンファイル名
#define MARK2_SIZE		70.0					// パターンの幅（80mm）
//-----
#define MARK3_MARK_ID	3						// マーカーID
#define MARK3_PATT_NAME	"Data/marker/patt.mi"		// パターンファイル名
#define MARK3_SIZE		70.0					// パターンの幅（80mm）
//-----
#define MARK4_MARK_ID	4						// マーカーID
#define MARK4_PATT_NAME	"Data/marker/patt.fa"		// パターンファイル名
#define MARK4_SIZE		70.0					// パターンの幅（80mm）
//-----
#define MARK5_MARK_ID	5						// マーカーID
#define MARK5_PATT_NAME	"Data/marker/patt.so2"		// パターンファイル名
#define MARK5_SIZE		70.0					// パターンの幅（80mm）
//-----
#define MARK6_MARK_ID	6						// マーカーID
#define MARK6_PATT_NAME	"Data/marker/patt.ra"		// パターンファイル名
#define MARK6_SIZE		70.0					// パターンの幅（80mm）
//-----
#define MARK7_MARK_ID	7						// マーカーID
#define MARK7_PATT_NAME	"Data/marker/patt.si"		// パターンファイル名
#define MARK7_SIZE		70.0					// パターンの幅（80mm）
//-----
#define MARK8_MARK_ID	8						// マーカーID
#define MARK8_PATT_NAME	"Data/marker/takaido.pat"		// パターンファイル名
#define MARK8_SIZE		70.0					// パターンの幅（80mm）
//-----

typedef struct {
	char   *patt_name;			// パターンファイル
	int    patt_id;				// パターンのID
	int    mark_id;				// マーカーID
	bool   visible;				// 検出フラグ
	double patt_width;			// パターンのサイズ（単位：ｍｍ）
	double patt_center[2];		// パターンの中心座標
	double patt_trans[3][4];	// 座標変換行列
} MARK_T;
//-----
MARK_T   marker[MARK_NUM] = {
	{MARK1_PATT_NAME, -1, MARK1_MARK_ID, 0, MARK1_SIZE, {0.0, 0.0}},
	{MARK2_PATT_NAME, -1, MARK2_MARK_ID, 0, MARK2_SIZE, {0.0, 0.0}},
	{MARK3_PATT_NAME, -1, MARK3_MARK_ID, 0, MARK3_SIZE, {0.0, 0.0}},
	{MARK4_PATT_NAME, -1, MARK4_MARK_ID, 0, MARK4_SIZE, {0.0, 0.0}},
	{MARK5_PATT_NAME, -1, MARK5_MARK_ID, 0, MARK5_SIZE, {0.0, 0.0}},
	{MARK6_PATT_NAME, -1, MARK6_MARK_ID, 0, MARK6_SIZE, {0.0, 0.0}},
	{MARK7_PATT_NAME, -1, MARK7_MARK_ID, 0, MARK7_SIZE, {0.0, 0.0}},
	{MARK8_PATT_NAME, -1, MARK8_MARK_ID, 0, MARK8_SIZE, {0.0, 0.0}}
};


// プロトタイプ宣言
void Init(void);
void MainLoop(void);
void SetupLighting1(void);
void SetupLighting2(void);
void SetupMaterial1(void);
void SetupMaterial2(void);
void KeyEvent( unsigned char key, int x, int y );
void MouseEvent( int button, int state, int x, int y );
void Cleanup(void);
void DrawObject( int mark_id, double (&patt_trans)[3][4] ,int i);
void Drawnashi( int mark_id,double (&patt_trans)[3][4]);
static void	  midi_out(int flg);
static void   midi_stop(int flg);
static int    midi_init(void);

//=======================================================
// main関数
//=======================================================
int main( int argc, char **argv )
{
	// GLUTの初期化
	glutInit( &argc, argv );

	// ARアプリケーションの初期化
	Init();

	midi_init();
	// ビデオキャプチャの開始
	arVideoCapStart();

	// メインループの開始
	argMainLoop( MouseEvent, KeyEvent, MainLoop );


	return 0;
}


//=======================================================
// 初期化関数
//=======================================================
void Init(void)
{
	ARParam wparam;		// カメラパラメータ

	// ビデオデバイスの設定
	if( arVideoOpen( vconf_name ) < 0 ){
		printf("ビデオデバイスのエラー\n");
		exit(0);
	}

	// ウィンドウサイズの取得
	if( arVideoInqSize( &xsize, &ysize ) < 0 ) exit(0);
	printf("Image size (x,y) = (%d,$d)\n", xsize, ysize);

	// カメラパラメータの設定
	if( arParamLoad( cparam_name, 1, &wparam ) < 0 ){
		printf("カメラパラメータの読み込みに失敗しました\n");
		exit(0);
	}

	// カメラパラメータのサイズ調整
	arParamChangeSize( &wparam, xsize, ysize, &cparam );
	// カメラパラメータの初期化
	arInitCparam( &cparam );
	printf("*** Camera Parameter ***\n");
	arParamDisp( &cparam );

	// パターンファイルのロード
	for( int i=0; i<MARK_NUM; i++ ){
		if( (marker[i].patt_id = arLoadPatt(marker[i].patt_name)) < 0){
			printf("パターンファイルの読み込みに失敗しました\n");
			printf("%s\n", marker[i].patt_name);
			exit(0);
		}
	}

	// gsubライブラリの初期化
	argInit( &cparam, 1.0, 0, 0, 0, 0 );

	mqoInit(); //メタセコイアの初期化

	// モデルの読み込み

	for(int i=0;i<8;i++){
		if ( (model[i] = mqoCreateModel( mqo_name[i], 1.0 )) == NULL ) {
				printf("モデルの読み込みに失敗しました\n");
			//	return -1;
		}
	}
	if ( (rmodel = mqoCreateModel( mqo_rnm, 1.0 )) == NULL ) {
		printf("モデルの読み込みに失敗しました\n");
		//	return -1;
	}


	// ウィンドウタイトルの設定
	glutSetWindowTitle("PaperPiano");
}


//=======================================================
// メインループ関数
//=======================================================
void MainLoop(void)
{
	ARUint8          *image;			// カメラキャプチャ画像
	ARMarkerInfo     *marker_info;		// マーカ情報
	int              marker_num;		// 検出されたマーカの数
	int              i, j, k;

	// カメラ画像の取得
	if( (image = (ARUint8 *)arVideoGetImage()) == NULL ){
		arUtilSleep( 2 );
		return;
	}
	if( count == 0 ) arUtilTimerReset();
	count++;

	// カメラ画像の描画
	argDrawMode2D();
	argDispImage( image, 0, 0 );


	// マーカの検出と認識
	if( arDetectMarker( image, thresh, &marker_info, &marker_num ) < 0 ){
		Cleanup();
		exit(0);
	}

	// 次の画像のキャプチャ指示
	arVideoCapNext();

	// 3Dオブジェクトを描画するための準備
	argDrawMode3D();
	argDraw3dCamera( 0, 0 );
	glClearDepth(1.0);					// デプスバッファの消去値
	glClear( GL_DEPTH_BUFFER_BIT );		// デプスバッファの初期化

	if(movex[0]!=0 && movex[3]!=0 && movex[7]!=0){
		rmove++;
		
		if(rmove!=0){
			Drawnashi( marker[3].mark_id, marker[3].patt_trans);
		}
		if(rmove>40.0){
			rmove=0.0;
			for(int i=0;i<MARK_NUM;i++){
				movex[i]=0;
			}
		}
	}else{
		// マーカの一致度の比較
		for( i=0; i<MARK_NUM; i++ ){
			k = -1;
			for( j=0; j<marker_num; j++ ){
				if( marker[i].patt_id == marker_info[j].id ){
					if( k == -1 ) k = j;
					else if( marker_info[k].cf < marker_info[j].cf ) k = j;
				}
			}

			// マーカーが見つからなかったとき
			if( k == -1 ){
				if(marker[i].visible != 0){
					midi_out(i+1);
					midi_stop(i+1);
					movex[i]=1;
					marker[i].visible = 0;
				}else if(movex[i]!=0){
					DrawObject( marker[i].mark_id, marker[i].patt_trans,i );
				}
			}else{
				// 座標変換行列を取得
				if( marker[i].visible == 0 ) {
					// 1フレームを使ってマーカの位置・姿勢（座標変換行列）の計算
					arGetTransMat( &marker_info[k], marker[i].patt_center, marker[i].patt_width, marker[i].patt_trans );    //初回の認識ではarGetTransMatを2回目以降ではarGetTransMatContを使うと安定するらしい
					marker[i].visible = 1;

				} else {
					// 前のフレームを使ってマーカの位置・姿勢（座標変換行列）の計算
					arGetTransMatCont( &marker_info[k], marker[i].patt_trans, marker[i].patt_center, marker[i].patt_width, marker[i].patt_trans );
				}

				// 3Dオブジェクトの描画
				if(movex[i]!=0){
					DrawObject( marker[i].mark_id, marker[i].patt_trans,i );
				}
			}
			if(movex[i]>=40.0) movex[i]=0;
			if(movex[i]!=0) movex[i]++;
		}
	}
	// バッファの内容を画面に表示
	argSwapBuffers();
}


//=======================================================
// 3Dオブジェクトの描画を行う関数
//=======================================================
void DrawObject( int mark_id,double (&patt_trans)[3][4] ,int i)
{
	double gl_para[16];	// ARToolKit->OpenGL変換行列

	// 陰面消去
	glEnable( GL_DEPTH_TEST );			// 陰面消去・有効
	glDepthFunc( GL_LEQUAL );			// デプステスト

	// 変換行列の適用
	argConvGlpara( patt_trans, gl_para );	// ARToolKitからOpenGLの行列に変換
	glMatrixMode( GL_MODELVIEW );			// 行列変換モード・モデルビュー
	glLoadMatrixd( gl_para );				// 読み込む行列を指定

	switch( mark_id ){
		case MARK1_MARK_ID:
			// ライティング
			SetupLighting1();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial1();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i] );				// モデルの描画
			glPopMatrix();

			break;

		case MARK2_MARK_ID:
			// ライティング
			SetupLighting2();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial2();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i] );				// モデルの描画
			glPopMatrix();


			break;

		case MARK3_MARK_ID:
			// ライティング
			SetupLighting1();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial2();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i] );				// モデルの描画
			glPopMatrix();


			break;
		case MARK4_MARK_ID:
			// ライティング
			SetupLighting1();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial1();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i]);				// モデルの描画
			glPopMatrix();

			break;

		case MARK5_MARK_ID:
			// ライティング
			SetupLighting2();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial2();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i] );				// モデルの描画
			glPopMatrix();

			break;
		case MARK6_MARK_ID:
			// ライティング
			SetupLighting1();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial2();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i] );				// モデルの描画
			glPopMatrix();

			break;

		case MARK7_MARK_ID:
			// ライティング
			SetupLighting1();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial1();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i] );				// モデルの描画
			glPopMatrix();

			break;

			case MARK8_MARK_ID:
			// ライティング
			SetupLighting1();			// ライトの定義
			glEnable( GL_LIGHTING );	// ライティング・有効
			glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
			SetupMaterial1();

			// 3Dオブジェクトの描画
			if (movex[i] < 20.0){
			movez[i] = -pow(10.0-movex[i],2.0)+100;
			}else{
			movez[i] = -pow(10.0-(movex[i]-20.0),2.0)+100;
			}

			/* 3次元オブジェクトの描画 */
			glTranslatef(-5*movex[i], 0.0 , 25+movez[i]);		// オブジェクトの平行移動（z軸方向に25mm移動）
			glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

			glPushMatrix();
				glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
				mqoCallModel( model[i] );				// モデルの描画
			glPopMatrix();

			break;
	}
	// 終了処理
	glDisable( GL_LIGHTING );		// ライティング・無効
	glDisable( GL_DEPTH_TEST );		// デプステスト・無効
}

void Drawnashi( int mark_id,double (&patt_trans)[3][4]){
	double gl_para[16];	// ARToolKit->OpenGL変換行列

	// 陰面消去
	glEnable( GL_DEPTH_TEST );			// 陰面消去・有効
	glDepthFunc( GL_LEQUAL );			// デプステスト

	// 変換行列の適用
	argConvGlpara( patt_trans, gl_para );	// ARToolKitからOpenGLの行列に変換
	glMatrixMode( GL_MODELVIEW );			// 行列変換モード・モデルビュー
	glLoadMatrixd( gl_para );				// 読み込む行列を指定

	SetupLighting1();			// ライトの定義

	glEnable( GL_LIGHTING );	// ライティング・有効
	glEnable( GL_LIGHT0 );		// ライト0・オン
			// オブジェクトの材質
	SetupMaterial1();

			// 3Dオブジェクトの描画
	GLfloat rmovez;

	if (rmove < 20.0){
	rmovez = -pow(10.0-rmove,2.0)+100;
		}else{
	rmovez = -pow(10.0-(rmove-20.0),2.0)+100;
		}

			/* 3次元オブジェクトの描画 */
	glTranslatef(5*rmove, 0.0 , 25+rmovez);		// オブジェクトの平行移動（z軸方向に25mm移動）
	glRotatef(90, 0.0, 0.0, 1.0);		// オブジェクトをz軸まわりに回転 元はｙが０でｚが１だった

	glPushMatrix();
		glRotatef( 90, 1.0, 0.0, 0.0 );	// モデルを立たせる　もとは始めが90だったから立ってた
		mqoCallModel( rmodel );				// モデルの描画
	glPopMatrix();

	glDisable( GL_LIGHTING );		// ライティング・無効
	glDisable( GL_DEPTH_TEST );		// デプステスト・無効
}


//=======================================================
// ライティング
//=======================================================
void SetupLighting1(void)
{
	// ライトの定義
	GLfloat lt0_position[] = {100.0, -200.0, 200.0, 0.0};	// ライト0の位置
	GLfloat lt0_ambient[]  = {0.1, 0.1, 0.1, 1.0};			// 　　　　 環境光
	GLfloat lt0_diffuse[]  = {0.8, 0.8, 0.8, 1.0};			// 　　　　 拡散光

	// ライトの設定
	glLightfv( GL_LIGHT0, GL_POSITION, lt0_position );
	glLightfv( GL_LIGHT0, GL_AMBIENT, lt0_ambient );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, lt0_diffuse );
}

void SetupLighting2(void)
{
	// ライトの定義
	GLfloat lt0_position[] = {100.0, 200.0, 200.0, 0.0};	// ライト0の位置
	GLfloat lt0_ambient[]  = {0.2, 0.2, 0.2, 1.0};			// 　　　　 環境光
	GLfloat lt0_diffuse[]  = {0.8, 0.8, 0.8, 1.0};			// 　　　　 拡散光

	// ライトの設定
	glLightfv( GL_LIGHT0, GL_POSITION, lt0_position );
	glLightfv( GL_LIGHT0, GL_AMBIENT, lt0_ambient );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, lt0_diffuse );
}


//=======================================================
// マテリアルの設定
//=======================================================
void SetupMaterial1(void)
{
	// オブジェクトの材質
	GLfloat mat_ambient[] = {0.0, 1.0, 1.0, 1.0};	// 材質の環境光
	GLfloat mat_specular[] = {0.0, 0.0, 1.0, 1.0};	// 鏡面光
	GLfloat mat_shininess[] = {50.0};				// 鏡面係数

	// マテリアルの設定
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void SetupMaterial2(void)
{
	// オブジェクトの材質
	GLfloat mat_ambient[] = {0.0, 0.0, 1.0, 1.0};	// 材質の環境光
	GLfloat mat_specular[] = {0.0, 0.0, 1.0, 1.0};	// 鏡面光
	GLfloat mat_shininess[] = {50.0};				// 鏡面係数

	// マテリアルの設定
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}


//=======================================================
// キーボード入力処理関数
//=======================================================
void KeyEvent( unsigned char key, int x, int y )
{
	// ESCキーを入力したらアプリケーション終了
	if (key == 0x1b ){
		printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
		Cleanup();
		exit(0);
	}
}


//=======================================================
// マウス入力処理関数
//=======================================================
void MouseEvent( int button, int state, int x, int y )
{
	// 入力状態を表示
	printf("ボタン：%d 状態：%d 座標：(x,y)=(%d,%d) \n", button, state, x, y );
}


//=======================================================
// 終了処理関数
//=======================================================
void Cleanup(void)
{
	arVideoCapStop();	// ビデオキャプチャの停止
	arVideoClose();		// ビデオデバイスの終了
	argCleanup();		// ARToolKitの終了処理
}

static void midi_out(int flg){
	if(flg==1) midiOutShortMsg(g_hMidi, 0x00403c90);       //ド　on
	else if(flg==2) midiOutShortMsg(g_hMidi, 0x01403e90);  //レ　on
	else if(flg==3) midiOutShortMsg(g_hMidi, 0x02404090);  //ミ　on
	else if(flg==4) midiOutShortMsg(g_hMidi, 0x03404190);  //ファon
	else if(flg==5) midiOutShortMsg(g_hMidi, 0x04404390);  //ソ　on
	else if(flg==6) midiOutShortMsg(g_hMidi, 0x05404590);  //ラ  on
	else if(flg==7) midiOutShortMsg(g_hMidi, 0x06404790);  //シ　on
	else if(flg==8) midiOutShortMsg(g_hMidi, 0x07404890);  //ド　on
}

static void midi_stop(int flg){
	if(flg==1) midiOutShortMsg(g_hMidi, 0xB07800c0);         //ド　off
	else if(flg==2) midiOutShortMsg(g_hMidi, 0xB17800c0);    //レ　off
	else if(flg==3) midiOutShortMsg(g_hMidi, 0xB27800c0);  //ミ　off
	else if(flg==4) midiOutShortMsg(g_hMidi, 0xB37800c0);  //ファoff
	else if(flg==5) midiOutShortMsg(g_hMidi, 0xB47800c0);  //ソ　off
	else if(flg==6) midiOutShortMsg(g_hMidi, 0xB57800c0);  //ラ  off
	else if(flg==7) midiOutShortMsg(g_hMidi, 0xB67800c0);  //シ　off
	else if(flg==8) midiOutShortMsg(g_hMidi, 0xB77800c0);  //ド　off
}
static int midi_init(void){

	MIDIHDR mhMidi;

	/* GMリセット用データ */
	BYTE abyGMReset[] = {0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7};


	/* MIDIデバイスオープン */
	if (midiOutOpen(&g_hMidi, MIDIMAPPER, 0, 0, 0) != MMSYSERR_NOERROR) {
		return 0;
	}

	ZeroMemory(&mhMidi, sizeof(mhMidi));

	/* GMリセット送信用バッファ設定 */
	mhMidi.lpData = (LPSTR)abyGMReset;
	mhMidi.dwBufferLength = 6;
	mhMidi.dwBytesRecorded = 6;
	midiOutPrepareHeader(g_hMidi, &mhMidi, sizeof(mhMidi));

	/* GMリセットメッセージ送信 */
	if (midiOutLongMsg(g_hMidi, &mhMidi, sizeof(mhMidi)) != MMSYSERR_NOERROR) {

		midiOutUnprepareHeader(g_hMidi, &mhMidi, sizeof(mhMidi));

		std::cout<<"MIDI音源の初期化に失敗しました。"<<std::endl;
		midiOutClose(g_hMidi);

		return 0;

	}

		/* GMリセット完了待機 */
		while ((mhMidi.dwFlags & MHDR_DONE) == 0);

		midiOutUnprepareHeader(g_hMidi, &mhMidi, sizeof(mhMidi));


		return 0;
}
