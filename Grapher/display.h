#define _CRT_SECURE_NO_WARNINGS 1
#define TRACK_MAX 16
#define JUMP_FOR_DYNAMIC_ARRAY_OF_POINTS 1000;
#define EXTRA_LINE_LENGTH 64
#include <GL/glut.h>
#pragma comment(lib,"freeglut.lib")
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")


#include <process.h>
#include <time.h>
#include <math.h>

struct xyz_structure {
	float x;
	float y;
	float z;
};
struct quaternion {
	xyz_structure vec;
	float w;
};
struct point_structure {
	xyz_structure coord;			// координаты точки

	xyz_structure a;				// вектор ускорения					 // дополнительнные данные
	quaternion q;				// вектор направления				 // выводятся на экран
	float t;						// таймер							 // не влияют на вид траектории
	char str[EXTRA_LINE_LENGTH];					// дополнительная строка комментариев//

};
// структура траектории, ее состояние на экранее и данные в каждой точке, 
struct trajectory {
	
	int number;				// количество точек
	int size_of_array;		//длинна массива, для динамического выделения
	int index;				// индекс, задающий текущее положение при перемотке
	point_structure* points;// массив точек с данными
	xyz_structure start_position;// координаты старта
	xyz_structure color;
	bool active;			// выбрана ли текущая траектория активной	1 да, 0 нет
	bool show;				// показывать траекторию					1 да, 0 нет
	char name[64];
};
// структура точки,


extern const xyz_structure  deltaCam , deltaMove ;
struct textureData
{
	GLsizei sizeX;
	GLsizei sizeY;
	GLubyte* data;
};

extern trajectory track[TRACK_MAX];
extern int number_of_track;
extern GLuint texture[3];// массив для хранения текстурок
				  //94.95 метра 
extern bool keychar[256];
extern bool keyint[256]; 
extern float mapAngleNorth;
extern float mapAngle;
//extern float deltaAngleMap;
extern float verticalAngleNorth;
extern float horizontalAngleNorth;
extern int delta_i;
extern xyz_structure cam, direct, angleCam, angle_rate;
//extern bool real_time;
extern bool show_line;
extern bool draw_map_var;
//extern int current_i;
//extern int global_i;

textureData* LoadBMP(const char* imagepath);
GLvoid LoadGLTextures();
GLvoid InitGL();
void draw_map();
void draw_symbols();
void draw_lines();
void calculateScene();
void Display(void);