#define _CRT_SECURE_NO_WARNINGS 1

#include "display.h"

GLuint texture[3];//массив для хранения текстурок
				  //94.95 метра 
const xyz_structure deltaCam = { 0.025,0.025,0.025 }, deltaMove = { 0.64,0.2,0.2 };
bool keychar[256];
bool keyint[256];
trajectory track[16];
int number_of_track = 0;
float mapAngleNorth = 0;
float mapAngle = 0;
//float deltaAngleMap = 0;
float verticalAngleNorth = 0;
float horizontalAngleNorth = 0;
int delta_i = 0;
xyz_structure cam = { 0,5,1 }, direct = { 0,0,-1 }, angleCam = { 0,0,0 }, angle_rate = { 0,0,0 };
//bool real_time = true;
bool show_line = true;
bool draw_map_var = true;
//int current_i = 0;
//int global_i = 0;


GLvoid LoadGLTextures()
{
	glEnable(GL_TEXTURE_2D);// Разрешение наложение текстуры
	glGenTextures(3, &texture[0]);

	textureData* textureA[3];
	
	if (textureA[0] = LoadBMP("map.bmp"))
	{
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textureA[0]->sizeX, textureA[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureA[0]->data);
		free(textureA[0]->data);
		free(textureA[0]);
	}
	if (textureA[1] = LoadBMP("white.bmp"))
	{
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textureA[1]->sizeX, textureA[1]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureA[1]->data);
		free(textureA[1]->data);
		free(textureA[1]);
	}
	if (textureA[2] = LoadBMP("font-texture.bmp"))
	{
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textureA[2]->sizeX, textureA[2]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureA[2]->data);
		free(textureA[2]->data);
		free(textureA[2]);
	}
	




}
GLvoid InitGL()
{
	LoadGLTextures();// Загрузка текстур
	glEnable(GL_TEXTURE_2D);// Разрешение наложение текстуры
	glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

void draw_text(const char* str_for_character,int len) {
	for (int i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str_for_character[i]);
}
void draw_text(const char* str_for_character) {
	draw_text(str_for_character, strlen(str_for_character));
}
void draw_symbols()
{
	glPushMatrix();
	glTranslated(0, 0, -0.1);
	glColor3f(0, 0, 0);
	glRasterPos3f(-0.0004, 0, 0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, '+');
	
	char str_for_character[128];
	sprintf(str_for_character, "%0.3f", (float)track[0].points[track[0].index].t / 1000.0);
	glRasterPos3f(0.015, -0.02, 0);
	draw_text(str_for_character, strlen(str_for_character));

	sprintf(str_for_character, "x: %+5f y: %+5f z: %+5f", track[0].points[track[0].index].coord.x, track[0].points[track[0].index].coord.y, track[0].points[track[0].index].coord.z);
	glRasterPos3f(-0.03, 0.02, 0);
	draw_text("coord: ");
	draw_text(str_for_character, strlen(str_for_character));
	
	sprintf(str_for_character, "x: %+5f y: %+5f z: %+5f", track[0].points[track[0].index].a.x, track[0].points[track[0].index].a.y, track[0].points[track[0].index].a.z);
	glRasterPos3f(-0.03, 0.025, 0);
	draw_text("accel: ");
	draw_text(str_for_character, strlen(str_for_character));

	xyz_structure tempvec = track[0].points[track[0].index].q.vec;
	double ac = acos(track[0].points[track[0].index].q.w);
	float temp = sin(ac);
	sprintf(str_for_character, "x: %+5f y: %+5f z: %+5f w: %+5f all: %+5f ", tempvec.x, tempvec.y , tempvec.z , track[0].points[track[0].index].q.w, sqrt(tempvec.x* tempvec.x+ tempvec.y * tempvec.y + tempvec.z * tempvec.z)/temp);
	glRasterPos3f(-0.03, 0.03, 0);
	draw_text("quatr: ");
	draw_text(str_for_character, strlen(str_for_character));
	
	glPopMatrix();
}
void draw_map() {
	glPushMatrix();
	glRotatef(mapAngleNorth + mapAngle, 0, 0, 1);
	// нарисуем "землю"
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(1,0);
	//glTexxyz_structure2f(1, 0); 
	glVertex3f(27.929 - 0.5, 24.929 - 0.5, 0); //право низ 
	glTexCoord2f(0, 0);
	//glTexxyz_structure2f(0, 0); 
	glVertex3f(-23.929 - 0.5, 24.929 - 0.5, 0);//лево  Низ
	glTexCoord2f(0, 1);
	//glTexxyz_structure2f(0, 1); 
	glVertex3f(-23.929 - 0.5, -26.929 - 0.5, 0); //лево Верх 
	glTexCoord2f(1, 1);
	//glTexxyz_structure2f(1, 1); 
	glVertex3f(27.929 - 0.5, -26.929 - 0.5, 0);  //право Верх 
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glPopMatrix();
}
void draw_lines()
{
	glPushMatrix();
	glRotatef(mapAngleNorth + mapAngle, 0, 0, 1);
	glBegin(GL_LINES);
	glColor3d(0, 0, 1);
	for (int i = -30; i < 30; i++)
		for (int j = -30; j < 30; j++)
			glVertex3f(i, -30, 0), glVertex3f(i, 30, 0), glVertex3f(-30, j, 0), glVertex3f(30, j, 0);
	glEnd();
	glPopMatrix();
	glPushMatrix();
	glRotatef(mapAngle, 0, 0, 1);
	glBegin(GL_LINES);
	glColor3f(0, 0, 0);
	glVertex3d(0, 0.5, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0.5, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 0.5);
	glVertex3d(0, 0, 0);
	glEnd();
	glPopMatrix();
}
void calculateScene()
{
	for (int h = 0; h < TRACK_MAX; h++)
	{
		if (track[h].active)
			track[h].index += delta_i;
		if (track[h].index < 0)
			track[h].index = 0;
		if (track[h].index >= track[h].number && track[h].number!=0) {
			track[h].index = track[h].number-1;
		}
	}
}
void Display(void) {
	calculateScene();
	glClearColor(0.3, 0.8, 0.8, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//очистить буфер цвета и глубины
	glLoadIdentity();// обнулить трансформацию



	draw_symbols();
	gluLookAt(cam.x, cam.y, cam.z,
		cam.x + direct.x, cam.y + direct.y, cam.z + direct.z,
		0, 0, 1);// установить камеру 
				 /*gluLookAt(cam.x, cam.y, cam.z,
				 cam.x + magnito[global_i-1].z, cam.y - magnito[global_i-1].y, cam.z + magnito[global_i-1].z,
				 0, 1, 0);*/



	glLineWidth(1);
	if (show_line)
		draw_lines();
	if (draw_map_var)
		draw_map();


	glRotatef(mapAngle, 0, 0, 1);
	glPushMatrix();
	
	glColor3f(1, 0, 0);

	glLineWidth(2);
	for (int h = 0; h < TRACK_MAX; h++)
	{
		if (track[h].show) {
			glBegin(GL_LINE_STRIP);

			glColor3f(track[h].color.x, track[h].color.y, track[h].color.z);
			for (int i = 0; i < track[h].index; i++)
			{
				
				glVertex3f(track[h].points[i].coord.x, track[h].points[i].coord.y, track[h].points[i].coord.z);
				
			}

			glEnd();





			glPushMatrix();
			
			//temp = temp * temp;
			//temp = 1 - temp;
			//temp = sqrt(temp);


			double ac = acos(track[h].points[track[h].index].q.w);
			float temp = sin(ac);
			xyz_structure tempvec= track[h].points[track[h].index].q.vec;
			float tempw = 2 * ac * 180.0 / 3.141592;

			glTranslatef(track[h].points[track[h].index].coord.x, track[h].points[track[h].index].coord.y, track[h].points[track[h].index].coord.z);
			glRotatef(tempw, tempvec.x / temp, tempvec.y / temp, tempvec.z/temp );


			glBegin(GL_QUADS);
			glVertex3f(0.2, 0.1, 0);
			glVertex3f( -0.2, 0.1, 0);
			glVertex3f( -0.2, -0.1, 0 );
			glVertex3f( 0.2, -0.1, 0);
			glEnd();
			glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(1, 0, 0);
			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 1, 0);
			glColor3f(0, 0, 1);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 1);
			glColor3f(1, 1, 0);
			
			glEnd();
			//glVertex3f(track[h].points[track[h].index].q.vec.x/temp  ,track[h].points[track[h].index].q.vec.y / temp, track[h].points[track[h].index].q.vec.z / temp);
			
			
			glPopMatrix();
			
			glBegin(GL_LINES);
			glColor3f(0, 1, 1);
			tempvec = track[h].points[track[h].index].a;
			glVertex3f(0, 0, 0);
			glVertex3f(tempvec.x * 4, tempvec.y * 4, tempvec.z * 4);
			glEnd();
		}
	}
	




	glPopMatrix();


	glutSwapBuffers();
}