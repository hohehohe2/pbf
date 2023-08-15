#include <GL/glut.h>
#include "draw.h"





// 視点操作のための変数
float  camera_pitch_x = 0.0;   // Y軸を軸とするカメラの回転角度
float  camera_pitch_y = 0.0;  // Ｘ軸を軸とするカメラの回転角度

float camera_x = 0.0;
float camera_y = 0.0;

// マウスのドラッグのための変数
int  drag_mouse_l = 0;  // 右ボタンをドラッグ中かどうかのフラグ（0:非ドラッグ中,1:ドラッグ中）
int  zoom_mouse_l = 0;  // 右ボタンをドラッグ中かどうかのフラグ（0:非ドラッグ中,1:ドラッグ中）
int  last_mouse_x;      // 最後に記録されたマウスカーソルのＸ座標
int  last_mouse_y;      // 最後に記録されたマウスカーソルのＹ座標
double  zoom = -30;

//
//  ウィンドウ再描画時に呼ばれるコールバック関数
//
void  display( void )
{
    // 画面をクリア（ピクセルデータとＺバッファの両方をクリア）
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // 変換行列を設定（ワールド座標系→カメラ座標系）
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( camera_x, camera_y, zoom );
    glRotatef(-camera_pitch_y, 1.0, 0.0, 0.0);
    glRotatef(-camera_pitch_x, 0.0, 1.0, 0.0);

    // 光源位置を設定（モデルビュー行列の変更にあわせて再設定）
    float  light0_position[] = { 10.0, 10.0, 10.0, 1.0 };
    glLightfv( GL_LIGHT0, GL_POSITION, light0_position );

    glEnable(GL_DEPTH_TEST);
    draw();
    glDisable(GL_DEPTH_TEST);

    // バックバッファに描画した画面をフロントバッファに表示
    glutSwapBuffers();
}


//
//  ウィンドウサイズ変更時に呼ばれるコールバック関数
//
void  reshape( int w, int h )
{
    // ウィンドウ内の描画を行う範囲を設定（ウィンドウ全体に描画するように設定）
    glViewport(0, 0, w, h);

    // カメラ座標系→スクリーン座標系への変換行列を設定
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45, (double)w/h, 1, 500 );
}


//
//  マウスクリック時に呼ばれるコールバック関数
//
void  mouse( int button, int state, int mx, int my )
{
    // 右ボタンが押されたらドラッグ開始のフラグを設定
    if ( ( button == GLUT_LEFT_BUTTON ) && ( state == GLUT_DOWN ) )
        drag_mouse_l = 1;
    // 右ボタンが離されたらドラッグ終了のフラグを設定
    else if ( ( button == GLUT_LEFT_BUTTON ) && ( state == GLUT_UP ) )
        drag_mouse_l = 0;

    // 右ボタンが押されたらドラッグ開始のフラグを設定
    if ( ( button == GLUT_RIGHT_BUTTON ) && ( state == GLUT_DOWN ) )
        zoom_mouse_l = 1;
    // 右ボタンが離されたらドラッグ終了のフラグを設定
    else if ( ( button == GLUT_RIGHT_BUTTON ) && ( state == GLUT_UP ) )
        zoom_mouse_l = 0;

    // 現在のマウス座標を記録
    last_mouse_x = mx;
    last_mouse_y = my;
}

void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 'j':
        camera_x -= 1;
        break;
    case 'k':
        camera_x += 1;
        break;
    case 'i':
        camera_y += 1;
        break;
    case 'm':
        camera_y -= 1;
        break;
    default:
        step();
    }
    display();
}

//
//  マウスドラッグ時に呼ばれるコールバック関数
//
void  motion( int mx, int my )
{
    // 右ボタンのドラッグ中であれば、マウスの移動量に応じて視点を回転する
    if ( drag_mouse_l == 1 )
    {
        // マウスの縦移動に応じてＸ軸を中心に回転
        camera_pitch_y -= ( my - last_mouse_y ) * 1.0;
        if ( camera_pitch_y < -90.0 )
            camera_pitch_y = -90.0;

        // マウスの縦移動に応じてY軸を中心に回転
        camera_pitch_x -= ( mx - last_mouse_x ) * 1.0;
        if ( camera_pitch_x < -90.0 )
            camera_pitch_x = -90.0;
    }

    if ( zoom_mouse_l == 1 )
    {
        zoom += my - last_mouse_y;
    }

    // 今回のマウス座標を記録
    last_mouse_x = mx;
    last_mouse_y = my;

    // 再描画の指示を出す（この後で再描画のコールバック関数が呼ばれる）
    glutPostRedisplay();
}


//
//  アイドル時に呼ばれるコールバック関数
//
void  idle( void )
{
    // 現在は、何も処理を行なわない
}


//
//  環境初期化関数
//
void  initEnvironment( void )
{
    // 光源を作成する
    float  light0_position[] = { 10.0, 10.0, 10.0, 1.0 };
    float  light0_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    float  light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    float  light0_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
    glLightfv( GL_LIGHT0, GL_POSITION, light0_position );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, light0_diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, light0_specular );
    glLightfv( GL_LIGHT0, GL_AMBIENT, light0_ambient );
    glEnable( GL_LIGHT0 );

    // 光源計算を有効にする
    glEnable( GL_LIGHTING );

    // 物体の色情報を有効にする
    glEnable( GL_COLOR_MATERIAL );

    // Ｚテストを有効にする
    glEnable( GL_DEPTH_TEST );

    // 背面除去を有効にする
    //glCullFace( GL_BACK );
    //glEnable( GL_CULL_FACE );

    // 背景色を設定
    glClearColor( 1, 1, 1, 0.0 );
}


//
//  メイン関数（プログラムはここから開始）
//
int  main( int argc, char ** argv )
{
    // GLUTの初期化
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    glutInitWindowSize( 640, 640 );
    glutInitWindowPosition( 0, 0 );
    glutCreateWindow( "OpenGL & GLUT sample program" );

    // コールバック関数の登録
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutMouseFunc( mouse );
    glutKeyboardFunc( keyboard );
    glutMotionFunc( motion );
    glutIdleFunc( idle );

    // 環境初期化
    initEnvironment();

    initialize();

    // GLUTのメインループに処理を移す
    glutMainLoop();
    
    // プログラムを終了
    return 0;
}
