#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define TO_RADIAN		0.017453292519943296 

int rightbuttonpressed = 0;
int leftbuttonpressed = 0, center_selected = 0;

float r, g, b; // Background color
float px, py, qx, qy; // Line (px, py) --> (qx, qy)
int n_object_points = 6;
float object[6][2], object_center_x, object_center_y;
// PIKACHU
float pikachu[147][2] =
{ {-9, -8}, {-14, -5}, {-15, -3}, {-15, -2}, {-15, -1}, {-15, 1}, {-14, 3},
{-13, 6}, {-13, 10}, {-12, 13}, {-11, 16}, {-8, 20}, {-7, 19}, {-7, 12},
{-8, 9}, {-9, 8}, {-8, 9}, {3, 8}, {1, 7}, {3, 8}, {13, 11}, {18, 11}, {17, 10},
{12, 7}, {6, 5}, {4, 5}, {5, 5}, {5, -4}, {4, -5}, {3, -6}, {2, -7}, {0, -7},
{4, -5}, {4, -7}, {5, -9}, {6, -11}, {7, -11}, {8, -12}, {7, -13}, {9, -11},
{10, -11}, {10, -14}, {8, -18}, {7, -19}, {6, -19}, {6, -17}, {6, -19}, {-1, -19},
{2, -15}, {3, -12}, {3, -11}, {-1, -19}, {0, -20}, {-6, -20}, {-5, -19},
{-5, -14}, {-4, -10}, {-6, -11}, {-5, -14}, {-5, -19}, {-7, -20}, {-10, -20},
{-10, -19}, {-9, -18}, {-12, -14}, {-13, -11}, {-13, -9}, {-10, -19}, {-17, -19},
{-19, -16}, {-20, -13}, {-20, -12}, {-19, -11}, {-18, -11}, {-17, -13},
{-16, -16}, {-17, -13}, {-17, -12}, {-15, -11}, {-14, -9}, {-13, -7}, {-13, -6},
{-11, -1}, {-10, -1}, {-9, -4}, {-7, -6}, {-6, -6}, {-4, -4}, {-3, -2}, {-3,-1},
{-2, -1}, {-4, -1}, {-6, 0}, {-8, 0}, {-9, -1}, {-11, -1}, {5, 1}, {10, 3},
{15, 4}, {20, 4}, {15, -4}, {8, -4}, {10, -9}, {6, -10}, {6, -11}, {5, -9},
{7, -8}, {5, -4}, {-15, 0}, {-14, 0}, {-13, -1}, {-13, -2}, {-14, -3}, {-15,-3},
{0, -2}, {1, -1}, {3, -1}, {4, -2}, {4, -3}, {3, -4}, {1, -4}, {0, -3}, {0,-2},
{-11, 1}, {-12, 2}, {-12, 3}, {-11, 4}, {-10, 4}, {-9, 3}, {-9, 2}, {-10, 1},
{-11, 1}, {-2, 2}, {-2, 3}, {-1, 4}, {0, 4}, {1, 3}, {1, 2}, {0, 1}, {-1, 1},
{-2, 2}, {-7, 1}, {12, 7}, {13, 11}, {-12, 13}, {-9, 15}, {-7, 15} };
int pikachu_center_x, pikachu_center_y;
float pikachu_size = 0.03;
int n_pikachu_points = 147;
float pikachu_x = -0.5, pikachu_y = -0.5;
float pikachu_sh_x = 0;
// PIKACHU

float rotation_angle_in_degree;
int window_width, window_height;
int wheelMoving = 0;
int dx, dy;

float xCoord(float x) {
	return (x - window_width / 2.0f) / 250.0f;
}

float yCoord(float y) {
	return (-y + window_height / 2.0f) / 250.0f;
}


void draw_axes() {
	glLineWidth(3.0f);
	glBegin(GL_LINES);			// 이제부터 그린다!
	glColor3f(1.0f, 0.0f, 0.0f); // 빨간색으로 설정!
	glVertex2f(0.0f, 0.0f);		// 빨간색 선의 시작점(원점)
	glVertex2f(1.0f, 0.0f);		// 빨간색 선의 끝점
	glVertex2f(0.975f, 0.025f); // 빨간색 화살표의 끝부분 (1)
	glVertex2f(1.0f, 0.0f);		// 빨간색 화살표의 끝부분 (1)
	glVertex2f(0.975f, -0.025f); // 빨간색 화살표의 끝부분 (2)
	glVertex2f(1.0f, 0.0f);		// 빨간색 화살표의 끝부분 (2)

	glColor3f(0.0f, 1.0f, 0.0f); // 초록색으로 설정!
	glVertex2f(0.0f, 0.0f);		// 초록색 선의 시작점
	glVertex2f(0.0f, 1.0f);		// 초록색 선의 끝점
	glVertex2f(0.025f, 0.975f);	// 화살표 함수의 끝부분 (1)
	glVertex2f(0.0f, 1.0f);		// 화살표 함수의 끝부분 (1)
	glVertex2f(-0.025f, 0.975f);// 화살표 함수의 끝부분 (2)
	glVertex2f(0.0f, 1.0f);		// 화살표 함수의 끝부분 (2)
	glEnd();					// 그리는 것 끝낸다!
	glLineWidth(1.0f);			// 선의 굵기 설정

	glPointSize(7.0f);			// 점 크기 설정
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);
	glEnd();
	glPointSize(1.0f);
}
void draw_line(float px, float py, float qx, float qy) {

	glBegin(GL_LINES);		// 이제부터 그린다!
	glColor3f(1.0f, 0.0f, 0.0f);	// 빨간색으로 설정!
	glVertex2f(px, py);		// 빨간색 선의 시작점
	glVertex2f(qx, qy);		// 빨간색 선의 끝점
	glEnd();

	glPointSize(5.0f);	// 점 크기 설정
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(px, py);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(qx, qy);

	glEnd();
	glPointSize(1.0f);
}

void draw_object(void) {
	// 중심점 계산
	float sum_x = 0, sum_y = 0;
	for (int i = 0; i < n_object_points; i++) {
		sum_x += object[i][0];
		sum_y += object[i][1];
	}
	object_center_x = sum_x / n_object_points;
	object_center_y = sum_y / n_object_points;

	// 다각형 그리기
	glBegin(GL_LINE_LOOP);
	glColor3f(0.0f, 1.0f, 0.0f);
	for (int i = 0; i < n_object_points; i++)
		glVertex2f(object[i][0], object[i][1]);
	glEnd();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < n_object_points; i++)
		glVertex2f(object[i][0], object[i][1]);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(object_center_x, object_center_y);
	glEnd();
	glPointSize(1.0f);
}

void draw_pikachu_line(int st, int n, float r, float g, float b, int mode=GL_LINE_STRIP) {
	glBegin(mode);
	glColor3f(r, g, b);
	for (int i = st; i < st + n; i++) {
		glVertex2f(pikachu[i][0] * pikachu_size + pikachu_sh_x*pikachu[i][1] + pikachu_x,
			pikachu[i][1] * pikachu_size+pikachu_y);
	}
	glColor3f(r, g, b);
	glEnd();
}

void draw_pikachu() {
	// 몸통
	draw_pikachu_line(0, 32, 1.0f, 1.0f, 0.0f);   // 1 (32)
	draw_pikachu_line(32, 19, 1.0f, 1.0f, 0.0f);  // 2 (19)
	draw_pikachu_line(51, 6, 1.0f, 1.0f, 0.0f);   // 3 (6)
	draw_pikachu_line(57, 10, 1.0f, 1.0f, 0.0f);  // 4 (10)
	draw_pikachu_line(67, 15, 1.0f, 1.0f, 0.0f);  // 5 (15)
	draw_pikachu_line(82, 14, 1.0f, 1.0f, 0.0f);  // 6 (14)
	draw_pikachu_line(96, 12, 1.0f, 1.0f, 0.0f);  // 7 (12)
	// 눈, 볼
	draw_pikachu_line(108, 6, 1.0f, 0.0f, 0.0f, GL_POLYGON);  // 8 (6)
	draw_pikachu_line(114, 9, 1.0f, 0.0f, 0.0f, GL_POLYGON);  // 9 (9)
	draw_pikachu_line(123, 9, 0.0f, 0.0f, 0.0f, GL_POLYGON);  // 10 (9)
	draw_pikachu_line(132, 9, 0.0f, 0.0f, 0.0f, GL_POLYGON);  // 11 (9)
	// 귀, 코
	draw_pikachu_line(142, 2, 0.0f, 0.0f, 0.0f);  // 13 (2)
	draw_pikachu_line(144, 3, 0.0f, 0.0f, 0.0f);  // 14 (3)
	draw_pikachu_line(141, 1, 0.0f, 0.0f, 0.0f);  // 12 (1)
}


void display(void) {
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	draw_axes();
	draw_line(px, py, qx, qy);
	draw_object();
	draw_pikachu();

	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':
		r = 1.0f; g = b = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 'g':
		g = 1.0f; r = b = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 'b':
		b = 1.0f; r = g = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 's':
		r = 250.0f / 255.0f, g = 128.0f / 255.0f, b = 114.0f / 255.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
}

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		r -= 0.1f;
		if (r < 0.0f) r = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		r += 0.1f;
		if (r > 1.0f) r = 1.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		g -= 0.1f;
		if (g < 0.0f) g = 0.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		g += 0.1f;
		if (g > 1.0f) g = 1.0f;
		fprintf(stdout, "$$$ The new window background color is (%5.3f, %5.3f, %5.3f).\n", r, g, b);
		glutPostRedisplay();
		break;
	}
}


float prev_mx = -1.0f;
float prev_my = -1.0f;

// int prevx, prevy;
void mousepress(int button, int state, int x, int y) {
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON
		&& xCoord(x) - 0.05 <= px && px <= xCoord(x) + 0.05
		&& yCoord(y) - 0.05 <= py && py <= yCoord(y) + 0.05) { // 왼쪽 버튼을 눌렀을 때

		leftbuttonpressed = 1;
	}
	else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		leftbuttonpressed = 0;
	}

	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) { // 오른쪽 버튼을 눌렀을 때
		rightbuttonpressed = 1;
	}
	else if (state == GLUT_UP && button == GLUT_RIGHT_BUTTON) {
		rightbuttonpressed = 0;
		prev_mx = prev_my = -1.0f;
		fprintf(stdout, "오른쪽 마우스 뗌!\n");
	}
}


void mousemove(int x, int y) {
	int key = glutGetModifiers();
	fprintf(stdout, "%d %d\n", x, y);
	// Shift + 왼쪽 버튼 클릭으로 파란색 점 이동 구현
	if (leftbuttonpressed) {
		if (key == GLUT_ACTIVE_SHIFT) {
			fprintf(stdout, "현재 점의 위치(변환): %lf %lf\n", qx, qy);
			fprintf(stdout, "클릭한 점 위치(int) : %d %d\n", x, y);
			fprintf(stdout, "=============================\n");
			px = xCoord(x);
			py = yCoord(y); 
		}
	}

	// Alt + 오른쪽 버튼 클릭으로 다각형 움직이기 구현
	if (rightbuttonpressed) {
		if (key == GLUT_ACTIVE_ALT) {
			float mx = xCoord(x), my = yCoord(y); // 마우스 위치
			float dx = mx - prev_mx;
			float dy = my - prev_my;

			// 처음 클릭한 경우, prev_mx, prev_my 초기화
			if (prev_mx == -1.0 && prev_my == -1.0) {
				prev_mx = mx;
				prev_my = my;
			}
			else {
				for (int i = 0; i < n_object_points; i++) {
					object[i][0] += dx; // 다각형의 각 점에 대해 dx, dy를 더해주면서 이동시킴
					object[i][1] += dy;
				}
				prev_mx = mx;
				prev_my = my;
			}
		}
		else if (key == GLUT_ACTIVE_SHIFT) {
			fprintf(stdout, "SHIFT+RIGHT Click!\n");
			pikachu_x = xCoord(x);
			pikachu_y = yCoord(y);
		}
	}

	// Ctrl + 마우스 오른쪽 버튼 클릭으로 다각형 크기 변경
	if (rightbuttonpressed) {
		if (key == GLUT_ACTIVE_CTRL) {
			float mx = xCoord(x), my = yCoord(y); // 마우스 위치
			float dx = mx - prev_mx;

			// 처음 클릭한 경우, prev_mx, prev_my 초기화
			if (prev_mx == -1.0 && prev_my == -1.0) {
				prev_mx = mx;
			}
			else {
				for (int i = 0; i < n_object_points; i++) {
					object[i][0] = (object[i][0] - object_center_x) * (1 + dx) + object_center_x; // 다각형의 각 점에 대해 dx, dy를 더해주면서 이동시킴
					object[i][1] = (object[i][1] - object_center_y) * (1 + dx) + object_center_y;
				}
				
				prev_mx = mx;
				prev_my = my;
			}
		}
	}
	glutPostRedisplay();
}

float prvx, prvy;

void mousewheel(int wheel, int direction, int x, int y) {
	int key = glutGetModifiers();
	if (key == GLUT_ACTIVE_ALT) {
		if(direction > 0) {
			fprintf(stdout, "ALT + Zoom in\n");
			pikachu_size += 0.01;
			pikachu_sh_x += 0.01;
		}
		else if (direction < 0) {
			fprintf(stdout, "ALT + Zoom out\n");
			pikachu_size -= 0.01;
			pikachu_sh_x -= 0.01;
		}
	}


	// 스크롤했을 때, 아핀 변환 구현. 
	if (direction > 0) {
		fprintf(stdout, "Zoom in\n");
		qx -= px; qy -= py; // 원점 이동

		prvx = qx; prvy = qy; // cos, sin 전에 백업
		qx = prvx * cos(0.01) - prvy * sin(0.01);
		qy = prvx * sin(0.01) + prvy * cos(0.01);

		qx += px; qy += py; // 다시 이동
	}
	else if (direction < 0) {
		fprintf(stdout, "Zoom out\n");
		qx -= px; qy -= py; // 원점 이동

		prvx = qx; prvy = qy; // cos, sin 전에 백업
		qx = prvx * cos(-0.01) - prvy * sin(-0.01);
		qy = prvx * sin(-0.01) + prvy * cos(-0.01);

		qx += px; qy += py; // 다시 이동
	}
	glutPostRedisplay();
}

void reshape(int width, int height) {
	// DO NOT MODIFY THIS FUNCTION!!!
	window_width = width, window_height = height;
	glViewport(0.0f, 0.0f, window_width, window_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-window_width / 500.0f, window_width / 500.0f, -window_height / 500.0f, window_height / 500.0f, -1.0f, 1.0f);

	glutPostRedisplay();
}


void close(void) {
	fprintf(stdout, "\n^^^ The control is at the close callback function now.\n\n");
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMouseWheelFunc(mousewheel);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutCloseFunc(close);
}

void initialize_renderer(void) {
	register_callbacks();
	r = 250.0f / 255.0f, g = 128.0f / 255.0f, b = 114.0f / 255.0f; // Background color = Salmon
	px = -0.5f, py = 0.40f, qx = -0.25f, qy = 0.30f;
	rotation_angle_in_degree = 1.0f; // 1 degree

	float sq_cx = 0.55f, sq_cy = -0.45f, sq_side = 0.25f; // 크기 변하는 사각형의 중심점 좌표, 크기. 
	object[0][0] = sq_cx + sq_side;
	object[0][1] = sq_cy + sq_side;
	object[1][0] = sq_cx + 0.3 * sq_side;
	object[1][1] = sq_cy + 1.2 * sq_side;
	object[2][0] = sq_cx - sq_side;
	object[2][1] = sq_cy + sq_side;
	object[3][0] = sq_cx - sq_side;
	object[3][1] = sq_cy - sq_side;
	object[4][0] = sq_cx - 0.2 * sq_side;
	object[4][1] = sq_cy - 1.9 * sq_side;
	object[5][0] = sq_cx + sq_side;
	object[5][1] = sq_cy - sq_side;
	object_center_x = object_center_y = 0.0f;	// 사각형의 무게 중심
	for (int i = 0; i < n_object_points; i++) {
		object_center_x += object[i][0];
		object_center_y += object[i][1];
	}
	object_center_x /= n_object_points;
	object_center_y /= n_object_points;	// 무게 중심점 구하는 로직.. 계산 => 도형 무궁무진하게 만들 수 있다!
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = TRUE;
	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s \n\t\tby 20191559 SangWon Kang\n\n", program_name);

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 4
void main(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Simple 2D Transformations";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'r', 'g', 'b', 's', 'q'",
		"    - Special keys used: LEFT, RIGHT, UP, DOWN",
		"    - Mouse used: SHIFT/L-click and move, ALT/R-click and move,\n\t\t  CTRL/R-click and move, SHIFT/R-click and move",
		"    - Wheel used: up and down scroll, ALT/up and down scroll\n"
		"    - Other operations: window size change"
	};

	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // <-- Be sure to use this profile for this example code!
	//	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_RGBA);

	glutInitWindowSize(750, 750);
	glutInitWindowPosition(500, 200);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	// glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_EXIT); // default
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutMainLoop();
	fprintf(stdout, "^^^ The control is at the end of main function now.\n\n");
}