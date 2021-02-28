#define _WINSOCK_DEPRECATED_NO_WARNINGS 


#include "display.h"
//#pragma comment(lib,"legacy_stdio_definitions.lib")

char host[16] = "127.0.0.1";
int port = 5628;
bool init_vertical = false;
int time_start = 0;




//cam координаты камеры, direct сферические координаты, angleCam поворот камеры, deltaCam временная переменная, deltaMove временно для движения
textureData* LoadBMP(const char* imagepath)
{
	Sleep(100);
	// Данные, прочитанные из заголовка BMP-файла
	unsigned char header[54]; // Каждый BMP-файл начинается с заголовка, длиной в 54 байта
	unsigned int dataPos;     // Смещение данных в файле (позиция данных)
	unsigned int width, height;
	unsigned int imageSize;   // Размер изображения = Ширина * Высота * 3
	// RGB-данные, полученные из файла
	unsigned char* data;
	FILE* file = fopen(imagepath, "rb");
	if (!file) {
		printf("Изображение не может быть открыто\n");
		return 0;
	}
	if (fread(header, 1, 54, file) != 54) { // Если мы прочитали меньше 54 байт, значит возникла проблема
		printf("Некорректный BMP-файл\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Некорректный BMP-файл\n");
		return 0;
	}
	// Читаем необходимые данные
	dataPos = *(int*) & (header[0x0A]); // Смещение данных изображения в файле
	imageSize = *(int*) & (header[0x22]); // Размер изображения в байтах
	width = *(int*) & (header[0x12]); // Ширина
	height = *(int*) & (header[0x16]); // Высота
	// Некоторые BMP-файлы имеют нулевые поля imageSize и dataPos, поэтому исправим их
	if (imageSize == 0)    imageSize = width * height * 3; // Ширину * Высоту * 3, где 3 - 3 компоненты цвета (RGB)
	if (dataPos == 0)      dataPos = 54; // В таком случае, данные будут следовать сразу за заголовком
	// Создаем буфер
	data = new unsigned char[imageSize];
	int a=ftell(file);
	fseek(file, dataPos, SEEK_SET);
	a = ftell(file);
	// Считываем данные из файла в буфер
	fread(data, 1, imageSize, file);

	// Закрываем файл, так как больше он нам не нужен
	fclose(file);
	textureData* p = new textureData();
	p->sizeX = width;
	p->sizeY = height;
	p->data = data;
	return p;
}
void Reshape(int w, int h) {
	// предотвращение деления на ноль
	int width = w, height = h;
	if (h == 0)
		h = 1;
	float ratio = w * 1 / h;
	// используем матрицу проекции

	glMatrixMode(GL_PROJECTION);
	// обнуляем матрицу
	glLoadIdentity();
	//glViewport(-(1000 - w) / 2, -(1000 - h) / 2, w / h * 1000, 1000);
	glViewport(0, 0, w, h);
	// установить параметры вьюпорта

	// установить корректную перспективу
	gluPerspective(45, 1, 0.1, 100); 
	// вернуться к матрице проекции
	glMatrixMode(GL_MODELVIEW);
}
point_structure parse_YAML_point(char * str)
{
	point_structure point;
	point.a = { 0,0,0 };
	point.coord = { 0,0,0 };
	point.q = { { 0,0,0 }, 1};
	point.t = 0;
	memset(point.str, 0, sizeof(char)* EXTRA_LINE_LENGTH);
	char buftmp[32];
	char* pointer[5];
	char* sptr = 0;
	pointer[0] = strstr(str, "str:");
	if (pointer[0])//сначала обрабатываем str, ибо в ней может содержаться все чо угодно
	{
		sptr = strchr(pointer[0], '\"');
		if (sptr == 0 || strchr(sptr + 1, '\"') == 0)
		{
			strcpy(point.str, "#!error: there isn't \" symbols after str mark");
			return point;
		}
		strncpy(point.str, sptr + 1, strchr(sptr + 1, '\"')-(sptr + 1));
		for (int i = 1; sptr[i] != '\"'; i++)
			sptr[i] = ' ';
	}
	pointer[1] = strstr(str, "a:");
	pointer[2] = strstr(str, "q:");
	pointer[3] = strstr(str, "coord:");
	pointer[4] = strstr(str, "t:");
	if (pointer[1]) { pointer[1][-1] = 0; pointer[1] += 2; }//a
	if (pointer[2]) { pointer[2][-1] = 0; pointer[2] += 2; }//q
	if (pointer[3]) { pointer[3][-1] = 0; pointer[3] += 6; }//coord
	if (pointer[4]) { pointer[4][-1] = 0; pointer[4] += 2; }//t
	for(int h=1;h<4;h++)
		if (pointer[h])	//обработка маркеров a: q: coord:
		{
			int n = strlen(pointer[h]);
			xyz_structure* xyzptr = 0;
			float* floatPtrForQuaternion=0;
			switch (h)
			{
			case 1: xyzptr = &point.a; break;//a
			case 2: xyzptr = &point.q.vec; floatPtrForQuaternion = &point.q.w; break;//q
			case 3: xyzptr = &point.coord; break;//coord
			}
			for (int i = 0; i < n; i++)
			{
				while (i < n && ((pointer[h][i] != 'x' && pointer[h][i] != 'y' && pointer[h][i] != 'z' && pointer[h][i] != 'w') || pointer[h][i + 1] != ':'))
					i++;
				sptr = pointer[h] + i + 2;
				if (i >= n)break;
				float* fptr = 0;
				switch (pointer[h][i])
				{
				case 'x':fptr = &xyzptr->x; break;
				case 'y':fptr = &xyzptr->y; break;
				case 'z':fptr = &xyzptr->z; break;
				case 'w':if(h==2)fptr = floatPtrForQuaternion; break;
				}
				while (i < n && pointer[h][i] != ',' && pointer[h][i] != '}' && pointer[h][i] != '\n')
					i++;
				pointer[h][i] = 0;
				sscanf(sptr, "%f", fptr);
			}
			
		}
	if (pointer[4])	// обработка маркера t:
	{
		int i = 0;
		while (pointer[4][i]!=0 && pointer[4][i] != ',' && pointer[4][i] != '}' && pointer[4][i] != '\n')
			i++;
		pointer[4][i] = 0;
		sscanf(pointer[4], "%f", &point.t);
	}
	return point;
	exit_parse_YAML_point:
	strcpy(point.str, "#!error: bad point");
	return { {0,0,0}, {0,0,0}, {0,0,0}, -1, "error: bad point" };
}

bool socketClosed = false;
const int arr_size = 2048;//для выделения памяти
bool endOfSocket(void* pParam) {
	return socketClosed;
}
void getLineFromSocket(void* pParam, char* buff, int len) {
	
	static char localBuff[arr_size];
	char* tem;
	while ((tem=strchr(localBuff, '\n')) == 0 && strchr(localBuff, EOF) == 0)
		if (recv(*((SOCKET*)pParam), localBuff+strlen(localBuff), len, 0) == 0) {
			socketClosed = true;
		}
	
	strncpy(buff, localBuff, tem - localBuff + 1);
	strncpy(localBuff, tem + 1, arr_size - (tem + 1 - localBuff));
	fwrite(buff, 1, tem - localBuff + 1, stdout);
	return ;
}
bool endOfFile(void* pParam) {
	return feof((FILE*)pParam);
}
void getLineFromFile(void* pParam, char* buff, int len) {
	
	fgets(buff, len, (FILE*)pParam);
	return;
}
void readBuffer(void *pParam) {
	void (*getLine)(void*,char*,int)=0;
	bool (*endOfData)(void*) = 0;
	void* parameter = 0;
	
	static char buf[arr_size];
	if (pParam == 0) {
		WSADATA wsaData;
		// Initialize Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return ;
		}
		endOfData = endOfSocket;
		getLine = getLineFromSocket;
		struct sockaddr_in serv_addr;
		SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
		parameter = (void*)&(sockfd);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(5629);

		serv_addr.sin_addr.S_un.S_addr;
		serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

		if (connect(sockfd, (struct sockaddr*) & serv_addr, sizeof(serv_addr)))
		{
			printf("\nERROR, can't connect to server");
			return;
		}
		printf("connection!\n");
	}
	else {
		endOfData = endOfFile;
		getLine = getLineFromFile;
		parameter = (void*)fopen((char*)pParam, "r");
		if (parameter == 0)
		{
			printf("\nERROR, can't open file");
			return;
		}
	}
	


	
	//int arr_size_count = 1;


	//char* buf = (char*)malloc((arr_size + 1) * sizeof(char));	//
	//memset(buf, 0, (arr_size + 1) * sizeof(char));

	int string_counter = 0;     //чтобы выводить на какой строчке ошибка
	int space_counter = 0;		//для правильной табуляции
	int local_space_counter = 0;
	int tmp = 0;	// просто промежуточные вычисления
	getLine(parameter,buf,arr_size);
	string_counter++;
	for (local_space_counter = 0;
		(buf[local_space_counter] == ' ' || buf[local_space_counter] == '\t') && buf[local_space_counter] != EOF && buf[local_space_counter] != 0;
		local_space_counter++);
	while (!endOfData(parameter))
	{
		space_counter = local_space_counter;
		while ((strchr(buf, ':') == 0 || buf[space_counter] == 0) && !endOfData(parameter))		// поиск ключа траектории и подсчет пробелов
		{
			getLine(parameter,buf,arr_size);
			
			string_counter++;
			for (space_counter = 0;
				(buf[space_counter] == ' ' || buf[space_counter] == '\t') && buf[space_counter] != EOF && buf[space_counter] != 0;
				space_counter++);
		}



		if (buf[space_counter] == EOF)break;	// файл заканчивается пустой строкой либо строкой с пробелами
		if (buf[space_counter] == 0)continue;   //строка с пробелами


		for (tmp = strchr(buf, ':') - buf - 1; tmp >= 0 && (buf[tmp] == ' ' || buf[tmp] == '\t'); tmp--);
		if (tmp >= 0)
			if ((tmp - space_counter + 1) < 64)
			{
				strncpy(track[number_of_track].name, buf + space_counter, tmp - space_counter + 1);
				track[number_of_track].name[tmp - space_counter + 1] = 0;
			}
			else
			{
				strncpy(track[number_of_track].name, buf + space_counter, 63);
				track[number_of_track].name[63] = 0;
			}
		memset(buf, 0, sizeof(char) * strlen(buf));


		char* pointer = buf; //для перемещения по буферу
		do {//считывания всего блока данных, определяя границу по табуляции

			pointer = pointer + strlen(pointer);
			//if ((pointer - buf) >= arr_size)
			//{
			//	printf("error, so much character\n");
			//	return ;
			//}
			if (!strchr(pointer, '\n') && !strchr(pointer,EOF))
				getLine(parameter, pointer, arr_size - (pointer - buf));
			string_counter++;
			for (local_space_counter = 0;
				(pointer[local_space_counter] == ' ' || pointer[local_space_counter] == '\t') && pointer[local_space_counter] != EOF && pointer[local_space_counter] != 0;
				local_space_counter++);
			if (pointer[0] == 'x')
				printf("debug");
			//if (local_space_counter <= space_counter)
			//	printf("debug");
			if (pointer[local_space_counter] == EOF)               break;	// файл заканчивается пустой строкой либо строкой с пробелами
			if (pointer[local_space_counter] == 0 && !endOfData(parameter)) continue;   //строка с пробелами
			tmp = pointer - buf + local_space_counter;
			if (pointer[local_space_counter] == '-' || local_space_counter <= space_counter || endOfData(parameter))
			{
				if (buf[tmp] == '-' && (strchr(buf, '-') == pointer + local_space_counter))
					continue;						//первое считывание
				else
					if (pointer == buf)
						break;						//первое считывание

				while (buf[tmp] != '\n')
					tmp--;
				if (track[number_of_track].number == track[number_of_track].size_of_array)//  растягиваемое выделение памяти
				{
					track[number_of_track].size_of_array += JUMP_FOR_DYNAMIC_ARRAY_OF_POINTS;
					track[number_of_track].points = (point_structure*)realloc(track[number_of_track].points, sizeof(point_structure) * track[number_of_track].size_of_array);
				}

				buf[tmp] = 0;			// подготовка к парсингу точки
				strchr(buf, '-')[0] = ' ';
				track[number_of_track].points[track[number_of_track].number] = parse_YAML_point(buf);

				if (track[number_of_track].points[track[number_of_track].number].str[0] == '#' && track[number_of_track].points[track[number_of_track].number].str[1] == '!')
					printf("%s . Before %d string\n", track[number_of_track].points[track[number_of_track].number].str, string_counter);
				else {
					if(!track[number_of_track].active)
						track[number_of_track].active = true;
					if (track[number_of_track].index == track[number_of_track].number - 1)
						track[number_of_track].index++;
					track[number_of_track].number++;
					
				}
				//for (int i = 0; buf[tmp + i] != 0; i++)
				//	buf[i] = buf[tmp + i];
				tmp++;
				strcpy(buf, buf + tmp);
				pointer = buf;
			}



		} while (!endOfData(parameter) && local_space_counter > space_counter && pointer[local_space_counter] != EOF);
		if (track[number_of_track].number > 0)
		{
			track[number_of_track].active = true;
			track[number_of_track].index = track[number_of_track].number;
			number_of_track++;
		}

	}
	//free(buf);

	//global_i = current_i;
	return;
}
int ReadData(const char* pParam)	// считывание данных из файла, возвращает не ноль если успешно
{							// формат данных - YAML
	if (pParam[0] == 's' && pParam[1] == 0)
	{
		_beginthread(readBuffer, 0, 0);
	}
	else
		_beginthread(readBuffer, 0, (void *)pParam);
	return 1;
}
void keyboard_char(unsigned char key, int x, int y) 
{
	keychar[key] = true;
	switch (key) {
	
	//case '0':current_i = 2; real_time = false; break;
	case 'm':
	case 'M':draw_map_var = !draw_map_var; break;
	case 'l':
	case 'L': show_line = !show_line;
	case 'r':
	case 'R':for (int i = 0; i < TRACK_MAX; i++) if (track[i].active) track[i].color = { 1,0,0 }; break;
	case 'g':
	case 'G':for (int i = 0; i < TRACK_MAX; i++) if (track[i].active) track[i].color = { 0,1,0 }; break;
	case 'b':
	case 'B':for (int i = 0; i < TRACK_MAX; i++) if (track[i].active) track[i].color = { 0,0,1 }; break;
	case 'u':
	case 'U':for (int i = 0; i < TRACK_MAX; i++) if (track[i].active) track[i].color = { 0.5, 0.5, 0.5 }; break;
	case '0': if (keychar['c'] || keychar['C'])track[0].active = !track[0].active; if(keychar['h'] || keychar['H'])track[0].show = !track[0].show; break;
	case '1': if (keychar['c'] || keychar['C'])track[1].active = !track[1].active; if(keychar['h'] || keychar['H'])track[1].show = !track[1].show; break;
	case '2': if (keychar['c'] || keychar['C'])track[2].active = !track[2].active; if(keychar['h'] || keychar['H'])track[2].show = !track[2].show; break;
	case '3': if (keychar['c'] || keychar['C'])track[3].active = !track[3].active; if(keychar['h'] || keychar['H'])track[3].show = !track[3].show; break;
	case '4': if (keychar['c'] || keychar['C'])track[4].active = !track[4].active; if(keychar['h'] || keychar['H'])track[4].show = !track[4].show; break;
	case '5': if (keychar['c'] || keychar['C'])track[5].active = !track[5].active; if(keychar['h'] || keychar['H'])track[5].show = !track[5].show; break;
	case '6': if (keychar['c'] || keychar['C'])track[6].active = !track[6].active; if(keychar['h'] || keychar['H'])track[6].show = !track[6].show; break;
	case '7': if (keychar['c'] || keychar['C'])track[7].active = !track[7].active; if(keychar['h'] || keychar['H'])track[7].show = !track[7].show; break;
	case '8': if (keychar['c'] || keychar['C'])track[8].active = !track[8].active; if(keychar['h'] || keychar['H'])track[8].show = !track[8].show; break;
	case '9': if (keychar['c'] || keychar['C'])track[9].active = !track[9].active; if(keychar['h'] || keychar['H'])track[9].show = !track[9].show; break;


	}
}
void keyboard_char_up(unsigned char key, int x, int y) 
{
	keychar[key] = false;
	
}
void keyboard_int(int key, int xx, int yy) 
{
	keyint[key] = true;
}
void keyboard_int_up(int key, int x, int y) 
{
	keyint[key] = false;
}

void keyboardParse() //обработка всех кнопок управления с клавиатуры
{
	

	//переметка траектории, отслеживание 
	//if (keychar['1'] || keychar['2'] || keychar['3'])real_time = false;
	if (!keychar['h'] && !keychar['H'] && !keychar['c'] && !keychar['C'])//left alt
		delta_i = (keychar['1']*(-100)+ keychar['2'] * (-10) + keychar['3'] * (-1) + keychar['4'] * (1) + keychar['5'] * (10) + keychar['6'] * (100));
	//вращение карты вокруг центра
	if (keychar[244] || keychar[212] || keychar['A'] || keychar['a'])mapAngle += deltaMove.x;
	if (keychar[194] || keychar[226] || keychar['D'] || keychar['d']) mapAngle -= deltaMove.x;
	if (mapAngle > 360)mapAngle -= 360;
	if (mapAngle < -360)mapAngle += 360;
	//перемещение камеры от центра и к центру
	if (keychar[251] || keychar[219] || keychar['S'] || keychar['s']) cam.y += deltaMove.y;
	if (keychar[214] || keychar[246] || keychar['W'] || keychar['w']) cam.y -= deltaMove.y;
	if (cam.y > 100) cam.y -= deltaMove.y;
	if (cam.y < 0) cam.y += deltaMove.y;
	//перемещение камеры вверх вниз 
	if (keychar['q'] || keychar['Q']) cam.z += deltaMove.z;
	if (keychar['e'] || keychar['E']) cam.z -= deltaMove.z;
	if (cam.z > 100) cam.z -= deltaMove.z;
	if (cam.z < -20) cam.z += deltaMove.z;
	//вращение камеры вокруг своих локальных осей
	if (keyint[GLUT_KEY_UP])   angleCam.x += deltaCam.x;
	if (keyint[GLUT_KEY_DOWN]) angleCam.x -= deltaCam.x;
	if (keyint[GLUT_KEY_RIGHT])angleCam.z -= deltaCam.z;
	if (keyint[GLUT_KEY_LEFT]) angleCam.z += deltaCam.z;
	if (angleCam.x >  1.57) angleCam.x -= deltaCam.x;
	if (angleCam.x < -1.57) angleCam.x += deltaCam.x;
	direct.y = -cos(angleCam.x) * cos(angleCam.z);
	direct.z = sin(angleCam.x);
	direct.x = cos(angleCam.x) * sin(angleCam.z);

}
void timef(int value) {
	keyboardParse();
	glutPostRedisplay();  // перерисовывает окно

	glutTimerFunc(40, timef, 0); // рекурсия(нет)
}
void mause(int button, int state, int x, int y) {
	//printf("%d %d %d %d\n", button, state, x, y);
}
int main(int argc, char** argv) {
	
	float k = 50;
	FILE* fp;
	for (int i = 0; i < TRACK_MAX; i++)
	{
		track[i].number = 0;
		track[i].size_of_array = 1;
		track[i].index = 0;
		track[i].show = 1;
		track[i].active = 0;
		track[i].color = { 0.5, 0.5, 0.5 };
		track[i].points = (point_structure*)realloc(track[i].points, sizeof(point_structure) * track[i].size_of_array);
	}
	fp = fopen("angleNorth.txt", "r");
	if (fp != 0)
	{
		fscanf(fp, "%f", &mapAngleNorth);
		fclose(fp);
	}
	char str[56];
	printf("read data from file [filename] or connect to socket server [s]?\n");
	scanf("%s", str);
	if (str[0] == 's' && str[1] == 0) {
		printf("default host and port is %s:%d\n",host,port);
		printf("host:");
		fgets(str, 56, stdin);
		strcpy(host, str);
		printf("port:");
		fgets(str, 56, stdin);
		port=atoi(str);
		ReadData("s");
	}
	else {
		ReadData(str);
	}

	// инициализация GLUT и создание окна
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("Trajectory");
	InitGL();
	glutDisplayFunc(Display);//рисовалка
	glutReshapeFunc(Reshape);

	glutTimerFunc(20, timef, 0);
	glutSpecialFunc(keyboard_int);//для обработки нажатий стрелок
	glutSpecialUpFunc(keyboard_int_up);//для обработки отжатий стрелок
	glutKeyboardFunc(keyboard_char);//для обработки нажатий символов (w,a,s,d)
	glutKeyboardUpFunc(keyboard_char_up);//для обработки отжатий символов (w,a,s,d)
										 //glutIgnoreKeyRepeat(0);// что это блин?
	glutMouseFunc(mause);
	

	glEnable(GL_DEPTH_TEST);// OpenGL - инициализация функции теста
	srand(time(0));


	glutMainLoop();
	return 0;
}