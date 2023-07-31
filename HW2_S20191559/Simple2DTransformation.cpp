#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0


const float PI = 3.14159265358979323846;
unsigned int timestamp = 0;

std::vector<std::pair<unsigned int, unsigned int>> time_table(0);


void draw_circle(float x, float y, float radius, int num_segments, bool filled = true) {
	if (filled) {
		glBegin(GL_TRIANGLE_FAN);
	}
	else {
		glBegin(GL_LINE_LOOP);
	}
	for (int i = 0; i <= num_segments; i++) {
		float angle = i * 2.0f * PI / num_segments;
		float dx = radius * cosf(angle);
		float dy = radius * sinf(angle);
		glVertex2f(x + dx, y + dy);
	}
	glEnd();
}


int win_width = 0, win_height = 0; 
float centerx = 0.0f, centery = 0.0f, rotate_angle = 0.0f;


GLfloat line[2][2];
GLfloat line_color[3] = { 0.76f, 0.60f, 0.42f };
GLuint VBO_line, VAO_line;


void prepare_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height; 
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f; 
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_line);
	glBindVertexArray(VAO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height; 
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f; 
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_line(void) { // Draw line in its MC.
	// y = x - win_height/4
	glUniform3fv(loc_primitive_color, 1, line_color);
	glBindVertexArray(VAO_line);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

// sword
#define SWORD_BODY 0
#define SWORD_BODY2 1
#define SWORD_HEAD 2
#define SWORD_HEAD2 3
#define SWORD_IN 4
#define SWORD_DOWN 5
#define SWORD_BODY_IN 6

GLfloat sword_body[4][2] = { { -6.0, 0.0 },{ -6.0, -4.0 },{ 6.0, -4.0 },{ 6.0, 0.0 } };
GLfloat sword_body2[4][2] = { { -2.0, -4.0 },{ -2.0, -6.0 } ,{ 2.0, -6.0 },{ 2.0, -4.0 } };
GLfloat sword_head[4][2] = { { -2.0, 0.0 },{ -2.0, 16.0 } ,{ 2.0, 16.0 },{ 2.0, 0.0 } };
GLfloat sword_head2[3][2] = { { -2.0, 16.0 },{ 0.0, 19.46 } ,{ 2.0, 16.0 } };
GLfloat sword_in[4][2] = { { -0.3, 0.7 },{ -0.3, 15.3 } ,{ 0.3, 15.3 },{ 0.3, 0.7 } };
GLfloat sword_down[4][2] = { { -2.0, -6.0 } ,{ 2.0, -6.0 },{ 4.0, -8.0 },{ -4.0, -8.0 } };
GLfloat sword_body_in[4][2] = { { 0.0, -1.0 } ,{ 1.0, -2.732 },{ 0.0, -4.464 },{ -1.0, -2.732 } };

GLfloat sword_color[7][3] = {
	{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_sword, VAO_sword;
void prepare_sword() {
	GLsizeiptr buffer_size = sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down) + sizeof(sword_body_in);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory


	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sword_body), sword_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body), sizeof(sword_body2), sword_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2), sizeof(sword_head), sword_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head), sizeof(sword_head2), sword_head2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2), sizeof(sword_in), sword_in);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in), sizeof(sword_down), sword_down);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down), sizeof(sword_body_in), sword_body_in);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_sword);
	glBindVertexArray(VAO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_sword() {
	glBindVertexArray(VAO_sword);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD2]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 3);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_DOWN]);
	glDrawArrays(GL_TRIANGLE_FAN, 19, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 23, 4);

	glBindVertexArray(0);
}

//shirt
#define SHIRT_LEFT_BODY 0
#define SHIRT_RIGHT_BODY 1
#define SHIRT_LEFT_COLLAR 2
#define SHIRT_RIGHT_COLLAR 3
#define SHIRT_FRONT_POCKET 4
#define SHIRT_BUTTON1 5
#define SHIRT_BUTTON2 6
#define SHIRT_BUTTON3 7
#define SHIRT_BUTTON4 8
GLfloat left_body[6][2] = { { 0.0, -9.0 },{ -8.0, -9.0 },{ -11.0, 8.0 },{ -6.0, 10.0 },{ -3.0, 7.0 },{ 0.0, 9.0 } };
GLfloat right_body[6][2] = { { 0.0, -9.0 },{ 0.0, 9.0 },{ 3.0, 7.0 },{ 6.0, 10.0 },{ 11.0, 8.0 },{ 8.0, -9.0 } };
GLfloat left_collar[4][2] = { { 0.0, 9.0 },{ -3.0, 7.0 },{ -6.0, 10.0 },{ -4.0, 11.0 } };
GLfloat right_collar[4][2] = { { 0.0, 9.0 },{ 4.0, 11.0 },{ 6.0, 10.0 },{ 3.0, 7.0 } };
GLfloat front_pocket[6][2] = { { 5.0, 0.0 },{ 4.0, 1.0 },{ 4.0, 3.0 },{ 7.0, 3.0 },{ 7.0, 1.0 },{ 6.0, 0.0 } };
GLfloat button1[3][2] = { { -1.0, 6.0 },{ 1.0, 6.0 },{ 0.0, 5.0 } };
GLfloat button2[3][2] = { { -1.0, 3.0 },{ 1.0, 3.0 },{ 0.0, 2.0 } };
GLfloat button3[3][2] = { { -1.0, 0.0 },{ 1.0, 0.0 },{ 0.0, -1.0 } };
GLfloat button4[3][2] = { { -1.0, -3.0 },{ 1.0, -3.0 },{ 0.0, -4.0 } };

GLfloat shirt_color[9][3] = {
	{ 200 / 255.0f, 200 / 255.0f, 200 / 255.0f },
	{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f }
};

GLfloat shirt_color_2[9][3] = {
	{ 255 / 255.0f, 255 / 255.0f, 0 / 255.0f },
	{ 255 / 255.0f, 255 / 255.0f, 0 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f }
};


GLuint VBO_shirt, VAO_shirt;
void prepare_shirt() {
	GLsizeiptr buffer_size = sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1) + sizeof(button2) + sizeof(button3) + sizeof(button4);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_shirt);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_shirt);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(left_body), left_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body), sizeof(right_body), right_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body), sizeof(left_collar), left_collar);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar), sizeof(right_collar), right_collar);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar),
		sizeof(front_pocket), front_pocket);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket), sizeof(button1), button1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1), sizeof(button2), button2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1) + sizeof(button2), sizeof(button3), button3);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1) + sizeof(button2) + sizeof(button3), sizeof(button4), button4);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_shirt);
	glBindVertexArray(VAO_shirt);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_shirt);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_shirt() {
	glBindVertexArray(VAO_shirt);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_LEFT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_RIGHT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_LEFT_COLLAR]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_RIGHT_COLLAR]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_FRONT_POCKET]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON1]);
	glDrawArrays(GL_TRIANGLE_FAN, 26, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON2]);
	glDrawArrays(GL_TRIANGLE_FAN, 29, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON3]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON4]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 3);
	glBindVertexArray(0);
}

void draw_shirt_2() {
	glBindVertexArray(VAO_shirt);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_LEFT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_RIGHT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_LEFT_COLLAR]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_RIGHT_COLLAR]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_FRONT_POCKET]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_BUTTON1]);
	glDrawArrays(GL_TRIANGLE_FAN, 26, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_BUTTON2]);
	glDrawArrays(GL_TRIANGLE_FAN, 29, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_BUTTON3]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color_2[SHIRT_BUTTON4]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 3);
	glBindVertexArray(0);
}

// cake
#define CAKE_FIRE 0
#define CAKE_CANDLE 1
#define CAKE_BODY 2
#define CAKE_BOTTOM 3
#define CAKE_DECORATE 4

GLfloat cake_fire[4][2] = { { -0.5, 14.0 },{ -0.5, 13.0 },{ 0.5, 13.0 },{ 0.5, 14.0 } };
GLfloat cake_candle[4][2] = { { -1.0, 8.0 } ,{ -1.0, 13.0 },{ 1.0, 13.0 },{ 1.0, 8.0 } };
GLfloat cake_body[4][2] = { { 8.0, 5.0 },{ -8.0, 5.0 } ,{ -8.0, 8.0 },{ 8.0, 8.0 } };
GLfloat cake_bottom[4][2] = { { -10.0, 1.0 },{ -10.0, 5.0 },{ 10.0, 5.0 },{ 10.0, 1.0 } };
GLfloat cake_decorate[4][2] = { { -10.0, 0.0 },{ -10.0, 1.0 },{ 10.0, 1.0 },{ 10.0, 0.0 } };

GLfloat cake_color[5][3] = {
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 204 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
{ 102 / 255.0f, 51 / 255.0f, 0 / 255.0f }
};

GLuint VBO_cake, VAO_cake;

void prepare_cake() {
	int size = sizeof(cake_fire);
	GLsizeiptr buffer_size = sizeof(cake_fire) * 5;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, size, cake_fire);
	glBufferSubData(GL_ARRAY_BUFFER, size, size, cake_candle);
	glBufferSubData(GL_ARRAY_BUFFER, size * 2, size, cake_body);
	glBufferSubData(GL_ARRAY_BUFFER, size * 3, size, cake_bottom);
	glBufferSubData(GL_ARRAY_BUFFER, size * 4, size, cake_decorate);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cake);
	glBindVertexArray(VAO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_cake() {
	glBindVertexArray(VAO_cake);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_FIRE]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_CANDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_DECORATE]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glBindVertexArray(0);
}

//draw cocktail
#define COCKTAIL_NECK 0
#define COCKTAIL_LIQUID 1
#define COCKTAIL_REMAIN 2
#define COCKTAIL_STRAW 3
#define COCKTAIL_DECO 4

GLfloat neck[6][2] = { { -6.0, -12.0 },{ -6.0, -11.0 },{ -1.0, 0.0 },{ 1.0, 0.0 },{ 6.0, -11.0 },{ 6.0, -12.0 } };
GLfloat liquid[6][2] = { { -1.0, 0.0 },{ -9.0, 4.0 },{ -12.0, 7.0 },{ 12.0, 7.0 },{ 9.0, 4.0 },{ 1.0, 0.0 } };
GLfloat remain[4][2] = { { -12.0, 7.0 },{ -12.0, 10.0 },{ 12.0, 10.0 },{ 12.0, 7.0 } };
GLfloat straw[4][2] = { { 7.0, 7.0 },{ 12.0, 12.0 },{ 14.0, 12.0 },{ 9.0, 7.0 } };
GLfloat deco[8][2] = { { 12.0, 12.0 },{ 10.0, 14.0 },{ 10.0, 16.0 },{ 12.0, 18.0 },{ 14.0, 18.0 },{ 16.0, 16.0 },{ 16.0, 14.0 },{ 14.0, 12.0 } };

GLfloat cocktail_color[5][3] = {
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 0 / 255.0f, 63 / 255.0f, 122 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 191 / 255.0f, 255 / 255.0f, 0 / 255.0f },
	{ 218 / 255.0f, 165 / 255.0f, 32 / 255.0f }
};

GLuint VBO_cocktail, VAO_cocktail;
void prepare_cocktail() {
	GLsizeiptr buffer_size = sizeof(neck) + sizeof(liquid) + sizeof(remain) + sizeof(straw)
		+ sizeof(deco);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cocktail);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cocktail);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(neck), neck);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck), sizeof(liquid), liquid);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid), sizeof(remain), remain);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid) + sizeof(remain), sizeof(straw), straw);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid) + sizeof(remain) + sizeof(straw),
		sizeof(deco), deco);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cocktail);
	glBindVertexArray(VAO_cocktail);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cocktail);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_cocktail() {
	glBindVertexArray(VAO_cocktail);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_NECK]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_LIQUID]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_REMAIN]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_STRAW]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_DECO]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 8);

	glBindVertexArray(0);
}


//house
#define HOUSE_ROOF 0
#define HOUSE_BODY 1
#define HOUSE_CHIMNEY 2
#define HOUSE_DOOR 3
#define HOUSE_WINDOW 4

GLfloat roof[3][2] = { { -12.0, 0.0 },{ 0.0, 12.0 },{ 12.0, 0.0 } };
GLfloat house_body[4][2] = { { -12.0, -14.0 },{ -12.0, 0.0 },{ 12.0, 0.0 },{ 12.0, -14.0 } };
GLfloat chimney[4][2] = { { 6.0, 6.0 },{ 6.0, 14.0 },{ 10.0, 14.0 },{ 10.0, 2.0 } };
GLfloat door[4][2] = { { -8.0, -14.0 },{ -8.0, -8.0 },{ -4.0, -8.0 },{ -4.0, -14.0 } };
GLfloat window[4][2] = { { 4.0, -6.0 },{ 4.0, -2.0 },{ 8.0, -2.0 },{ 8.0, -6.0 } };

GLfloat house_color[5][3] = {
	{ 39 / 255.0f, 200 / 255.0f, 42 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 233 / 255.0f, 113 / 255.0f, 23 / 255.0f },
	{ 44 / 255.0f, 180 / 255.0f, 49 / 255.0f }
};

GLfloat house_color_2[5][3] = {
	{ 39 / 255.0f, 200 / 255.0f, 42 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 233 / 255.0f, 113 / 255.0f, 23 / 255.0f },
	{ 44 / 255.0f, 180 / 255.0f, 49 / 255.0f }
};

GLuint VBO_house, VAO_house;
void prepare_house() {
	GLsizeiptr buffer_size = sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door)
		+ sizeof(window);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(roof), roof);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof), sizeof(house_body), house_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body), sizeof(chimney), chimney);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney), sizeof(door), door);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door),
		sizeof(window), window);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_house);
	glBindVertexArray(VAO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


float loading_percentage = 30.0f;
float loading_percentage_2 = 30.0f;

void draw_house() {
	glBindVertexArray(VAO_house);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_ROOF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_CHIMNEY]);
	glDrawArrays(GL_TRIANGLE_FAN, 7, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_DOOR]);
	glDrawArrays(GL_TRIANGLE_FAN, 11, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glBindVertexArray(0);
}

void draw_house_2() {
	glBindVertexArray(VAO_house);

	glUniform3fv(loc_primitive_color, 1, house_color_2[HOUSE_ROOF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	glUniform3fv(loc_primitive_color, 1, house_color_2[HOUSE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

	glUniform3fv(loc_primitive_color, 1, house_color_2[HOUSE_CHIMNEY]);
	glDrawArrays(GL_TRIANGLE_FAN, 7, 4);

	glUniform3fv(loc_primitive_color, 1, house_color_2[HOUSE_DOOR]);
	glDrawArrays(GL_TRIANGLE_FAN, 11, 4);

	glUniform3fv(loc_primitive_color, 1, house_color_2[HOUSE_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glBindVertexArray(0);
}



// PIKACHU
GLfloat pikachu_body_1[32][2] = { {-9, -8}, {-14, -5}, {-15, -3}, {-15, -2}, {-15, -1}, {-15, 1}, {-14, 3},
{-13, 6}, {-13, 10}, {-12, 13}, {-11, 16}, {-8, 20}, {-7, 19}, {-7, 12},
{-8, 9}, {-9, 8}, {-8, 9}, {3, 8}, {1, 7}, {3, 8}, {13, 11}, {18, 11}, {17, 10},
{12, 7}, {6, 5}, {4, 5}, {5, 5}, {5, -4}, {4, -5}, {3, -6}, {2, -7}, {0, -7} };
GLfloat pikachu_body_2[19][2] = { {4, -5}, {4, -7}, {5, -9}, {6, -11}, {7, -11}, {8, -12}, {7, -13}, {9, -11},
{10, -11}, {10, -14}, {8, -18}, {7, -19}, {6, -19}, {6, -17}, {6, -19}, {-1, -19},
{2, -15}, {3, -12}, {3, -11} };
GLfloat pikachu_body_3[6][2] = { {-1, -19}, {0, -20}, {-6, -20}, {-5, -19},
{-5, -14}, {-4, -10} };
GLfloat pikachu_body_4[10][2] = { {-6, -11}, {-5, -14}, {-5, -19}, {-7, -20}, {-10, -20},
{-10, -19}, {-9, -18}, {-12, -14}, {-13, -11}, {-13, -9} };
GLfloat pikachu_body_5[15][2] = { {-10, -19}, {-17, -19},
{-19, -16}, {-20, -13}, {-20, -12}, {-19, -11}, {-18, -11}, {-17, -13},
{-16, -16}, {-17, -13}, {-17, -12}, {-15, -11}, {-14, -9}, {-13, -7}, {-13, -6} };
GLfloat pikachu_body_6[14][2] = { {-11, -1}, {-10, -1}, {-9, -4}, {-7, -6}, {-6, -6}, {-4, -4}, {-3, -2}, {-3,-1},
{-2, -1}, {-4, -1}, {-6, 0}, {-8, 0}, {-9, -1}, {-11, -1} };
GLfloat pikachu_body_7[12][2] = { {5, 1}, {10, 3},
{15, 4}, {20, 4}, {15, -4}, {8, -4}, {10, -9}, {6, -10}, {6, -11}, {5, -9},
{7, -8}, {5, -4} };
GLfloat pikachu_cheek_1[6][2] = { {-15, 0}, {-14, 0}, {-13, -1}, {-13, -2}, {-14, -3}, {-15,-3} };
GLfloat pikachu_cheek_2[9][2] = { {0, -2}, {1, -1} , {3, -1}, {4, -2}, {4, -3}, {3, -4}, {1, -4}, {0, -3}, {0,-2} };
GLfloat pikachu_eye_1[9][2] = { {-11, 1}, {-12, 2} , {-12, 3}, {-11, 4}, {-10, 4}, {-9, 3}, {-9, 2}, {-10, 1},
{-11, 1} };
GLfloat pikachu_eye_2[9][2] = { {-2, 2}, {-2, 3}, {-1, 4}, {0, 4}, {1, 3}, {1, 2}, {0, 1}, {-1, 1},
{-2, 2} };
GLfloat pikachu_ear_1[1][2] = { {-7, 1} };
GLfloat pikachu_ear_2[2][2] = { {12, 7}, {13, 11} };
GLfloat pikachu_nose[3][2] = { {-12, 13}, {-9, 15}, {-7, 15} };

GLfloat pikachu_fill[30][2] = {
	{-13, -6}, {-13, -7}, {-14,-9}, {-15, -11}, {-17, -12},
	{-17, -13},{-18,-11}, {-19, -11},{-20, -12}, {-20, -13},
	{-19, -16}, {-17, -19}, {-10, -19}, {-10, -20}, {-7, -20},
	{-5, -19}, {-6, -20}, {0, -20}, {-1, -19}, {7, -19},
	{8, -18}, {10, -14}, {10, -11}, {9, -11}, {8, -12},
	{7, -11}, {6, -11}, {4, -7}, {4, -5}, {-13, -6}
};

GLfloat pikachu_color[14][3] = {
	{1.0f, 217 / 255.0f, 36 / 255.0f},
	{0.8f, 0.8f, 0.0f},
	{1.0f, 217 / 255.0f, 36 / 255.0f},
	{1.0f, 217 / 255.0f, 36 / 255.0f},
	{0.8f, 0.8f, 0.0f},
	{181 / 255.0f, 41 / 255.0f, 10 / 255.0f}, // mouth
	{0.8f, 0.8f, 0.0f},

	{254 / 255.0f, 70 / 255.0f, 46 / 255.0f},
	{254 / 255.0f, 70 / 255.0f, 46 / 255.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},

	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f}
};
GLuint VBO_pikachu, VAO_pikachu;

void prepare_pikachu() {
	GLsizeiptr buffer_size = sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1) + sizeof(pikachu_cheek_2) + sizeof(pikachu_eye_1) + sizeof(pikachu_eye_2) + sizeof(pikachu_ear_1) + sizeof(pikachu_ear_2) + sizeof(pikachu_nose) + sizeof(pikachu_fill);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_pikachu);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_pikachu);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory


	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pikachu_body_1), pikachu_body_1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1), sizeof(pikachu_body_2), pikachu_body_2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2), sizeof(pikachu_body_3), pikachu_body_3);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3), sizeof(pikachu_body_4), pikachu_body_4);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4), sizeof(pikachu_body_5), pikachu_body_5);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5), sizeof(pikachu_body_6), pikachu_body_6);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6), sizeof(pikachu_body_7),
		pikachu_body_7);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7),
		sizeof(pikachu_cheek_1), pikachu_cheek_1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1), sizeof(pikachu_cheek_2), pikachu_cheek_2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1) + sizeof(pikachu_cheek_2), sizeof(pikachu_eye_1), pikachu_eye_1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1) + sizeof(pikachu_cheek_2) + sizeof(pikachu_eye_1), sizeof(pikachu_eye_2), pikachu_eye_2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1) + sizeof(pikachu_cheek_2) + sizeof(pikachu_eye_1) + sizeof(pikachu_eye_2), sizeof(pikachu_ear_1), pikachu_ear_1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1) + sizeof(pikachu_cheek_2) + sizeof(pikachu_eye_1) + sizeof(pikachu_eye_2) + sizeof(pikachu_ear_1), sizeof(pikachu_ear_2), pikachu_ear_2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1) + sizeof(pikachu_cheek_2) + sizeof(pikachu_eye_1) + sizeof(pikachu_eye_2) + sizeof(pikachu_ear_1) + sizeof(pikachu_ear_2), sizeof(pikachu_nose), pikachu_nose);

	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pikachu_body_1) + sizeof(pikachu_body_2) + sizeof(pikachu_body_3) + sizeof(pikachu_body_4) + sizeof(pikachu_body_5) + sizeof(pikachu_body_6) + sizeof(pikachu_body_7)
		+ sizeof(pikachu_cheek_1) + sizeof(pikachu_cheek_2) + sizeof(pikachu_eye_1) + sizeof(pikachu_eye_2) + sizeof(pikachu_ear_1) + sizeof(pikachu_ear_2) + sizeof(pikachu_nose), sizeof(pikachu_fill), pikachu_fill);


	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_pikachu);
	glBindVertexArray(VAO_pikachu);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_pikachu);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_pikachu() {
	glBindVertexArray(VAO_pikachu);

	glLineWidth(5.0f);
	////
	glUniform3fv(loc_primitive_color, 1, pikachu_color[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 32);


	glUniform3fv(loc_primitive_color, 1, pikachu_color[5]);
	glDrawArrays(GL_LINE_STRIP, 82, 14); // MOUTH

	glUniform3fv(loc_primitive_color, 1, pikachu_color[6]);
	glDrawArrays(GL_TRIANGLE_FAN, 96, 12); // TAIL

	//
	glUniform3fv(loc_primitive_color, 1, pikachu_color[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 147, 177); // BODY
	//
	////

	glLineWidth(5.0f);
	glUniform3fv(loc_primitive_color, 1, pikachu_color[13]);
	glDrawArrays(GL_LINE_STRIP, 0, 32);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[13]);
	glDrawArrays(GL_LINE_STRIP, 32, 19);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[13]);
	glDrawArrays(GL_LINE_STRIP, 51, 6);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[13]);
	glDrawArrays(GL_LINE_STRIP, 57, 10);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[13]);
	glDrawArrays(GL_LINE_STRIP, 67, 15);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[5]);
	glDrawArrays(GL_LINE_STRIP, 82, 14);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[13]);
	glDrawArrays(GL_LINE_STRIP, 96, 12);
	//
	glUniform3fv(loc_primitive_color, 1, pikachu_color[7]);
	glDrawArrays(GL_TRIANGLE_FAN, 108, 6);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[8]);
	glDrawArrays(GL_TRIANGLE_FAN, 114, 9);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[9]);
	glDrawArrays(GL_TRIANGLE_FAN, 123, 9);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[10]);
	glDrawArrays(GL_TRIANGLE_FAN, 132, 9);
	//
	glUniform3fv(loc_primitive_color, 1, pikachu_color[11]);
	glDrawArrays(GL_LINE_STRIP, 141, 1);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[12]);
	glDrawArrays(GL_LINE_STRIP, 142, 2);

	glUniform3fv(loc_primitive_color, 1, pikachu_color[13]);
	glDrawArrays(GL_LINE_STRIP, 144, 3);

	glBindVertexArray(0);
	glLineWidth(1.0f);
}

float pikachu_scale = 0.0f;
int jump_count = 0;
float jump_height = 0.0f;

float pokeball_scale = 0.0f;
GLfloat pokeball_top[4][2] = { {-1.0, 0.0}, {1.0, 0.0}, {1.0, 0.5}, {-1.0, 0.5} };
GLfloat pokeball_bottom[4][2] = { {-1.0, -0.5}, {1.0, -0.5}, {1.0, 0.0}, {-1.0, 0.0} };
GLfloat pokeball_center[4][2] = { {-0.2, -0.2}, {0.2, -0.2}, {0.2, 0.2}, {-0.2, 0.2} };

GLfloat pokeball_colors[5][3] = {
	{1.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{1.0f, 1.0f, 1.0f},
};

GLfloat pokeball_colors_2[5][3] = {
	{1.0f, 1.0f, 1.0f},
	{228.0f/255.0f, 135.0f/255.0f, 131.0f/255.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{1.0f, 1.0f, 1.0f},
};

enum {
	POKEBALL_TOP = 0,
	POKEBALL_BOTTOM,
	POKEBALL_CENTER,
	POKEBALL_OUTER_CENTER,
	POKEBALL_INNER_CENTER
};

const int num_segments = 50;
const float radius = 1.0f;
const float outer_circle_radius = 0.25f;
const float inner_circle_radius = 0.2f;

std::vector<GLfloat> pokeball_top_vertices;
std::vector<GLfloat> pokeball_bottom_vertices;
std::vector<GLfloat> pokeball_outer_center_vertices;
std::vector<GLfloat> pokeball_inner_center_vertices;

void generate_circle_vertices(std::vector<GLfloat>& vertices, int segments, float r, float start_angle, float end_angle) {
	for (int i = 0; i <= segments; ++i) {
		float theta = start_angle + (end_angle - start_angle) * (float)i / (float)segments;
		float x = r * cosf(theta);
		float y = r * sinf(theta);
		vertices.push_back(x);
		vertices.push_back(y);
	}
}

GLuint VBO_pokeball, VAO_pokeball;

void prepare_pokeball() {
	generate_circle_vertices(pokeball_top_vertices, num_segments / 2, radius, PI, 2 * PI);
	generate_circle_vertices(pokeball_bottom_vertices, num_segments / 2, radius, 0, PI);
	generate_circle_vertices(pokeball_outer_center_vertices, num_segments, outer_circle_radius, 0, 2 * PI);
	generate_circle_vertices(pokeball_inner_center_vertices, num_segments, inner_circle_radius, 0, 2 * PI);

	GLsizeiptr buffer_size = pokeball_top_vertices.size() * sizeof(GLfloat) + pokeball_bottom_vertices.size() * sizeof(GLfloat) + pokeball_outer_center_vertices.size() * sizeof(GLfloat) + pokeball_inner_center_vertices.size() * sizeof(GLfloat);

	glGenBuffers(1, &VBO_pokeball);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_pokeball);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, pokeball_top_vertices.size() * sizeof(GLfloat), pokeball_top_vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, pokeball_top_vertices.size() * sizeof(GLfloat), pokeball_bottom_vertices.size() * sizeof(GLfloat), pokeball_bottom_vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, pokeball_top_vertices.size() * sizeof(GLfloat) + pokeball_bottom_vertices.size() * sizeof(GLfloat), pokeball_outer_center_vertices.size() * sizeof(GLfloat), pokeball_outer_center_vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, pokeball_top_vertices.size() * sizeof(GLfloat) + pokeball_bottom_vertices.size() * sizeof(GLfloat) + pokeball_outer_center_vertices.size() * sizeof(GLfloat), pokeball_inner_center_vertices.size() * sizeof(GLfloat), pokeball_inner_center_vertices.data());
	
	glGenVertexArrays(1, &VAO_pokeball);
	glBindVertexArray(VAO_pokeball);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_pokeball);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_pokeball() {
	glBindVertexArray(VAO_pokeball);

	glUniform3fv(loc_primitive_color, 1, pokeball_colors[POKEBALL_TOP]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, num_segments / 2 + 1);

	glUniform3fv(loc_primitive_color, 1, pokeball_colors[POKEBALL_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, num_segments / 2 + 1, num_segments / 2 + 1);

	glUniform3fv(loc_primitive_color, 1, pokeball_colors[POKEBALL_OUTER_CENTER]);
	glDrawArrays(GL_TRIANGLE_FAN, num_segments + 2, num_segments + 1);

	glUniform3fv(loc_primitive_color, 1, pokeball_colors[POKEBALL_INNER_CENTER]);
	glDrawArrays(GL_TRIANGLE_FAN, 2 * num_segments + 3, num_segments + 1);

	glBindVertexArray(0);
}


void draw_voltorb() {
	glBindVertexArray(VAO_pokeball);

	glUniform3fv(loc_primitive_color, 1, pokeball_colors_2[POKEBALL_TOP]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, num_segments / 2 + 1);

	glUniform3fv(loc_primitive_color, 1, pokeball_colors_2[POKEBALL_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, num_segments / 2 + 1, num_segments / 2 + 1);

	glBindVertexArray(0);
}

float voltorb_scale = 0.0f;


void draw_sword_with_transforms(float x, float y, float scale, float sx, float sy) {
	float dx =  sx-x;
	float dy =  sy-y;
	float rotation = atan2(dy, dx) * TO_DEGREE - 90.0f;  // Subtract 90 degrees to align the sword's tip with the destination

	glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rotation * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scale, scale, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();
}

// Define constants for the number of cocktails and their falling speed
const int NUM_COCKTAILS = 100;
const float COCKTAIL_FALL_SPEED = 5.0f;
glm::vec3 cocktail_positions[NUM_COCKTAILS];

// Initialize the cocktail positions with random x-coordinates and y-coordinates above the top of the screen
void init_cocktail_positions() {
	srand(time(0)); // Seed the random number generator
	for (int i = 0; i < NUM_COCKTAILS; i++) {
		float x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / win_width));
		float y = win_height + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 100.0f));
		cocktail_positions[i] = glm::vec3(x, y, 0.0f);
	}
}

typedef enum {
	APPEAR = 0,
	FIRST_ATK,
	FIRST_RESP,
	SECOND_ATK,
	SECOND_RESP,
	THIRD_ATK, 
	THIRD_RESP,
	FOURTH_ATK,
	FOURTH_RESP,
	FIFTH_ATK,
	FIFTH_RESP,
	GAME_OVER
};

void prepare_time_table() {
	time_table.push_back(std::make_pair(0, 300));	// APPEAR
	time_table.push_back(std::make_pair(500, 800));	// FIRST_ATK
	time_table.push_back(std::make_pair(850, 950)); // FIRST_RESP

	time_table.push_back(std::make_pair(1100, 1400)); // SECOND_ATK
	time_table.push_back(std::make_pair(1450, 1550)); // SEOCND_RESP

	time_table.push_back(std::make_pair(1700, 2000)); // THIRD_ATK
	time_table.push_back(std::make_pair(2050, 2150)); // THIRD_RESP

	time_table.push_back(std::make_pair(2300, 2600)); // FOURTH_ATK
	time_table.push_back(std::make_pair(2650, 2750)); // FOURTH_RESP

	time_table.push_back(std::make_pair(2900, 3200)); // FIFTH_ATK
	time_table.push_back(std::make_pair(3250, 3350)); // FIFTH_RESP

	time_table.push_back(std::make_pair(3450, UINT_MAX)); // GAME_OVER
}


bool in_time(int idx) {
	if (time_table[idx].first < timestamp && timestamp < time_table[idx].second)
		return true;
	else return false;
}

void display(void) {
	int i;
	float x, r, s, delx, delr, dels;
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);

	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	for (int i = 0; i < 100; i++) {

		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1000, 300 - 10 * i * i * 0.01, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 2.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, -30 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_line();
	}
	// draw_airplane();

	// Draw 30 swords with the desired animation
	if (in_time(FIRST_ATK)) {
		float angle, distance, distance2;
		for (int i = 0; i < 30; ++i) {
			angle = (360.0f / 30.0f) * i;
			distance = 300.0f - (8 * timestamp % 300);
			distance2 = 500.0f - (8 * timestamp % 500);

			float x = cos(angle * TO_RADIAN) * distance;
			float x2 = cos(angle * TO_RADIAN) * distance2;
			float y = sin(angle * TO_RADIAN) * distance;
			float y2 = sin(angle * TO_RADIAN) * distance2;


			draw_sword_with_transforms(x + 250.0f, y + 100.0f, 1.0f, 250, 100);
			draw_sword_with_transforms(x2 + 250.0f, y2 + 100.0f, 2.0f, 250, 100);
		}
	}

	if (in_time(FOURTH_ATK)) {
		float angle, distance, distance2;
		for (int i = 0; i < 30; ++i) {
			angle = (360.0f / 30.0f) * i;
			distance = 300.0f - (8 * timestamp % 300);
			distance2 = 500.0f - (8 * timestamp % 500);

			float x = cos(angle * TO_RADIAN) * distance;
			float x2 = cos(angle * TO_RADIAN) * distance2;
			float y = sin(angle * TO_RADIAN) * distance;
			float y2 = sin(angle * TO_RADIAN) * distance2;


			draw_sword_with_transforms(x - 250.0f, y - 100.0f, 1.0f, -250, -100);
			draw_sword_with_transforms(x2 - 250.0f, y2 - 100.0f, 2.0f, -250, -100);
		}
	}

	int shirt_clock = timestamp % 360;
	delr = r = 5.0f;
	float a = 250.0f; // Radius along the X-axis
	float b = 40.0f; // Radius along the Y-axis

	float center_x = 250.0f; // Center X-coordinate
	float center_y = -30.0f; // Center Y-coordinate

	// Shirt
	for (i = 0; i < 72; i++, r += delr) {
		float angle = shirt_clock * TO_RADIAN + r * TO_RADIAN;

		// Calculate the position on the ellipse
		float ellipse_x = center_x + a * cos(angle);
		float ellipse_y = center_y + b * sin(angle);

		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(ellipse_x, ellipse_y, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 0.0f));

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_shirt();
	}
	// Shirt 2
	a = 375.0f; // Radius along the X-axis
	b = 60.0f; // Radius along the Y-axis

	center_x = -350.0f; // Center X-coordinate
	center_y = -300.0f; // Center Y-coordinate
	for (i = 0; i < 72; i++, r += delr) {
		float angle = shirt_clock * TO_RADIAN + r * TO_RADIAN;

		// Calculate the position on the ellipse
		float ellipse_x = center_x + a * cos(angle);
		float ellipse_y = center_y + b * sin(angle);

		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(ellipse_x, ellipse_y, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 0.0f));

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_shirt();
	}


	if (in_time(FIRST_RESP) || in_time(THIRD_RESP) || in_time(FIFTH_RESP)) {
		float blink_factor = sin(timestamp * 0.2f) * 0.5f + 0.5f;
		if (blink_factor > 0.5f) {
			pikachu_color[0][1] = 0;
		}
		else {
			pikachu_color[0][1] = 217 / 255.0f;
		}
	}
	else pikachu_color[0][1] = 217 / 255.0f;


	if (in_time(SECOND_RESP) || in_time(FOURTH_RESP)) {
		float blink_factor = sin(timestamp * 0.2f) * 0.5f + 0.5f;
		if (blink_factor > 0.5f) {
			pokeball_colors_2[1][0] = 1;
			pokeball_colors_2[1][1] = 0;
			pokeball_colors_2[1][2] = 0;

			pokeball_colors_2[0][1] = 0;
			pokeball_colors_2[0][2] = 0;
		}
		else {
			pokeball_colors_2[1][0] = 228.0f / 255.0f;
			pokeball_colors_2[1][1] = 135.0f / 255.0f;
			pokeball_colors_2[1][2] = 131.0f / 255.0f;

			pokeball_colors_2[0][1] = 1;
			pokeball_colors_2[0][2] = 1;
		}
	}
	else {
		pokeball_colors_2[1][0] = 228.0f / 255.0f;
		pokeball_colors_2[1][1] = 135.0f / 255.0f;
		pokeball_colors_2[1][2] = 131.0f / 255.0f;

		pokeball_colors_2[0][1] = 1;
		pokeball_colors_2[0][2] = 1;
	}

	// Calculate the trembling effect
	float tremble_amplitude = 1.0f; // Adjust this value to control the trembling intensity
	float tremble_frequency = 3.0f; // Adjust this value to control the trembling speed
	float tremble_offset = sin(timestamp * tremble_frequency) * tremble_amplitude;

	// Animate Pikachu's appearance and jumping
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f, 100 + jump_height, 0.0f));

	float spring_bounce = sin(timestamp * 0.05f) * 0.1f + 1.0f; // Adjust 0.05f and 0.1f to control speed and amplitude
	if (timestamp > time_table[THIRD_ATK].first) ModelMatrix = glm::scale(ModelMatrix, glm::vec3(pikachu_scale * spring_bounce, pikachu_scale / spring_bounce, pikachu_scale));
	else ModelMatrix = glm::scale(ModelMatrix, glm::vec3(pikachu_scale, pikachu_scale, pikachu_scale));

	if (in_time(FIRST_ATK) || in_time(FIFTH_ATK)) {
		// Apply the trembling effect to Pikachu's position
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(tremble_offset, tremble_offset, 0.0f));
	}
	if (timestamp > time_table[GAME_OVER].first) {
		// Animate Pikachu's death
		float time_since_death = timestamp - time_table[GAME_OVER].first;
		float fall_height = -20.0f * time_since_death;
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, fall_height, 0.0f));
	}
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_pikachu();


	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, -200 + jump_height, 0.0f));
	spring_bounce = sin((timestamp - 90 * TO_RADIAN) * 0.05f) * 0.1f + 1.0f; // Adjust 0.05f and 0.1f to control speed and amplitude
	// if (timestamp > time_table[THIRD_ATK].first) ModelMatrix = glm::scale(ModelMatrix, glm::vec3(voltorb_scale * spring_bounce, voltorb_scale / spring_bounce, pikachu_scale));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(voltorb_scale, voltorb_scale, voltorb_scale));

	if (in_time(SECOND_ATK) || in_time(FOURTH_ATK)) {
		// Apply the trembling effect to Pikachu's position
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(tremble_offset * 0.1, tremble_offset * 0.1, 0.0f));
	}
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_voltorb();

	if (in_time(THIRD_ATK)) {
		delx = 2.0f; delr = 15.0f; dels = 1.1f;
		x = -delx; r = delr; s = dels;
		// Cake
		for (i = 0; i < 240; i++, x -= delx, r += delr) {
			float oscillation = sin((timestamp + i * 3) * 0.1f) * 100.0f; // Adjust 0.01f and 30.0f to control speed and amplitude

			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(250, 100.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, shirt_clock * TO_RADIAN + r * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-50 + x, oscillation, 0.0f)); // Add oscillation to y-coordinate

			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_cake();
		}
	}
	if (in_time(SECOND_ATK)) {
		// Draw the cocktails falling like rain
		for (int i = 0; i < NUM_COCKTAILS; i++) {
			// Update the cocktail position
			cocktail_positions[i].y -= COCKTAIL_FALL_SPEED * (1.0 + i);
			if (cocktail_positions[i].y < -50.0f) {
				cocktail_positions[i].y = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 500.0f)) - 450;
				cocktail_positions[i].x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 500.0f)) - 550;
			}

			// Draw the cocktail with the updated position
			ModelMatrix = glm::translate(glm::mat4(1.0f), cocktail_positions[i]);
			ModelMatrix = glm::rotate(ModelMatrix, shirt_clock * TO_RADIAN + i * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.7f, 0.7f, 1.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_cocktail();
		}
	}

	if (in_time(FIFTH_ATK)) {
		const int num_shirts = 10;
		float lightning_speed = 2.5f; // Higher frequency for a more rigid effect
		float lightning_amplitude = 50.0f; // Adjust this to control the height of the lightning effect

		for (int i = 0; i < num_shirts; i++) {
			float x_pos = 150.0f + i * 20.0f; // Adjust the 20.0f value to control the distance between the shirts
			float y_offset = ceil(sin(timestamp * lightning_speed + i * 1.5f) * lightning_amplitude / 20.0f) * 50.0f;

			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x_pos, 100.0f + y_offset, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 2, 1.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_shirt_2();
		}
	}

	// Calculate the number of houses to draw based on the loading_percentage
	int num_houses_to_draw = (int)(10 * loading_percentage); // Assuming 10 houses in total

	// Set the house_spacing based on the width of the house and desired padding
	float house_spacing = 1.0f; // Adjust this value based on the house's width and desired padding
	if (timestamp > 200) {
		for (int i = 0; i < 35; i++) {
			for (int j = 0; j < 10; j++) {
				ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-350 + 10 * i, 100 + 10 * j, 0.0f));
				ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
				glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
				draw_shirt();
			}
		}
		for (int i = 0; i < 35; i++) {
			for (int j = 0; j < 10; j++) {
				ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(100 + 10 * i, -300 + 10 * j, 0.0f));
				ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
				glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
				draw_shirt();
			}
		}
	

		// Draw the houses with a constant horizontal spacing
		for (int i = 0; i < num_houses_to_draw; ++i) {
			// Calculate the x-coordinate for each house
			float house_x = -325.0f + i * house_spacing;

			// Set the model matrix for the house position and scaling
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_x, 150.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f, 0.5f, 1.0f));

			// Update the ModelViewProjectionMatrix and draw the house
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_house();
		}

		int num_houses_to_draw_2 = (int)(10 * loading_percentage_2); // Assuming 10 houses in total

		// Set the house_spacing based on the width of the house and desired padding
		// 
		// Draw the houses with a constant horizontal spacing
		for (int i = 0; i < num_houses_to_draw_2; ++i) {
			// Calculate the x-coordinate for each house
			float house_x = 125.0f + i * house_spacing;

			// Set the model matrix for the house position and scaling
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_x, -250.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f, 0.5f, 1.0f));

			// Update the ModelViewProjectionMatrix and draw the house
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_house_2();
		}
	}
	for (int i = 0; i < 3; i++) {
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 50 - 325 + pokeball_scale, 200, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, -18 * pokeball_scale * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(pokeball_scale, pokeball_scale, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_pokeball();
	}
	for (int i = 0; i < 3; i++) {
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 50 + 325 - pokeball_scale, -200, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, 18 * pokeball_scale * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(pokeball_scale, pokeball_scale, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_pokeball();
	}

	// Move the Pokeball to the right and spin it clockwise
	float move_speed = 5.0f; // Adjust this value to control the speed of the Pokeball's movement
	float spin_speed = 3.0f; // Adjust this value to control the speed of the Pokeball's spinning
	float xpos = -500.0f + move_speed * timestamp;
	float rotation_angle = -spin_speed * timestamp; // Negative sign to make it spin clockwise

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(xpos, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rotation_angle * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(300.0f, 300.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_pokeball();

	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void special(int key, int x, int y) {
#define SENSITIVITY 2.0
	switch (key) {
	case GLUT_KEY_LEFT:
		centerx -= SENSITIVITY;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		centerx += SENSITIVITY;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		centery -= SENSITIVITY;
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		centery += SENSITIVITY;
		glutPostRedisplay();
		break;
	}
}

int leftbuttonpressed = 0;
void mouse(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		leftbuttonpressed = 1;
		printf("%d %d\n", x, y);
	}
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
		leftbuttonpressed = 0;
}

void motion(int x, int y) {
	static int delay = 0;
	static float tmpx = 0.0, tmpy = 0.0;
	float dx, dy;
	if (leftbuttonpressed) {
		centerx =  x - win_width/2.0f, centery = (win_height - y) - win_height/2.0f;
		if (delay == 8) {	
			dx = centerx - tmpx;
			dy = centery - tmpy;
	  
			if (dx > 0.0) {
				rotate_angle = atan(dy / dx) + 90.0f*TO_RADIAN;
			}
			else if (dx < 0.0) {
				rotate_angle = atan(dy / dx) - 90.0f*TO_RADIAN;
			}
			else if (dx == 0.0) {
				if (dy > 0.0) rotate_angle = 180.0f*TO_RADIAN;
				else  rotate_angle = 0.0f;
			}
			tmpx = centerx, tmpy = centery; 
			delay = 0;
		}
		glutPostRedisplay();
		delay++;
	}
} 
	
void reshape(int width, int height) {
	win_width = width, win_height = height;
	
  	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0, 
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_line();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_line);
	glDeleteBuffers(1, &VBO_line);
	////
	glDeleteVertexArrays(1, &VAO_cake);
	glDeleteBuffers(1, &VBO_cake);

	glDeleteVertexArrays(1, &VAO_shirt);
	glDeleteBuffers(1, &VBO_shirt);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);

	glDeleteVertexArrays(1, &VAO_cocktail);
	glDeleteBuffers(1, &VBO_cocktail);

	glDeleteVertexArrays(1, &VAO_sword);
	glDeleteBuffers(1, &VBO_sword);
}


unsigned int elapsed_time = 0;
const unsigned int pikachu_delay = 100; 

void timer(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	// elapsed_time = (elapsed_time + 1) % UINT_MAX;

	// Update animation state
	// Increase the scale until the desired size is reached
	if (time_table[0].second <= timestamp) {
		if (pikachu_scale < 8.0f) {
			pikachu_scale += 0.3f;
		}
		else {
			// Make Pikachu jump three times
			if (jump_count < 4) {
				jump_height += value * 3.0f; // Adjust the speed of the jump by changing the multiplier
				if (jump_height >= 20.0f) { // Adjust the maximum jump height
					value = -value;
					jump_count++;
				}
				else if (jump_height <= 0.0f) {
					value = -value;
					jump_height = 0.0f;
				}
			}
		}
	}
	if (time_table[0].second <= timestamp) {
		if (voltorb_scale < 150.0f) {
			voltorb_scale += 3.0f;
		}
		else {
			if (jump_count < 4) {
				jump_height += value * 3.0f; // Adjust the speed of the jump by changing the multiplier
				if (jump_height >= 20.0f) { // Adjust the maximum jump height
					value = -value;
					jump_count++;
				}
				else if (jump_height <= 0.0f) {
					value = -value;
					jump_height = 0.0f;
				}
			}
		}
	}

	float color_d = 0.2;
	// Pokeball & status bar
	if (250 <= timestamp) {
		if (pokeball_scale < 20.0f)
			pokeball_scale += 0.5f;
	}
	if (in_time(FIRST_RESP)) {
		loading_percentage -= 0.1;
		house_color[0][0] += 0.1;
		house_color[0][1] -= 0.002;
	} 
	if (in_time(SECOND_RESP)) {
		loading_percentage_2 -= 0.05;
		house_color_2[0][0] += 0.05;
		house_color_2[0][1] -= 0.001;
	} 
	if (in_time(THIRD_RESP)) {
		loading_percentage -= 0.03;
		house_color[0][0] += 0.03;
		house_color[0][1] -= 0.0006;
	}
	if (in_time(FOURTH_RESP)) {
		loading_percentage_2 -= 0.08;
		house_color_2[0][0] += 0.08;
		house_color_2[0][1] -= 0.0016;
	} 
	if (in_time(FIFTH_RESP)) {
		loading_percentage -= 0.2;
		house_color[0][0] += 0.2;
		house_color[0][1] -= 0.004;
	}
	glutPostRedisplay();
	glutTimerFunc(16, timer, value); // 16 ms between frames for approximately 60 FPS
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutTimerFunc(0, timer, 1);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE); 

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glClearColor(104 / 255.0f, 180 / 255.0f, 49 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_line();
	prepare_sword();
	prepare_pikachu();
	prepare_pokeball();
	prepare_shirt();
	prepare_cake();
	prepare_cocktail();
	prepare_house();

	prepare_time_table();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

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

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 2
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 Simple2DTransformation_GLSL_3.0";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC', four arrows",
		"    - Mouse used: L-click and move"
	};

	glutInit (&argc, argv);
 	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize (1080*0.95, 720*0.95);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop ();
}


