//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;


unsigned int timestamp_scene = 0; // the global clock in the scene
GLint loc_blind_effect;


// for simple shaders
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_GRD; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 13
GLint loc_global_ambient_color, loc_global_ambient_color_GRD;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED], loc_light_GRD[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material, loc_material_GRD;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_ModelViewProjectionMatrix_GRD, loc_ModelViewMatrix_GRD, loc_ModelViewMatrixInvTrans_GRD;

GLint loc_flag_diffuse_texture_mapping, loc_flag_normal_texture_mapping, loc_flag_emissive_texture_mapping, loc_flag_normal_based_directX;
GLint loc_flag_fog; int flag_fog;

GLint loc_diffuse_texture, loc_flag_texture_mapping;
int cur_frame_tiger = 0;

// for skybox shaders
GLuint h_ShaderProgram_skybox;
GLint loc_cubemap_skybox;
GLint loc_ModelViewProjectionMatrix_SKY;

// texture id
#define TEXTURE_INDEX_DIFFUSE	(0)
#define TEXTURE_INDEX_NORMAL	(1)
#define TEXTURE_INDEX_SPECULAR	(2)
#define TEXTURE_INDEX_EMISSIVE	(3)
#define TEXTURE_INDEX_SKYMAP	(4)

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f


GLint loc_texture;



/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_1,
	CAMERA_2,
	CAMERA_3,
	CAMERA_4,
	CAMERA_5,
	CAMERA_6,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
} Camera;

Camera camera_info[NUM_CAMERAS];
Camera current_camera;

using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
}

int current_camera_index = CAMERA_1;



bool isPhong = true;
bool showCameraLight = false;

void set_current_camera(int camera_num) {
	Camera* pCamera = &camera_info[camera_num];
	current_camera_index = camera_num;
	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) {
	//CAMERA_1 : original view
	Camera* pCamera = &camera_info[CAMERA_1];
	for (int k = 0; k < 3; k++)
	{
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k];
		pCamera->vaxis[k] = scene.camera.v[k];
		pCamera->naxis[k] = scene.camera.n[k];
	}

	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_2 : bronze statue view
	pCamera = &camera_info[CAMERA_2];
	pCamera->pos[0] = -593.047974f; pCamera->pos[1] = -3758.460938f; pCamera->pos[2] = 474.587830f;
	pCamera->uaxis[0] = 0.864306f; pCamera->uaxis[1] = -0.502877f; pCamera->uaxis[2] = 0.009328f;
	pCamera->vaxis[0] = 0.036087f; pCamera->vaxis[1] = 0.080500f; pCamera->vaxis[2] = 0.996094f;
	pCamera->naxis[0] = -0.501662f; pCamera->naxis[1] = -0.860599f; pCamera->naxis[2] = 0.087724f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_3 : bronze statue view
	pCamera = &camera_info[CAMERA_3];
	pCamera->pos[0] = -1.463161f; pCamera->pos[1] = 1720.545166f; pCamera->pos[2] = 683.703491f;
	pCamera->uaxis[0] = -0.999413f; pCamera->uaxis[1] = -0.032568f; pCamera->uaxis[2] = -0.010066f;
	pCamera->vaxis[0] = -0.011190f; pCamera->vaxis[1] = -0.034529f; pCamera->vaxis[2] = 0.999328f;
	pCamera->naxis[0] = -0.032200f; pCamera->naxis[1] = 0.998855f; pCamera->naxis[2] = -0.034872f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_4 : top view
	pCamera = &camera_info[CAMERA_4];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 18300.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 1.0f; pCamera->vaxis[2] = 0.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 1.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_5 : front view
	pCamera = &camera_info[CAMERA_5];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 11700.0f; pCamera->pos[2] = 0.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 1.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;
	
	// 3400.000000, 0.000000, 2000.000000
	//CAMERA_6 : side view
	pCamera = &camera_info[CAMERA_6];
	pCamera->pos[0] = 3400.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 2000.0f;
	pCamera->uaxis[0] = 0.0f; pCamera->uaxis[1] = 1.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 1.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	set_current_camera(CAMERA_3);
}



/////////////// MODIFY /////////////////
void move_camera_20191559(Camera* camera, float dx, float dy, float dz) {
	if(current_camera_index == CAMERA_1) {
		camera->pos[0] += dx * camera->uaxis[0] - dy * camera->naxis[0] + dz * camera->vaxis[0];
		camera->pos[1] += dx * camera->uaxis[1] - dy * camera->naxis[1] + dz * camera->vaxis[1];
		camera->pos[2] += dx * camera->uaxis[2] - dy * camera->naxis[2] + dz * camera->vaxis[2];


		fprintf(stdout, "Camera moved to (%lf, %lf, %lf)\n", camera->pos[0], camera->pos[1], camera->pos[2]);
		// Update the view matrix
		set_ViewMatrix_from_camera_frame();
	}
}

void move_camera_vertical_20191559(Camera* camera, float dy) {
	if (current_camera_index == CAMERA_1) {
		camera->pos[2] += dy;
		fprintf(stdout, "Camera moved to (%lf, %lf, %lf)\n", camera->pos[0], camera->pos[1], camera->pos[2]);
		// Update the view matrix
		set_ViewMatrix_from_camera_frame();
	}
}

typedef enum { U, V, N } axis;

void sync_camera_axis_20191559(glm::mat3 R, glm::vec3 direc, int axis) {
	using namespace glm;
	switch (axis) {
	case U:
		direc = R * vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
		current_camera.vaxis[0] = direc.x;
		current_camera.vaxis[1] = direc.y;
		current_camera.vaxis[2] = direc.z;

		direc = R * vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
		current_camera.naxis[0] = direc.x;
		current_camera.naxis[1] = direc.y;
		current_camera.naxis[2] = direc.z;
		break;
	case V:
		direc = R * vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
		current_camera.uaxis[0] = direc.x;
		current_camera.uaxis[1] = direc.y;
		current_camera.uaxis[2] = direc.z;

		direc = R * vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
		current_camera.naxis[0] = direc.x;
		current_camera.naxis[1] = direc.y;
		current_camera.naxis[2] = direc.z;
		break;
	case N:
		direc = R * vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);

		current_camera.uaxis[0] = direc.x;
		current_camera.uaxis[1] = direc.y;
		current_camera.uaxis[2] = direc.z;

		direc = R * vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);

		current_camera.vaxis[0] = direc.x;
		current_camera.vaxis[1] = direc.y;
		current_camera.vaxis[2] = direc.z;
		break;
	}
}

void move_camera_rotate_20191559(int angle, int axis) {
	using namespace glm;
	vec3 direction;
	mat3 RotationMatrix;
	switch (axis) {
	case U:
		RotationMatrix = mat3(
			rotate(mat4(1.0f),
				angle * TO_RADIAN,
				vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2])
			)
		);
		sync_camera_axis_20191559(RotationMatrix, direction, U);
		break;
	case V:
		RotationMatrix = mat3(
			rotate(mat4(1.0f),
				angle * TO_RADIAN,
				vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2])
			)
		);
		sync_camera_axis_20191559(RotationMatrix, direction, V);
		break;
	case N:
		RotationMatrix = mat3(
			rotate(mat4(1.0f),
				angle * TO_RADIAN,
				vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2])
			)
		);
		sync_camera_axis_20191559(RotationMatrix, direction, N);
		break;

	}
	fprintf(stdout, "[ Rotation Matrix: \n u{%lf, %lf, %lf}\n v{%lf, %lf, %lf}\n n{%lf, %lf, %lf} ]\n",
		current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2],
		current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2],
		current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]
	);
}

////////////////////////////////////////


/*********************************  END: camera *********************************/

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) {
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	ShaderInfo shader_info_TXPS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

	loc_material.diffuseTex = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_material.normalTex = glGetUniformLocation(h_ShaderProgram_TXPS, "u_normal_texture");
	loc_material.emissiveTex = glGetUniformLocation(h_ShaderProgram_TXPS, "u_emissive_texture");


	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_flag_diffuse_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_diffuse_texture_mapping");
	loc_flag_normal_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_normal_texture_mapping");
	loc_flag_emissive_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_emissive_texture_mapping");
	loc_flag_normal_based_directX = glGetUniformLocation(h_ShaderProgram_TXPS, "u_normal_based_directX");

	///// GOURAUD /////////
	ShaderInfo shader_info_GRD[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_GRD = LoadShaders(shader_info_GRD);
	loc_ModelViewProjectionMatrix_GRD = glGetUniformLocation(h_ShaderProgram_GRD, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GRD = glGetUniformLocation(h_ShaderProgram_GRD, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GRD = glGetUniformLocation(h_ShaderProgram_GRD, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color_GRD = glGetUniformLocation(h_ShaderProgram_GRD, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light_GRD[i].light_on = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light_GRD[i].position = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light_GRD[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light_GRD[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light_GRD[i].specular_color = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light_GRD[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light_GRD[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light_GRD[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GRD, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light_GRD[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GRD, string);
	}

	loc_material_GRD.ambient_color = glGetUniformLocation(h_ShaderProgram_GRD, "u_material.ambient_color");
	loc_material_GRD.diffuse_color = glGetUniformLocation(h_ShaderProgram_GRD, "u_material.diffuse_color");
	loc_material_GRD.specular_color = glGetUniformLocation(h_ShaderProgram_GRD, "u_material.specular_color");
	loc_material_GRD.emissive_color = glGetUniformLocation(h_ShaderProgram_GRD, "u_material.emissive_color");
	loc_material_GRD.specular_exponent = glGetUniformLocation(h_ShaderProgram_GRD, "u_material.specular_exponent");

	loc_material_GRD.diffuseTex = glGetUniformLocation(h_ShaderProgram_GRD, "u_base_texture");
	loc_material_GRD.normalTex = glGetUniformLocation(h_ShaderProgram_GRD, "u_normal_texture");
	loc_material_GRD.emissiveTex = glGetUniformLocation(h_ShaderProgram_GRD, "u_emissive_texture");

	/*
	loc_texture_GRD = glGetUniformLocation(h_ShaderProgram_GRD, "u_base_texture");
	loc_flag_diffuse_texture_mapping_GRD = glGetUniformLocation(h_ShaderProgram_GRD, "u_flag_diffuse_texture_mapping");
	loc_flag_normal_texture_mapping = glGetUniformLocation(h_ShaderProgram_GRD, "u_flag_normal_texture_mapping");
	loc_flag_emissive_texture_mapping = glGetUniformLocation(h_ShaderProgram_GRD, "u_flag_emissive_texture_mapping");
	loc_flag_normal_based_directX = glGetUniformLocation(h_ShaderProgram_GRD, "u_normal_based_directX");
	*/
	///////////////////////



	ShaderInfo shader_info_skybox[3] = {
		{ GL_VERTEX_SHADER, "Shaders/skybox.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/skybox.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_skybox = LoadShaders(shader_info_skybox);
	loc_cubemap_skybox = glGetUniformLocation(h_ShaderProgram_skybox, "u_skymap");

	loc_ModelViewProjectionMatrix_SKY = glGetUniformLocation(h_ShaderProgram_skybox, "u_ModelViewProjectionMatrix");

}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

//axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {

	//set grid vertices
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//sun_temple
GLuint* sun_temple_VBO;
GLuint* sun_temple_VAO;
int* sun_temple_n_triangles;
int* sun_temple_vertex_offset;
GLfloat** sun_temple_vertices;
GLuint* sun_temple_texture_names;

bool* flag_texture_mapping;

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

// lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];

/******************************** START: objects ********************************/
// texture stuffs
#define TEXTURE_FLOOR				(0)
#define TEXTURE_TIGER				(1)
#define TEXTURE_WOLF				(2)
#define TEXTURE_SPIDER				(3)
#define TEXTURE_DRAGON				(4)
#define TEXTURE_OPTIMUS				(5)
#define TEXTURE_COW					(6)
#define TEXTURE_BUS					(7)
#define TEXTURE_BIKE				(8)
#define TEXTURE_GODZILLA			(9)
#define TEXTURE_IRONMAN				(17)
#define TEXTURE_TANK				(11)
#define TEXTURE_NATHAN				(12)
#define TEXTURE_OGRE				(13)
#define TEXTURE_CAT					(14)
#define TEXTURE_ANT					(15)
#define TEXTURE_TOWER				(16)
#define N_TEXTURES_USED				(17)

GLuint texture_names[N_TEXTURES_USED];
// bool flag_texture_mapping = true;

// texture id
#define TEXTURE_ID_DIFFUSE	(0)
#define TEXTURE_ID_NORMAL	(1)


void My_glTexImage2D_from_file(const char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	FreeImage_FlipVertical(tx_pixmap);

	fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);
}

int read_geometry_vnt(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);

	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

// for multiple materials
int read_geometry_vntm(GLfloat** object, int bytes_per_primitive,
	int* n_matrial_indicies, int** material_indicies,
	int* n_materials, char*** diffuse_texture_names,
	Material_Parameters** material_parameters,
	bool* bOnce,
	char* filename) {
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}

	int n_faces;
	fread(&n_faces, sizeof(int), 1, fp);

	*object = (float*)malloc(n_faces * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_faces, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	fread(n_matrial_indicies, sizeof(int), 1, fp);

	int bytes_per_indices = sizeof(int) * 2;
	*material_indicies = (int*)malloc(bytes_per_indices * (*n_matrial_indicies)); // material id, offset
	if (*material_indicies == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*material_indicies, bytes_per_indices, (*n_matrial_indicies), fp);

	if (*bOnce == false) {
		fread(n_materials, sizeof(int), 1, fp);

		*material_parameters = (Material_Parameters*)malloc(sizeof(Material_Parameters) * (*n_materials));
		*diffuse_texture_names = (char**)malloc(sizeof(char*) * (*n_materials));
		for (int i = 0; i < (*n_materials); i++) {
			fread((*material_parameters)[i].ambient_color, sizeof(float), 3, fp); //Ka
			fread((*material_parameters)[i].diffuse_color, sizeof(float), 3, fp); //Kd
			fread((*material_parameters)[i].specular_color, sizeof(float), 3, fp); //Ks
			fread(&(*material_parameters)[i].specular_exponent, sizeof(float), 1, fp); //Ns
			fread((*material_parameters)[i].emissive_color, sizeof(float), 3, fp); //Ke

			(*material_parameters)[i].ambient_color[3] = 1.0f;
			(*material_parameters)[i].diffuse_color[3] = 1.0f;
			(*material_parameters)[i].specular_color[3] = 1.0f;
			(*material_parameters)[i].emissive_color[3] = 1.0f;

			(*diffuse_texture_names)[i] = (char*)malloc(sizeof(char) * 256);
			fread((*diffuse_texture_names)[i], sizeof(char), 256, fp);
		}
		*bOnce = true;
	}

	fclose(fp);

	return n_faces;
}

void set_material(Material_Parameters* material_parameters) {
	// assume ShaderProgram_TXPS is used
	glUniform4fv(loc_material.ambient_color, 1, material_parameters->ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_parameters->diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_parameters->specular_color);
	glUniform1f(loc_material.specular_exponent, material_parameters->specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_parameters->emissive_color);
}

void set_material_GRD(Material_Parameters* material_parameters) {
	// assume ShaderProgram_GRD is used
	glUniform4fv(loc_material_GRD.ambient_color, 1, material_parameters->ambient_color);
	glUniform4fv(loc_material_GRD.diffuse_color, 1, material_parameters->diffuse_color);
	glUniform4fv(loc_material_GRD.specular_color, 1, material_parameters->specular_color);
	glUniform1f(loc_material_GRD.specular_exponent, material_parameters->specular_exponent);
	glUniform4fv(loc_material_GRD.emissive_color, 1, material_parameters->emissive_color);
}

void bind_texture(GLuint tex, int glTextureId, GLuint texture_name) {
	glActiveTexture(GL_TEXTURE0 + glTextureId);
	glBindTexture(GL_TEXTURE_2D, texture_name);
	glUniform1i(tex, glTextureId);
}

bool readTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		//fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_sun_temple(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	sun_temple_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	sun_temple_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	sun_temple_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	sun_temple_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

	// vertices
	sun_temple_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		sun_temple_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		sun_temple_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			sun_temple_vertex_offset[materialIdx] = 0;
		else
			sun_temple_vertex_offset[materialIdx] = sun_temple_vertex_offset[materialIdx - 1] + 3 * sun_temple_n_triangles[materialIdx - 1];

		glGenBuffers(1, &sun_temple_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, sun_temple_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			sun_temple_vertices[materialIdx], GL_STATIC_DRAW);

		// As the geometry data exists now in graphics memory, ...
		free(sun_temple_vertices[materialIdx]);

		// Initialize vertex array object.
		glGenVertexArrays(1, &sun_temple_VAO[materialIdx]);
		glBindVertexArray(sun_temple_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", scene.n_materials);

	// textures
	sun_temple_texture_names = (GLuint*)malloc(sizeof(GLuint) * scene.n_textures);
	glGenTextures(scene.n_textures, sun_temple_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded sun temple textures into graphics memory.\n");

	free(sun_temple_vertices);
}

void bindTexture(GLuint tex, int glTextureId, int texId) {
	if (INVALID_TEX_ID != texId) {
		glActiveTexture(GL_TEXTURE0 + glTextureId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);
		glUniform1i(tex, glTextureId);
	}
}

void draw_sun_temple(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		glUniform4fv(loc_material.ambient_color, 1, scene.material_list[materialIdx].shading.ph.ka);
		glUniform4fv(loc_material.diffuse_color, 1, scene.material_list[materialIdx].shading.ph.kd);
		glUniform4fv(loc_material.specular_color, 1, scene.material_list[materialIdx].shading.ph.ks);
		glUniform1f(loc_material.specular_exponent, scene.material_list[materialIdx].shading.ph.spec_exp);
		glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);

		int diffuseTexId = scene.material_list[materialIdx].diffuseTexId;
		int normalTexId = scene.material_list[materialIdx].normalMapTexId;
		int emissiveTexId = scene.material_list[materialIdx].emissiveTexId;

		bindTexture(loc_material.diffuseTex, TEXTURE_INDEX_DIFFUSE, diffuseTexId);
		glUniform1i(loc_flag_diffuse_texture_mapping, flag_texture_mapping[diffuseTexId]);
		bindTexture(loc_material.normalTex, TEXTURE_INDEX_NORMAL, normalTexId);
		glUniform1i(loc_flag_normal_texture_mapping, flag_texture_mapping[normalTexId]);
		glUniform1i(loc_flag_normal_based_directX, 1); // only for sun temple
		bindTexture(loc_material.emissiveTex, TEXTURE_INDEX_EMISSIVE, emissiveTexId);
		glUniform1i(loc_flag_emissive_texture_mapping, flag_texture_mapping[emissiveTexId]);

		glEnable(GL_TEXTURE_2D);

		glBindVertexArray(sun_temple_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * sun_temple_n_triangles[materialIdx]);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}

// skybox
GLuint skybox_VBO, skybox_VAO;
GLuint skybox_texture_name;

GLfloat cube_vertices[72][3] = { 
	{ -1.0f, -1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, 
	{ -1.0f, -1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, 
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, 
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, 
	{ 1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ -1.0f, -1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, 
	{ -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, 
	{ -1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, 
	{ -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, 
	{ 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, 
	{ 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 
	{ 1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 
	{ -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, 
	{ -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }
};

GLint loc_ViewMatrix_TXPS, loc_u_fragment_alpha, loc_u_flag_blending;


Material_Parameters material_cube;
float cube_alpha = 1.0f;
GLuint cube_VBO, cube_VAO;
void prepare_cube(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &cube_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), &cube_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &cube_VAO);
	glBindVertexArray(cube_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_cube.ambient_color[0] = 0.1745f;
	material_cube.ambient_color[1] = 0.01175f;
	material_cube.ambient_color[2] = 0.01175f;
	material_cube.ambient_color[3] = 1.0f;

	material_cube.diffuse_color[0] = 0.61424f;
	material_cube.diffuse_color[1] = 0.04136f;
	material_cube.diffuse_color[2] = 0.04136f;
	material_cube.diffuse_color[3] = 1.0f;

	material_cube.specular_color[0] = 0.727811f;
	material_cube.specular_color[1] = 0.626959f;
	material_cube.specular_color[2] = 0.626959f;
	material_cube.specular_color[3] = 1.0f;

	material_cube.specular_exponent = 76.8f;

	material_cube.emissive_color[0] = 0.0f;
	material_cube.emissive_color[1] = 0.0f;
	material_cube.emissive_color[2] = 0.0f;
	material_cube.emissive_color[3] = 1.0f;

	cube_alpha = 0.5f;
}

int isTransparent = 1;
float transparency = 5.0f;

void set_material_cube(void) {
	cube_alpha = isTransparent == 1 ? transparency / 10.0f : 1.0f;

	glUniform4fv(loc_material.ambient_color, 1, material_cube.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_cube.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_cube.specular_color);
	glUniform1f(loc_material.specular_exponent, material_cube.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_cube.emissive_color);
}


void draw_cube(void) {
	glFrontFace(GL_CCW);
	glBindVertexArray(cube_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	glBindVertexArray(0);
}

bool flag_blend_mode = false; // for blending
float rotation_angle_cube = 0.0f;  // for cube rotation

void draw_cube_bf(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	if (flag_blend_mode) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(loc_u_flag_blending, true);
		glEnable(GL_BLEND);
		//glDisable(GL_DEPTH_TEST);
	}
	else
		glUniform1i(loc_u_flag_blending, false);



	cube_alpha = 0.5;
	// draw cube
	glEnable(GL_CULL_FACE);


	/*
	material_cube.ambient_color[3] = cube_alpha;
	material_cube.diffuse_color[3] = cube_alpha;
	material_cube.specular_color[3] = cube_alpha;
	material_cube.emissive_color[3] = cube_alpha;
	*/

	set_material_cube();
	glUniform1f(loc_u_fragment_alpha, cube_alpha);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(2400.0f, 0.0f, 2000.0));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(15.0f, 15.0f, 15.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_angle_cube, glm::vec3(1.0f, 1.0f, 1.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glUniformMatrix4fv(loc_ViewMatrix_TXPS, 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniform1i(loc_flag_texture_mapping, false);
	glUniform1i(loc_flag_fog, false);

	glCullFace(GL_FRONT);
	draw_cube();

	glCullFace(GL_BACK);
	draw_cube();


	glUniform1i(loc_u_flag_blending, false);

	glDisable(GL_CULL_FACE);
	if (flag_blend_mode) {
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	glUseProgram(0);
}



void readTexImage2DForCubeMap(const char* filename, GLenum texture_target) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	FreeImage_FlipVertical(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(texture_target, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap);
}

void prepare_skybox(void) { // Draw skybox.
	glGenVertexArrays(1, &skybox_VAO);
	glGenBuffers(1, &skybox_VBO);

	glBindVertexArray(skybox_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
	glBufferData(GL_ARRAY_BUFFER, 36 * 3 * sizeof(GLfloat), &cube_vertices[0][0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &skybox_texture_name);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	readTexImage2DForCubeMap("Scene/Cubemap/px.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/nx.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/py.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/ny.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/pz.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	readTexImage2DForCubeMap("Scene/Cubemap/nz.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	fprintf(stdout, " * Loaded cube map textures into graphics memory.\n\n");

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void draw_skybox(void) {
	glUseProgram(h_ShaderProgram_skybox);

	glUniform1i(loc_cubemap_skybox, TEXTURE_INDEX_SKYMAP);

	ModelViewMatrix = ViewMatrix * glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
											0.0f, 0.0f, 1.0f, 0.0f,
											0.0f, 1.0f, 0.0f, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f);
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20000.0f, 20000.0f, 20000.0f));
	//ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(20000.0f, 20000.0f, 20000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_SKY, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(skybox_VAO);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_SKYMAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);
}





/// TIGER ///

// for tiger animation
int flag_tiger_animation, flag_polygon_fill;
int cur_frame_ben = 0, cur_frame_wolf, cur_frame_spider = 0;
float rotation_angle_tiger = 0.0f;
// tiger object
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];

Material_Parameters material_tiger;

void prepare_tiger(void) { // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/tiger/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry_vnt(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_TIGER);
	glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[TEXTURE_TIGER]);

	// My_glTexImage2D_from_file("Data/dynamic_objects/tiger/tiger_tex.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
}

// spider object
#define N_SPIDER_FRAMES 16
GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLfloat* spider_vertices[N_SPIDER_FRAMES];

Material_Parameters material_spider;

void prepare_spider(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, spider_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_SPIDER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/spider/spider_vnt_%d%d.geom", i / 10, i % 10);
		spider_n_triangles[i] = read_geometry_vnt(&spider_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		spider_n_total_triangles += spider_n_triangles[i];

		if (i == 0)
			spider_vertex_offset[i] = 0;
		else
			spider_vertex_offset[i] = spider_vertex_offset[i - 1] + 3 * spider_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &spider_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glBufferData(GL_ARRAY_BUFFER, spider_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, spider_vertex_offset[i] * n_bytes_per_vertex,
			spider_n_triangles[i] * n_bytes_per_triangle, spider_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_SPIDER_FRAMES; i++)
		free(spider_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &spider_VAO);
	glBindVertexArray(spider_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_SPIDER);
	glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[TEXTURE_SPIDER]);

	My_glTexImage2D_from_file("Image/spider_tex.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}



// ironman object
glm::mat4 ModelMatrix_ironman;
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;

Material_Parameters material_ironman;

void prepare_ironman(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry_vnt(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW); // TODO


	// as the geometry data exists now in graphics memory, ...
	free(ironman_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	// glActiveTexture(GL_TEXTURE0 + TEXTURE_IRONMAN); // TODO
	// glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[TEXTURE_IRONMAN]);

	// My_glTexImage2D_from_file("Data/static_objects/Ironman_Body.png");


	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


glm::mat4 ModelMatrix_tiger;
glm::vec3 tiger_position(0.0f, 0.0f, 80.0f); // Tiger's position
float tiger_speed = 0.01f; // Speed of the tiger
float tiger_radius = 200.0f; // Radius of the circle
float tiger_oscillation = 30.0f; // Oscillation amplitude for the "wiggly" effect
glm::vec3 tiger_direction(0.0f, 0.0f, 0.0f); // Direction of the tiger



void draw_tiger(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glFrontFace(GL_CW);
	// glUseProgram(h_ShaderProgram_TXPS);
	// glUseProgram(h_ShaderProgram_GRD);
	glUniform1i(loc_flag_texture_mapping, false);
	
	
	
	// MOVE //
	ModelMatrix_tiger = glm::mat4(1.0f); // start with identity matrix
	ModelMatrix_tiger = glm::scale(ModelMatrix_tiger, glm::vec3(3.0f, 3.0f, 3.0f));
	ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, tiger_position); // Use tiger position
	
	
	float angle = atan2(tiger_direction.y, tiger_direction.x);
	ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis


	
	ModelViewMatrix = ViewMatrix * ModelMatrix_tiger;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));


	if (isPhong) {
		set_material(&material_tiger);
		glUseProgram(h_ShaderProgram_TXPS);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		// glUniformMatrix4fv(loc_ViewMatrix_TXPS, 1, GL_FALSE, &ViewMatrix[0][0]); // TODO

		glUniform4f(loc_material.ambient_color, 0.24725, 0.1995, 0.0745, 0.1);
		glUniform4f(loc_material.diffuse_color, 0.35164, 0.60648, 0.22648, 0.3);
		glUniform4f(loc_material.specular_color, 0.628281, 0.555802, 0.366065, 0.1);
		glUniform1f(loc_material.specular_exponent, 1.2);
		glUniform4f(loc_material.emissive_color, 0.1, 0.1, 0.0, 0.1);

		glUniform1i(loc_texture, scene.n_textures);
		glUniform1i(loc_flag_texture_mapping, false);
	}
	else {
		set_material_GRD(&material_tiger);
		glUseProgram(h_ShaderProgram_GRD);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GRD, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GRD, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GRD, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		// glUniformMatrix4fv(loc_ViewMatrix_GRD, 1, GL_FALSE, &ViewMatrix[0][0]);

		glUniform4f(loc_material_GRD.ambient_color, 0.24725, 0.1995, 0.0745, 0.1);
		glUniform4f(loc_material_GRD.diffuse_color, 0.35164, 0.20648, 0.22648, 0.3);
		glUniform4f(loc_material_GRD.specular_color, 0.328281, 0.455802, 0.466065, 0.1);
		glUniform1f(loc_material_GRD.specular_exponent, 1.2);
		glUniform4f(loc_material_GRD.emissive_color, 0.1, 0.1, 0.0, 0.1);

		glUniform1i(loc_texture, scene.n_textures);
		glUniform1i(loc_flag_texture_mapping, false);
	}


	GLfloat tiger_color[3] = { 255.0f / 255.0f, 114.0f / 255.0f, 0.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, tiger_color);


	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_TIGER); // Use the correct texture unit
	glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[TEXTURE_TIGER]); // Bind the tiger texture
	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	// glutPostRedisplay();
}

glm::mat4 ModelMatrix_spider;
glm::vec3 spider_position(0.0f, 0.0f, 0.0f); // Tiger's position
float spider_speed = 0.01f; // Speed of the tiger
float spider_radius = 200.0f; // Radius of the circle
float spider_oscillation = 30.0f; // Oscillation amplitude for the "wiggly" effect
glm::vec3 spider_direction(0.0f, 0.0f, 0.0f); // Direction of the tiger


void draw_spider(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glFrontFace(GL_CW);
	// glUseProgram(h_ShaderProgram_TXPS);
	// glUseProgram(h_ShaderProgram_GRD);
	glUniform1i(loc_flag_texture_mapping, false);

	glm::vec3 center = glm::vec3(0.0f, 0.0f, 1700.0f);
	float radius = 750.0f;
	float theta = ((sin((timestamp_scene + 80) / 30.0f) + 1) * glm::pi<float>());  // Range [0, 2pi]
	float phi = ((cos((timestamp_scene + 80) / 30.0f) + 1) * glm::pi<float>()) / 4.0f;  // Range [0, pi/2]

	glm::vec3 new_position;
	new_position.x = center.x + radius * sin(phi) * cos(theta);
	new_position.y = center.y + radius * sin(phi) * sin(theta);
	new_position.z = center.z + radius * cos(phi);
	spider_position = new_position;

	// Compute the direction from the spider's position to the center of the hemisphere
	glm::vec3 down_direction = glm::normalize(center - spider_position);



	// Compute the rotation matrix that aligns the y-axis with the computed direction
	glm::vec3 y_axis = glm::vec3(0.0f, 1.0f, 0.0f); // Assuming the spider's "down" direction is the negative y-axis
	glm::vec3 axis = glm::cross(y_axis, down_direction);
	float angle = glm::acos(glm::dot(y_axis, down_direction));
	glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle, axis);

	ModelMatrix_spider = glm::translate(glm::mat4(1.0f), spider_position); // Use spider position
	ModelMatrix_spider *= rotation_matrix; // Apply the rotation
	ModelMatrix_spider = glm::scale(ModelMatrix_spider, glm::vec3(200.0f, 200.0f, 200.0f));

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_spider;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));
	
	 // This is the position of the light in the Spider's Model Coordinate System (MCS)
	glm::vec4 light_position_in_spider_mcs = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // replace with actual position

	// Compute the light's position in the World Coordinate System (WCS) 
	glm::vec4 light_position_in_wcs = ModelMatrix_spider * light_position_in_spider_mcs;

	// Update the light's position
	light[10].position[0] = light_position_in_wcs.x;
	light[10].position[1] = light_position_in_wcs.y;
	light[10].position[2] = light_position_in_wcs.z;
	light[10].position[3] = light_position_in_wcs.w;  // If w = 0, the light source is at infinity

	// Then pass it to the shader
	glm::vec4 position_EC = ViewMatrix * light_position_in_wcs; // Convert position to Eye Coordinate System (ECS)
	glUniform4fv(loc_light[10].position, 1, &position_EC[0]);

	// Light direction in Spider's Model Coordinate System (MCS)
	glm::vec3 light_direction_in_spider_mcs = glm::vec3(0.0f, -1.0f, 0.0f); // replace with actual direction

	// Apply the same rotation to the light's direction
	glm::vec3 light_direction_in_wcs = glm::mat3(rotation_matrix) * light_direction_in_spider_mcs;

	// Update the light's direction
	light[10].spot_direction[0] = light_direction_in_wcs.x;
	light[10].spot_direction[1] = light_direction_in_wcs.y;
	light[10].spot_direction[2] = light_direction_in_wcs.z;

	// Then pass it to the shader
	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * light_direction_in_wcs; // Convert direction to Eye Coordinate System (ECS)
	glUniform3fv(loc_light[10].spot_direction, 1, &direction_EC[0]);


	if (isPhong) {
		set_material(&material_spider);
		glUseProgram(h_ShaderProgram_TXPS);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		// glUniformMatrix4fv(loc_ViewMatrix_TXPS, 1, GL_FALSE, &ViewMatrix[0][0]); // TODO

		glUniform4f(loc_material.ambient_color, 0.24725, 0.1995, 0.0745, 0.1);
		glUniform4f(loc_material.diffuse_color, 0.35164, 0.60648, 0.22648, 0.3);
		glUniform4f(loc_material.specular_color, 0.628281, 0.555802, 0.366065, 0.1);
		glUniform1f(loc_material.specular_exponent, 1.2);
		glUniform4f(loc_material.emissive_color, 0.1, 0.1, 0.0, 0.1);

		glUniform1i(loc_texture, scene.n_textures);
		glUniform1i(loc_flag_texture_mapping, false);
	}
	else {
		set_material_GRD(&material_spider);
		glUseProgram(h_ShaderProgram_GRD);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GRD, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GRD, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GRD, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		// glUniformMatrix4fv(loc_ViewMatrix_GRD, 1, GL_FALSE, &ViewMatrix[0][0]);

		glUniform4f(loc_material_GRD.ambient_color, 0.24725, 0.1995, 0.0745, 0.1);
		glUniform4f(loc_material_GRD.diffuse_color, 0.35164, 0.20648, 0.22648, 0.3);
		glUniform4f(loc_material_GRD.specular_color, 0.328281, 0.455802, 0.466065, 0.1);
		glUniform1f(loc_material_GRD.specular_exponent, 1.2);
		glUniform4f(loc_material_GRD.emissive_color, 0.1, 0.1, 0.0, 0.1);

		glUniform1i(loc_texture, scene.n_textures);
		glUniform1i(loc_flag_texture_mapping, false);
	}


	GLfloat tiger_color[3] = { 255.0f / 255.0f, 114.0f / 255.0f, 0.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, tiger_color);


	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_SPIDER); // Use the correct texture unit
	glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[TEXTURE_SPIDER]); // Bind the spider texture
	glBindVertexArray(spider_VAO);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	glutPostRedisplay();
}


glm::vec3 ironman_position(-432.0f, 1417.0f, 1377.0f); // ironman's position

void draw_ironman(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// glUseProgram(h_ShaderProgram_simple);

	glFrontFace(GL_CW);
	glUniform1i(loc_flag_texture_mapping, true);

	// MOVE //
	ModelMatrix_ironman = glm::translate(glm::mat4(1.0f), ironman_position); // Use spider position
	ModelMatrix_ironman = glm::scale(ModelMatrix_ironman, glm::vec3(200.0f, 200.0f, 200.0f));
	// ModelMatrix_ironman = glm::rotate(ModelMatrix_ironman, 240 * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix_ironman = glm::rotate(ModelMatrix_ironman, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_ironman;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	if (isPhong) {
		set_material(&material_ironman);
		glUseProgram(h_ShaderProgram_TXPS);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		// glUniformMatrix4fv(loc_ViewMatrix_TXPS, 1, GL_FALSE, &ViewMatrix[0][0]); // TODO

		glUniform4f(loc_material.ambient_color, 0.24725, 0.1995, 0.0745, 0.1);
		glUniform4f(loc_material.diffuse_color, 0.35164, 0.60648, 0.22648, 0.3);
		glUniform4f(loc_material.specular_color, 0.628281, 0.555802, 0.366065, 0.1);
		glUniform1f(loc_material.specular_exponent, 1.2);
		glUniform4f(loc_material.emissive_color, 0.1, 0.1, 0.0, 0.1);

		glUniform1i(loc_texture, scene.n_textures);
		glUniform1i(loc_flag_texture_mapping, true);
	}
	else {
		set_material_GRD(&material_ironman);
		glUseProgram(h_ShaderProgram_GRD);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GRD, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GRD, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GRD, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		// glUniformMatrix4fv(loc_ViewMatrix_TXPS, 1, GL_FALSE, &ViewMatrix[0][0]); // TODO

		glUniform4f(loc_material_GRD.ambient_color, 0.24725, 0.1995, 0.0745, 0.1);
		glUniform4f(loc_material_GRD.diffuse_color, 0.35164, 0.60648, 0.22648, 0.3);
		glUniform4f(loc_material_GRD.specular_color, 0.628281, 0.555802, 0.366065, 0.1);
		glUniform1f(loc_material_GRD.specular_exponent, 1.2);
		glUniform4f(loc_material_GRD.emissive_color, 0.1, 0.1, 0.0, 0.1);

		glUniform1i(loc_texture, scene.n_textures);
		glUniform1i(loc_flag_texture_mapping, true);
	}

	GLfloat ironman_color[3] = { 224.0f / 255.0f, 70.0f / 255.0f, 82.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, ironman_color);
	/*
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_IRONMAN); // Use the correct texture unit
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_IRONMAN]); // Bind the spider texture
	
	// bind_texture(loc_diffuse_texture, TEXTURE_ID_DIFFUSE, sun_temple_texture_names[TEXTURE_IRONMAN]);
	
	glBindVertexArray(ironman_VAO);
	// glDrawArrays(GL_TRIANGLES, ironman_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	*/

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_SPIDER); // Use the correct texture unit
	glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[TEXTURE_SPIDER]); // Bind the spider texture
	glBindVertexArray(ironman_VAO);
	// glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);


	// glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
}

/////////////


/*****************************  END: geometry setup *****************************/

/********************************  START: light *********************************/

void initialize_lights(void) { // follow OpenGL conventions for initialization

	for (int i = 0; i < scene.n_lights; i++) {
		light[i].light_on = 1;

		if (LIGHT_DIRECTIONAL == scene.light_list[i].type) {
			light[i].position[0] = scene.light_list[i].pos[0];
			light[i].position[1] = scene.light_list[i].pos[1];
			light[i].position[2] = scene.light_list[i].pos[2];
			light[i].position[3] = 0.0f;

			light[i].ambient_color[0] = 0.0f;
			light[i].ambient_color[1] = 0.0f;
			light[i].ambient_color[2] = 0.0f;
			light[i].ambient_color[3] = 1.0f;

			light[i].diffuse_color[0] = scene.light_list[i].color[0];
			light[i].diffuse_color[1] = scene.light_list[i].color[1];
			light[i].diffuse_color[2] = scene.light_list[i].color[2];
			light[i].diffuse_color[3] = 1.0f;

			light[i].specular_color[0] = 0.5f; 
			light[i].specular_color[1] = 0.5f;
			light[i].specular_color[2] = 0.5f; 
			light[i].specular_color[3] = 1.0f;
		}
		else if (LIGHT_POINT == scene.light_list[i].type) {
			light[i].position[0] = scene.light_list[i].pos[0];
			light[i].position[1] = scene.light_list[i].pos[1];
			light[i].position[2] = scene.light_list[i].pos[2];
			light[i].position[3] = scene.light_list[i].pos[3];

			light[i].ambient_color[0] = 0.0f;
			light[i].ambient_color[1] = 0.0f;
			light[i].ambient_color[2] = 0.0f;
			light[i].ambient_color[3] = 1.0f;

			light[i].diffuse_color[0] = scene.light_list[i].color[0];
			light[i].diffuse_color[1] = scene.light_list[i].color[1];
			light[i].diffuse_color[2] = scene.light_list[i].color[2];
			light[i].diffuse_color[3] = 1.0f;

			light[i].specular_color[0] = 0.8f; 
			light[i].specular_color[1] = 0.8f;
			light[i].specular_color[2] = 0.8f; 
			light[i].specular_color[3] = 1.0f;

			light[i].light_attenuation_factors[0] = 1.0f;
			light[i].light_attenuation_factors[1] = 0.01;
			light[i].light_attenuation_factors[2] = 0.0f;
			light[i].light_attenuation_factors[3] = 1.0f;

			light[i].spot_cutoff_angle = 180.0f;


		}
		else {
			// for spot light, volume light, ...
		}
	}
	// WC fixed
	light[11].light_on = 0;
	light[11].position[0] = 100;		light[11].position[1] = 100;
	light[11].position[2] = 50;			light[11].position[3] = 0;

	light[11].ambient_color[0] = 1.0;		light[11].ambient_color[1] = 0;
	light[11].ambient_color[2] = 0;		light[11].ambient_color[3] = 1;

	light[11].diffuse_color[0] = 1.0;	light[11].diffuse_color[1] = 0;
	light[11].diffuse_color[2] = 0;		light[11].diffuse_color[3] = 1;

	light[11].specular_color[0] = 0;	light[11].specular_color[1] = 0;
	light[11].specular_color[2] = 1;	light[11].specular_color[3] = 1;

	light[11].spot_direction[0] = 0;	light[11].spot_direction[1] = -1;
	light[11].spot_direction[2] = 0;

	light[11].spot_exponent = 10;		light[11].spot_cutoff_angle = 1;

	light[11].light_attenuation_factors[0] = 1;	light[11].light_attenuation_factors[1] = 0;
	light[11].light_attenuation_factors[2] = 0;	light[11].light_attenuation_factors[3] = 1;


	// EC moving
	light[12].light_on = 0;
	light[12].position[0] = 100;		light[12].position[1] = 100;
	light[12].position[2] = 50;			light[12].position[3] = 1;
	
	light[12].ambient_color[0] = 0;		light[12].ambient_color[1] = 0;
	light[12].ambient_color[2] = 1;		light[12].ambient_color[3] = 1;

	light[12].diffuse_color[0] = 0.82;	light[12].diffuse_color[1] = 0.3;
	light[12].diffuse_color[2] = 1;		light[12].diffuse_color[3] = 1;

	light[12].specular_color[0] = 0;	light[12].specular_color[1] = 0;
	light[12].specular_color[2] = 1;	light[12].specular_color[3] = 1;

	light[12].spot_direction[0] = 0;	light[12].spot_direction[1] = -1;
	light[12].spot_direction[2] = 0;

	light[12].spot_exponent = 10;		light[12].spot_cutoff_angle = 5;
	
	light[12].light_attenuation_factors[0] = 1;	light[12].light_attenuation_factors[1] = 0;
	light[12].light_attenuation_factors[2] = 0;	light[12].light_attenuation_factors[3] = 1;



	// MC fixed
	light[10].light_on = 0;
	light[10].position[0] = 0;		light[10].position[1] = 0;
	light[10].position[2] = 0;			light[10].position[3] = 1;

	light[10].ambient_color[0] = 0.1;	light[10].ambient_color[1] = 1;
	light[10].ambient_color[2] = 0;		light[10].ambient_color[3] = 1;

	light[10].diffuse_color[0] = 0.1;	light[10].diffuse_color[1] = 1;
	light[10].diffuse_color[2] = 0;		light[10].diffuse_color[3] = 1;

	light[10].specular_color[0] = 0;	light[10].specular_color[1] = 1;
	light[10].specular_color[2] = 0;	light[10].specular_color[3] = 1;

	light[10].spot_direction[0] = 0;	light[10].spot_direction[1] = -1;
	light[10].spot_direction[2] = 0;

	light[10].spot_exponent = 10;		light[10].spot_cutoff_angle = 50;

	light[10].light_attenuation_factors[0] = 1;	light[10].light_attenuation_factors[1] = 0;
	light[10].light_attenuation_factors[2] = 0;	light[10].light_attenuation_factors[3] = 1;


	
	if (current_camera_index == CAMERA_1) {

		glm::vec3 direction_EC = glm::mat3(ModelViewMatrix) *
			glm::vec3(light[11].spot_direction[0], light[11].spot_direction[1], light[11].spot_direction[2]);

		glUniform3fv(loc_light[11].spot_direction, 1, &direction_EC[0]);
	}
	
}

void set_lights(void) {
	if (isPhong) {
		glUseProgram(h_ShaderProgram_TXPS);

		glUniform4f(loc_global_ambient_color, 0.3f, 0.3f, 0.3f, 1.0f);

		glm::vec4 light_position_EC;
		glm::vec3 light_direction_EC;
		for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
			glUniform1i(loc_light[i].light_on, light[i].light_on);

			light_position_EC = ViewMatrix * glm::vec4(light[i].position[0], light[i].position[1], light[i].position[2], light[i].position[3]);
			glUniform4fv(loc_light[i].position, 1, &light_position_EC[0]);

			glUniform4fv(loc_light[i].ambient_color, 1, light[i].ambient_color);
			glUniform4fv(loc_light[i].diffuse_color, 1, light[i].diffuse_color);
			glUniform4fv(loc_light[i].specular_color, 1, light[i].specular_color);

			if (0.0f != light[i].position[3])
			{
				light_direction_EC = glm::transpose(glm::inverse(glm::mat3(ViewMatrix))) * glm::vec3(light[i].spot_direction[0], light[i].spot_direction[1], light[i].spot_direction[2]);
				glUniform3fv(loc_light[i].spot_direction, 1, &light_direction_EC[0]);
				glUniform1f(loc_light[i].spot_exponent, light[i].spot_exponent);
				glUniform1f(loc_light[i].spot_cutoff_angle, light[i].spot_cutoff_angle);
				glUniform4fv(loc_light[i].light_attenuation_factors, 1, light[i].light_attenuation_factors);
			}
		}
	}
	else {
		glUseProgram(h_ShaderProgram_GRD);

		glUniform4f(loc_global_ambient_color, 0.3f, 0.3f, 0.3f, 1.0f);

		glm::vec4 light_position_EC;
		glm::vec3 light_direction_EC;
		for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
			glUniform1i(loc_light_GRD[i].light_on, light[i].light_on);

			light_position_EC = ViewMatrix * glm::vec4(light[i].position[0], light[i].position[1], light[i].position[2], light[i].position[3]);
			glUniform4fv(loc_light_GRD[i].position, 1, &light_position_EC[0]);

			glUniform4fv(loc_light_GRD[i].ambient_color, 1, light[i].ambient_color);
			glUniform4fv(loc_light_GRD[i].diffuse_color, 1, light[i].diffuse_color);
			glUniform4fv(loc_light_GRD[i].specular_color, 1, light[i].specular_color);

			if (0.0f != light[i].position[3])
			{
				light_direction_EC = glm::transpose(glm::inverse(glm::mat3(ViewMatrix))) * glm::vec3(light[i].spot_direction[0], light[i].spot_direction[1], light[i].spot_direction[2]);
				glUniform3fv(loc_light_GRD[i].spot_direction, 1, &light_direction_EC[0]);
				glUniform1f(loc_light_GRD[i].spot_exponent, light[i].spot_exponent);
				glUniform1f(loc_light_GRD[i].spot_cutoff_angle, light[i].spot_cutoff_angle);
				glUniform4fv(loc_light_GRD[i].light_attenuation_factors, 1, light[i].light_attenuation_factors);
			}
		}
	}
	glUseProgram(0);
}
/*********************************  END: light **********************************/

/********************  START: callback function definitions *********************/
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_lights();

	draw_grid();
	draw_axes();
	draw_sun_temple();
	draw_skybox();

	draw_tiger();
	draw_ironman();
	draw_spider();


	glEnable(GL_MULTISAMPLE);
	draw_cube_bf();
	glDisable(GL_MULTISAMPLE);

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	static int flag_blind_effect = 0;

	static int flag_check_effect = 0;
	static int flag_wave_effect = 0;
	static int flag_water_effect = 0;

	GLint uniformLocation = glGetUniformLocation(h_ShaderProgram_TXPS, "u_blind_effect");

	GLint uniformLocation1 = glGetUniformLocation(h_ShaderProgram_TXPS, "u_check_effect");
	GLint uniformLocation2 = glGetUniformLocation(h_ShaderProgram_TXPS, "u_wave_effect");
	GLint uniformLocation3 = glGetUniformLocation(h_ShaderProgram_TXPS, "u_water_effect");


	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	
	case 'u':
		set_current_camera(CAMERA_3);
		set_ViewMatrix_from_camera_frame();
		glutPostRedisplay();
		break;
	
	case 'i':
		isPhong = true;
		glutPostRedisplay();


		set_current_camera(CAMERA_1);
		glutPostRedisplay();
		break;
	
	case 'o':
		isPhong = true;
		glutPostRedisplay();
		set_current_camera(CAMERA_6);
		glutPostRedisplay();
		break;

	case '1':
		if (current_camera_index == CAMERA_3) {
			isPhong = false;
			glutPostRedisplay();
		}
		break;
	case '2':
		isPhong = true;
		glutPostRedisplay();
		break;
	case '3':
		if (light[11].light_on) {
			isPhong = true;
			glutPostRedisplay();
			light[11].light_on = 0;
		}
		else {
			isPhong = true;
			light[11].light_on = 1;
		}

		glutPostRedisplay();
		break;
	case '4':
		if (light[12].light_on) {
			isPhong = true;
			glutPostRedisplay();
			light[12].light_on = 0;
		}
		else {
			isPhong = true;
			light[12].light_on = 1;
		}

		glutPostRedisplay();
		break;

	case '5':
		if (light[10].light_on) {
			isPhong = true;
			glutPostRedisplay();
			light[10].light_on = 0;
		}
		else {
			isPhong = true;
			light[10].light_on = 1;
		}

		glutPostRedisplay();
		break;

	case '7':
		glUseProgram(h_ShaderProgram_TXPS);
		flag_check_effect = 1 - flag_check_effect;

		glUniform1f(uniformLocation2, false);
		glUniform1f(uniformLocation3, false);
		glUniform1f(uniformLocation1, flag_check_effect);

		glUseProgram(0);
		glutPostRedisplay();
		break;
	case '8':
		glUseProgram(h_ShaderProgram_TXPS);
		flag_water_effect = 1 - flag_water_effect;

		glUniform1f(uniformLocation1, false);
		glUniform1f(uniformLocation3, false);
		glUniform1f(uniformLocation2, flag_water_effect);

		glUseProgram(0);
		glutPostRedisplay();
		break;
	case '9':
		glUseProgram(h_ShaderProgram_TXPS);
		flag_wave_effect = 1 - flag_wave_effect;
		
		glUniform1f(uniformLocation1, false);
		glUniform1f(uniformLocation2, false);
		glUniform1f(uniformLocation3, flag_wave_effect);

		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

// added//
void special_20191559(int key, int x, int y) {
	int mods;
	switch (key) {
	case GLUT_KEY_LEFT:
		move_camera_20191559(&current_camera, -50.0f, 0.0f, 0.0f);  // move left
		glutPostRedisplay();
		fprintf(stdout, "move left\n");
		break;
	case GLUT_KEY_UP:    // If the UP key is pressed
		mods = glutGetModifiers();   // Get the state of modifier keys
		if (mods == GLUT_ACTIVE_SHIFT) {   // If the Shift key is also pressed
			move_camera_vertical_20191559(&current_camera, 100.0f);  // Move the camera up
			fprintf(stdout, "move up\n");
		}
		else {
			move_camera_20191559(&current_camera, 0.0f, 200.0f, 0.0f); // Move camera forward
			fprintf(stdout, "move forward\n");
		}
		glutPostRedisplay();
		break;

	case GLUT_KEY_DOWN: // If the DOWN key is pressed
		mods = glutGetModifiers(); // Get the state of modifier keys
		if (mods == GLUT_ACTIVE_SHIFT) { // If the Shift key is also pressed
			move_camera_vertical_20191559(&current_camera, -100.0f);  // Move the camera down
			fprintf(stdout, "move \n");
		}
		else {
			move_camera_20191559(&current_camera, 0.0f, -200.0f, 0.0f); // Move camera backward
			fprintf(stdout, "move backward\n");
		}
		glutPostRedisplay();
		break;

	case GLUT_KEY_RIGHT:
		move_camera_20191559(&current_camera, 50.0f, 0.0f, 0.0f);  // move right
		glutPostRedisplay();
		fprintf(stdout, "move right\n");
		break;
	}
}

int prevx, prevy;
void mouse_20191559(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			if(current_camera_index == CAMERA_1)
				isPhong = false;

			current_camera.rotation_axis = U;
			prevx = x; prevy = y;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			if (current_camera_index == CAMERA_1)
				isPhong = false;

			current_camera.rotation_axis = N;
			prevx = x; prevy = y;
		}
	}
}

void mousewheel_20191559(int wheel, int direction, int x, int y) {
	int key = glutGetModifiers();
	if (key == GLUT_ACTIVE_CTRL) {
		if (direction > 0) {
			camera_info[current_camera_index].fovy += 0.1;
			// current_camera.fovy += 0.1;
			fprintf(stdout, "fovy: %lf\n", camera_info[current_camera_index].fovy);
			ProjectionMatrix = glm::perspective(camera_info[current_camera_index].fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
			// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			// glutPostRedisplay();
			fprintf(stdout, "CTRL + Zoom In\n");
		}
		else if (direction < 0) {
			camera_info[current_camera_index].fovy -= 0.1;
			// current_camera.fovy -= 0.1;
			fprintf(stdout, "fovy: %lf\n", camera_info[current_camera_index].fovy);
			ProjectionMatrix = glm::perspective(camera_info[current_camera_index].fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
			// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			// glutPostRedisplay();
			fprintf(stdout, "CTRL + Zoom Out\n");

		}
	}
	glutPostRedisplay();
}

void motion_20191559(int x, int y) {
	if (current_camera_index == CAMERA_1) {
		if (current_camera.rotation_axis == N) {
			move_camera_rotate_20191559(y - prevy, N);
		}
		else {
			move_camera_rotate_20191559(y - prevy, U);
			move_camera_rotate_20191559(x - prevx, V);
		}
	}
	prevx = x; prevy = y;

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}
//////////

void reshape(int width, int height) {
	float aspect_ratio;

	glViewport(0, 0, width, height);

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, sun_temple_VAO);
	glDeleteBuffers(scene.n_materials, sun_temple_VBO);

	glDeleteVertexArrays(1, &skybox_VAO);
	glDeleteBuffers(1, &skybox_VBO);

	glDeleteTextures(scene.n_textures, sun_temple_texture_names);

	free(sun_temple_n_triangles);
	free(sun_temple_vertex_offset);

	free(sun_temple_VAO);
	free(sun_temple_VBO);

	free(sun_temple_texture_names);
	free(flag_texture_mapping);
}


void timer_scene_20191559(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	cur_frame_tiger = (timestamp_scene / 4) % N_TIGER_FRAMES;
	cur_frame_spider = (timestamp_scene / 4) % N_SPIDER_FRAMES;
	rotation_angle_cube = (timestamp_scene / 12) % N_SPIDER_FRAMES;

	// Update tiger position
	float angle = tiger_speed * timestamp_scene;
	float wiggle_frequency = 5.0f;  // Adjust as needed
	float wiggle_amplitude = 20.0f;  // Adjust as needed
	float wiggle_offset = sin(angle * wiggle_frequency) * wiggle_amplitude;

	tiger_position.x = tiger_radius * cos(angle);
	tiger_position.y = tiger_radius * sin(angle) + wiggle_offset; // Added sine wave for "wiggly" effect

	// Calculate direction of the tiger
	tiger_direction.x = -tiger_radius * sin(angle) - wiggle_offset * wiggle_frequency * cos(angle * wiggle_frequency);
	tiger_direction.y = tiger_radius * cos(angle) + wiggle_offset * wiggle_frequency * cos(angle * wiggle_frequency); // Derivative of the position function
	tiger_direction = glm::normalize(tiger_direction); // Normalize to get unit vector

	// Adjust direction to be tangent to the circle and negate
	tiger_direction = glm::vec3(-tiger_direction.y, tiger_direction.x, 0.0f); // Perpendicular to the radius and negate



	fprintf(stdout, "current frame: %d\n", cur_frame_tiger);
	glutPostRedisplay();
	glutTimerFunc(50, timer_scene_20191559, 0);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	glutSpecialFunc(special_20191559);
	glutMouseFunc(mouse_20191559);
	glutMouseWheelFunc(mousewheel_20191559);
	glutMotionFunc(motion_20191559);

	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene_20191559, 0);

	glutCloseFunc(cleanup);
}

void initialize_flags(void) {
	flag_fog = 0;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag_fog);
	glUseProgram(0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
	initialize_flags();
}

void prepare_scene(void) {
	prepare_axes();
	prepare_grid();
	prepare_sun_temple();
	prepare_skybox();

	prepare_tiger();
	prepare_ironman();
	prepare_spider();

	prepare_cube();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 9
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Sun Temple Scene";
	char messages[N_MESSAGE_LINES][256] = { 
		"    - Keys used:",
		"		'f' : draw x, y, z axes and grid",
		"		'1' : set the camera for bronze statue view",
		"		'2' : set the camera for bronze statue view",
		"		'3' : set the camera for tree view",	
		"		'4' : set the camera for top view",
		"		'5' : set the camera for front view",
		"		'6' : set the camera for side view",
		"		'ESC' : program close",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
