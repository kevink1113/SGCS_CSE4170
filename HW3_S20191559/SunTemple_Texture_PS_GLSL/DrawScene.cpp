//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2023 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for PBR
GLuint h_ShaderProgram_TXPBR;
#define NUMBER_OF_LIGHT_SUPPORTED 13
GLint loc_global_ambient_color;
GLint loc_lightCount;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPBR, loc_ModelViewMatrix_TXPBR, loc_ModelViewMatrixInvTrans_TXPBR;
GLint loc_cameraPos;

#define TEXTURE_INDEX_DIFFUSE	(0)
#define TEXTURE_INDEX_NORMAL	(1)
#define TEXTURE_INDEX_SPECULAR	(2)
#define TEXTURE_INDEX_EMISSIVE	(3)
#define TEXTURE_INDEX_SKYMAP	(4)

// for skybox shaders
GLuint h_ShaderProgram_skybox;
GLint loc_cubemap_skybox;
GLint loc_ModelViewProjectionMatrix_SKY;

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.

// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.

glm::mat4 ModelMatrix_tiger;

glm::mat4 ModelViewMatrix, ModelViewMatrix_tiger, ModelViewMatrix_tiger_eye;
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

///////////////////////////////////////////// MODIFIED ///////////////////////////////////////////
GLuint h_ShaderProgram_TXPS; // handles to shader programs
int flag_texture_mapping;

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_texture, loc_flag_texture_mapping, loc_flag_fog;

// for tiger animation
unsigned int timestamp_scene = 0; // the global clock in the scene
int flag_tiger_animation, flag_polygon_fill;
int cur_frame_tiger = 0, cur_frame_ben = 0, cur_frame_wolf, cur_frame_spider = 0;
float rotation_angle_tiger = 0.0f;

// tiger object
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];

Material_Parameters material_tiger;


int read_geometry(GLfloat** object, int bytes_per_primitive, char* filename) {
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

void prepare_tiger(void) { // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/tiger/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
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

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


// Create a list of points for the tiger to move between
std::vector<glm::vec3> tiger_positions = {
	{0.0f, 0.0f, 80.0f},
	{500, 500, 200},
	{-500, -500, 80},
	{500, 500, 80},
	{-500, -500, 80},
	{500, 500, 200},
	{-500, -500, 80},
	{500, 500, 200},
	{-500, -500, 80},
	{500, 500, 200},
	{-500, -500, 80},
	{500, 500, 200},
	{-500, -500, 80},
	{500, 500, 200},
	{-500, -500, 80},
	{500, 500, 200},
	{-500, -500, 80}
	// Add additional points here
};

int num_positions = sizeof(tiger_positions) / sizeof(glm::vec3);
int current_position_index = 0;
float t = 0.0f;
float speed = 0.01f;

glm::vec3 lerp(glm::vec3 start, glm::vec3 end, float t) {
	return (1 - t) * start + t * end;
}


glm::vec3 tiger_position(0.0f, 0.0f, 80.0f); // Tiger's position
float tiger_speed = 0.01f; // Speed of the tiger
float tiger_radius = 200.0f; // Radius of the circle
float tiger_oscillation = 30.0f; // Oscillation amplitude for the "wiggly" effect
glm::vec3 tiger_direction(0.0f, 0.0f, 0.0f); // Direction of the tiger

void update_tiger_eye_camera_20191559(void);
void update_tiger_back_camera_20191559(void);


// Modify tiger's position for jumping
static const float jump_start_time = 0.0f;
static const float jump_interval = 60.0f * glm::pi<float>();
static const float jump_duration = 20.0f; // Jump duration in seconds
static const float max_jump_height = 200.0f;

void draw_tiger_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this line
	glUseProgram(h_ShaderProgram_simple);
	// Create a separate model matrix for the tiger
	
	ModelMatrix_tiger = glm::mat4(1.0f); // start with identity matrix

	ModelMatrix_tiger = glm::scale(ModelMatrix_tiger, glm::vec3(3.0f, 3.0f, 3.0f));
	ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, tiger_position); // Use tiger position

	float time_since_last_jump = fmod(timestamp_scene - jump_start_time, jump_interval);
	if (time_since_last_jump < jump_duration) {
		// We are in a jump, calculate height
		float progress_through_jump = time_since_last_jump / jump_duration;
		float height = max_jump_height * (4 * progress_through_jump * (1 - progress_through_jump));
		// tiger_position.z += height;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(0, 0, height));
	}

	// Rotate tiger to face the direction it is heading
	float angle = atan2(tiger_direction.y, tiger_direction.x);
	ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis

	// Then create the ModelViewMatrix by multiplying the view matrix with the tiger's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_tiger;

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(tiger_VAO);

	GLfloat tiger_color[3] = { 255.0f / 255.0f, 114.0f / 255.0f, 0.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, tiger_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	///////
	glm::vec4 tiger_eye_model(0.0f, -88.0f, 62.0f, 1.0f);
	glm::vec4 tiger_eye_world = ModelMatrix_tiger * tiger_eye_model;

	glm::vec4 tiger_direction_model(0.0f, 0.0f, -1.0f, 0.0f);
	glm::vec4 tiger_direction_world = ModelMatrix_tiger * tiger_direction_model;

	// Normalize the direction
	tiger_direction_world = glm::normalize(tiger_direction_world);

	// Create right vector (uaxis) by cross product of up vector (Y axis) and direction
	glm::vec3 tiger_right_world = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(tiger_direction_world));

	// Calculate up vector (vaxis) by cross product of direction and right
	glm::vec3 tiger_up_world = glm::cross(glm::vec3(tiger_direction_world), tiger_right_world);

	update_tiger_eye_camera_20191559();
	update_tiger_back_camera_20191559();
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
		spider_n_triangles[i] = read_geometry(&spider_vertices[i], n_bytes_per_triangle, filename);
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

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

glm::vec3 spider_position(0.0f, 0.0f, 1.0f); // spider's position


glm::mat4 ModelMatrix_spider;
void draw_spider_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this line
	glUseProgram(h_ShaderProgram_simple);

	glm::vec3 center = glm::vec3(0.0f, 0.0f, 1700.0f);
	float radius = 750.0f;
	float theta = ((sin((timestamp_scene+80) / 30.0f) + 1) * glm::pi<float>());  // Range [0, 2pi]
	float phi = ((cos((timestamp_scene+80) / 30.0f) + 1) * glm::pi<float>()) / 4.0f;  // Range [0, pi/2]

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

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(spider_VAO);

	GLfloat spider_color[3] = { 0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, spider_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// ben object
#define N_BEN_FRAMES 30
GLuint ben_VBO, ben_VAO;
int ben_n_triangles[N_BEN_FRAMES];
int ben_vertex_offset[N_BEN_FRAMES];
GLfloat* ben_vertices[N_BEN_FRAMES];

Material_Parameters material_ben;
void prepare_ben(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ben_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_BEN_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/ben/ben_vn%d%d.geom", i / 10, i % 10);
		ben_n_triangles[i] = read_geometry(&ben_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		ben_n_total_triangles += ben_n_triangles[i];

		if (i == 0)
			ben_vertex_offset[i] = 0;
		else
			ben_vertex_offset[i] = ben_vertex_offset[i - 1] + 3 * ben_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &ben_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glBufferData(GL_ARRAY_BUFFER, ben_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_BEN_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, ben_vertex_offset[i] * n_bytes_per_vertex,
			ben_n_triangles[i] * n_bytes_per_triangle, ben_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_BEN_FRAMES; i++)
		free(ben_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &ben_VAO);
	glBindVertexArray(ben_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


std::vector<glm::vec3> ben_path = {
	{600, -1884, 140},
	{600, -2282, 140},
	
	{600, -2650, 0},
	{600, -3500, 0},
	
	{-600, -3500, 0},
	{-600, -2650, 0},

	{-600, -2282, 140},
	{-600, -1884, 140}
	
	// more positions go here...
};


glm::vec3 ben_position(0.0f, 0.0f, 300.0f); // spider's position

glm::mat4 ModelMatrix_ben;
void draw_ben_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this liness
	glUseProgram(h_ShaderProgram_simple);

	/////
	// Calculate which segment of the path we're on based on timestamp_scene
	float speed_factor = 0.02f;  // Adjust this value to change the speed

	// Calculate which segment of the path we're on based on timestamp_scene
	int segment = int(timestamp_scene * speed_factor) % ben_path.size();
	// Calculate how far along that segment we are
	float segment_progress = fmod(timestamp_scene * speed_factor, 1.0f);

	// Calculate Ben's current position by interpolating along the current path segment
	glm::vec3 ben_position = glm::mix(ben_path[segment], ben_path[(segment + 1) % ben_path.size()], segment_progress);

	// Calculate the direction Ben is facing
	glm::vec3 ben_direction = glm::normalize(ben_path[(segment + 1) % ben_path.size()] - ben_path[segment]);

	// fprintf(stdout, "[segment: %d]\n", segment);
	if (segment == 0 || segment == 1 || segment == 2) {
		ModelMatrix_ben = glm::translate(glm::mat4(1.0f), ben_position);
		ModelMatrix_ben = glm::scale(ModelMatrix_ben, glm::vec3(1000.0f, 1000.0f, 1000.0f));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, 0 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	} else if (segment == 3) {
		ModelMatrix_ben = glm::translate(glm::mat4(1.0f), ben_position);
		ModelMatrix_ben = glm::scale(ModelMatrix_ben, glm::vec3(1000.0f, 1000.0f, 1000.0f));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, 270 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	} else if (segment == 4 || segment == 5 || segment == 6) {
		ModelMatrix_ben = glm::translate(glm::mat4(1.0f), ben_position);
		ModelMatrix_ben = glm::scale(ModelMatrix_ben, glm::vec3(1000.0f, 1000.0f, 1000.0f));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, 180 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	} else if (segment == 7) {
		ModelMatrix_ben = glm::translate(glm::mat4(1.0f), ben_position);
		ModelMatrix_ben = glm::scale(ModelMatrix_ben, glm::vec3(1000.0f, 1000.0f, 1000.0f));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, 450 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelMatrix_ben = glm::rotate(ModelMatrix_ben, -90 * TO_RADIAN, glm::vec3(1, 0, 0));
	}

	float time_since_last_jump = fmod(timestamp_scene+40 - jump_start_time, jump_interval);
	if (time_since_last_jump < jump_duration) {
		// We are in a jump, calculate height
		float progress_through_jump = time_since_last_jump / jump_duration;
		float height = 0.1 * (4 * progress_through_jump * (1 - progress_through_jump));
		// tiger_position.z += height;
		ModelMatrix_ben = glm::translate(ModelMatrix_ben, glm::vec3(0, -height, 0));
	}

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_ben;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(ben_VAO);

	GLfloat ben_color[3] = { 255.0f / 255.0f, 155.0f / 255.0f, 0.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, ben_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, ben_vertex_offset[cur_frame_spider], 3 * ben_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// ironman object
glm::mat4 ModelMatrix_ironman;
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;

Material_Parameters material_ironman;

// tank object
glm::mat4 ModelMatrix_tank;
GLuint tank_VBO, tank_VAO;
int tank_n_triangles;
GLfloat* tank_vertices;

Material_Parameters material_tank;

void prepare_ironman(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

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


	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
glm::vec3 ironman_position(-432.0f, 1417.0f, 1377.0f); // ironman's position
void draw_ironman_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this line
	glUseProgram(h_ShaderProgram_simple);

	glFrontFace(GL_CW);

	ModelMatrix_ironman = glm::translate(glm::mat4(1.0f), ironman_position); // Use spider position
	ModelMatrix_ironman = glm::scale(ModelMatrix_ironman, glm::vec3(200.0f, 200.0f, 200.0f));
	// ModelMatrix_ironman = glm::rotate(ModelMatrix_ironman, 240 * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix_ironman = glm::rotate(ModelMatrix_ironman, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_ironman;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(ironman_VAO);

	GLfloat ironman_color[3] = { 224.0f / 255.0f, 70.0f / 255.0f, 82.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, ironman_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void prepare_tank(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tank_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/tank_vnt.geom");
	tank_n_triangles = read_geometry(&tank_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	tank_n_total_triangles += tank_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &tank_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glBufferData(GL_ARRAY_BUFFER, tank_n_total_triangles * 3 * n_bytes_per_vertex, tank_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(tank_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &tank_VAO);
	glBindVertexArray(tank_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

glm::vec3 tank_position(-3000.0f, -1000.0f, 0.0f); // godzilla's position
void draw_tank_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this line
	glUseProgram(h_ShaderProgram_simple);

	glFrontFace(GL_CW);

	ModelMatrix_tank = glm::translate(glm::mat4(1.0f), tank_position); // Use spider position
	ModelMatrix_tank = glm::scale(ModelMatrix_tank, glm::vec3(500.0f, 500.0f, 500.0f));
	ModelMatrix_tank = glm::rotate(ModelMatrix_tank, 240 * TO_RADIAN, glm::vec3(0, 0, 1));
	// ModelMatrix_tank = glm::rotate(ModelMatrix_tank, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_tank;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(tank_VAO);

	GLfloat tank_color[3] = { 21.0f / 255.0f, 97.0f / 255.0f, 44.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, tank_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 3 * tank_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



glm::mat4 ModelMatrix_godzilla;
// godzilla object
GLuint godzilla_VBO, godzilla_VAO;
int godzilla_n_triangles;
GLfloat* godzilla_vertices;

Material_Parameters material_godzilla;

void prepare_godzilla(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, godzilla_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/godzilla_vnt.geom");
	godzilla_n_triangles = read_geometry(&godzilla_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	godzilla_n_total_triangles += godzilla_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &godzilla_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glBufferData(GL_ARRAY_BUFFER, godzilla_n_total_triangles * 3 * n_bytes_per_vertex, godzilla_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(godzilla_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &godzilla_VAO);
	glBindVertexArray(godzilla_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

glm::vec3 godzilla_position(8000.0f, -8000.0f, -5000.0f); // godzilla's position
void draw_godzilla_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this line
	glUseProgram(h_ShaderProgram_simple);

	glFrontFace(GL_CW);

	ModelMatrix_godzilla = glm::translate(glm::mat4(1.0f), godzilla_position); // Use spider position
	ModelMatrix_godzilla = glm::scale(ModelMatrix_godzilla, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelMatrix_godzilla = glm::rotate(ModelMatrix_godzilla, 240 * TO_RADIAN, glm::vec3(0, 0, 1));
	ModelMatrix_godzilla = glm::rotate(ModelMatrix_godzilla, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_godzilla;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(godzilla_VAO);

	GLfloat godzilla_color[3] = { 255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, godzilla_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 3 * godzilla_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

glm::mat4 ModelMatrix_dragon;

// dragon object
GLuint dragon_VBO, dragon_VAO;
int dragon_n_triangles;
GLfloat* dragon_vertices;

Material_Parameters material_dragon;


glm::mat4 ModelMatrix_optimus;
// optimus object
GLuint optimus_VBO, optimus_VAO;
int optimus_n_triangles;
GLfloat* optimus_vertices;

Material_Parameters material_optimus;

void prepare_dragon(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, dragon_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/dragon_vnt.geom");
	dragon_n_triangles = read_geometry(&dragon_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	dragon_n_total_triangles += dragon_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &dragon_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glBufferData(GL_ARRAY_BUFFER, dragon_n_total_triangles * 3 * n_bytes_per_vertex, dragon_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(dragon_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &dragon_VAO);
	glBindVertexArray(dragon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

glm::vec3 dragon_position(0.0f, -2000.0f, 2400.0f); // godzilla's position
void draw_dragon_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this line
	glUseProgram(h_ShaderProgram_simple);

	glFrontFace(GL_CW);

	ModelMatrix_dragon = glm::translate(glm::mat4(1.0f), dragon_position); // Use spider position
	ModelMatrix_dragon = glm::scale(ModelMatrix_dragon, glm::vec3(100.0f, 100.0f, 100.0f));
	// ModelMatrix_dragon = glm::rotate(ModelMatrix_dragon, 240 * TO_RADIAN, glm::vec3(0, 0, 1));
	// ModelMatrix_dragon = glm::rotate(ModelMatrix_dragon, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider's model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_dragon;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(dragon_VAO);

	GLfloat dragon_color[3] = { 255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, dragon_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void prepare_optimus(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, optimus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/optimus_vnt.geom");
	optimus_n_triangles = read_geometry(&optimus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	optimus_n_total_triangles += optimus_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &optimus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glBufferData(GL_ARRAY_BUFFER, optimus_n_total_triangles * 3 * n_bytes_per_vertex, optimus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(optimus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &optimus_VAO);
	glBindVertexArray(optimus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

glm::vec3 optimus_position(0.0f, -2000.0f, 2000.0f); // godzilla's position
void draw_optimus_20191559(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // added this line
	glUseProgram(h_ShaderProgram_simple);

	glFrontFace(GL_CW);

	ModelMatrix_optimus = glm::translate(glm::mat4(1.0f), optimus_position); // Use spider position
	ModelMatrix_optimus = glm::scale(ModelMatrix_optimus, glm::vec3(3.0f, 3.0f, 3.0f));
	// ModelMatrix_dragon = glm::rotate(ModelMatrix_optimus, 165 * TO_RADIAN, glm::vec3(0, 0, 1));
	// ModelMatrix_dragon = glm::rotate(ModelMatrix_dragon, 90 * TO_RADIAN, glm::vec3(1, 0, 0));

	// Then create the ModelViewMatrix by multiplying the view matrix with the spider`'s model matrix
	ModelViewMatrix = ViewMatrix * ModelMatrix_optimus;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glFrontFace(GL_CW);

	glBindVertexArray(optimus_VAO);

	GLfloat optimus_color[3] = { 30.0f / 255.0f, 0.0f / 255.0f, 200.0f / 255.0f };
	glUniform3fv(loc_primitive_color, 1, optimus_color);
	glLineWidth(1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 3 * optimus_n_triangles);
	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


const float radius = 40.0f; // Radius of the circle
glm::vec3 previous_tiger_position = tiger_position; // Store the previous position to calculate direction

void timer_scene_20191559(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	cur_frame_tiger = (timestamp_scene / 4) % N_TIGER_FRAMES;
	cur_frame_spider = (timestamp_scene / 4) % N_TIGER_FRAMES;

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

	glutPostRedisplay();
	if (flag_tiger_animation)
		glutTimerFunc(50, timer_scene_20191559, 0);
}



void initialize_flags(void) {
	flag_tiger_animation = 1;
	flag_polygon_fill = 1;
	flag_texture_mapping = 1;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_texture_mapping, flag_texture_mapping);
	glUseProgram(0);
}


///////////////////////////////////////////// MODIFIED ///////////////////////////////////////////


/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_u,
	CAMERA_i,
	CAMERA_o,
	CAMERA_p,
	CAMERA_a,
	CAMERA_6,
	CAMERA_t,
	CAMERA_g,
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


int current_camera_index = CAMERA_u;

void set_current_camera(int camera_num) {
	Camera* pCamera = &camera_info[camera_num];
	current_camera_index = camera_num;
	camera_info[current_camera_index].fovy = 0.7f;
	fprintf(stdout, "fovy and others: %lf, %lf, %lf, %lf\n", pCamera->fovy, pCamera->aspect_ratio, pCamera->near_c, pCamera->far_c);
	pCamera->fovy = 0.7f;
	if (current_camera_index == CAMERA_i) pCamera->fovy = 1.3f;
	else if (current_camera_index == CAMERA_p) pCamera->fovy = 1.6f;
	pCamera->aspect_ratio = 1.777777f;
	pCamera->near_c = 0.1f;
	pCamera->far_c = 50000.0f;
	ProjectionMatrix = glm::perspective(camera_info[current_camera_index].fovy, camera_info[current_camera_index].aspect_ratio, camera_info[current_camera_index].near_c, camera_info[current_camera_index].far_c);
	// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	// glutPostRedisplay();


	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) {


	//CAMERA_u
	Camera* pCamera = &camera_info[CAMERA_u];
	pCamera = &camera_info[CAMERA_u];
	pCamera->pos[0] = 1145.483887f; pCamera->pos[1] = 5158.519531f; pCamera->pos[2] = 2150.309082f;
	pCamera->uaxis[0] = -0.997670f; pCamera->uaxis[1] = -0.067447f; pCamera->uaxis[2] = -0.009732f;
	pCamera->vaxis[0] = -0.002554f; pCamera->vaxis[1] = -0.138464f; pCamera->vaxis[2] = 0.990351f;
	pCamera->naxis[0] = -0.067962f; pCamera->naxis[1] = 0.995259f; pCamera->naxis[2] = 0.069325f;
	pCamera->move = 0;
	pCamera->fovy = 0.7f;
	pCamera->aspect_ratio = 1.777777f;
	pCamera->near_c = 0.1f;
	pCamera->far_c = 50000.0f;
	// pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_i
	pCamera = &camera_info[CAMERA_i];
	pCamera->pos[0] = -257.090210f; pCamera->pos[1] = -3655.922363f; pCamera->pos[2] = 692.310791f;
	pCamera->uaxis[0] = -0.932467f; pCamera->uaxis[1] = 0.361274f; pCamera->uaxis[2] = -0.003095f;
	pCamera->vaxis[0] = 0.063281f; pCamera->vaxis[1] = 0.171757f; pCamera->vaxis[2] = 0.983115f;
	pCamera->naxis[0] = -0.355701f; pCamera->naxis[1] = -0.916518f; pCamera->naxis[2] = 0.183017f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_o
	pCamera = &camera_info[CAMERA_o];
	pCamera->pos[0] = 4048.696289f; pCamera->pos[1] = 5108.203613f; pCamera->pos[2] = 4309.340820f;
	pCamera->uaxis[0] = 0.797027f; pCamera->uaxis[1] = -0.603921f; pCamera->uaxis[2] = 0.006863f;
	pCamera->vaxis[0] = -0.261259f; pCamera->vaxis[1] = -0.334509f; pCamera->vaxis[2] = 0.905464f;
	pCamera->naxis[0] = 0.544528f; pCamera->naxis[1] = 0.723466f; pCamera->naxis[2] = 0.424388f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_p
	pCamera = &camera_info[CAMERA_p];
	pCamera->pos[0] = 404.235779f; pCamera->pos[1] = 761.755493f; pCamera->pos[2] = 394.795715f;
	pCamera->uaxis[0] = -0.965963f; pCamera->uaxis[1] = 0.256164f; pCamera->uaxis[2] = -0.036229f;
	pCamera->vaxis[0] = 0.072558f; pCamera->vaxis[1] = 0.410909f; pCamera->vaxis[2] = 0.908790f;
	pCamera->naxis[0] = 0.241936f; pCamera->naxis[1] = 0.844827f; pCamera->naxis[2] = -0.477208f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_a : moving mode
	pCamera = &camera_info[CAMERA_a];
	pCamera->pos[0] = 12053.893555f; pCamera->pos[1] = 1300.578735f; pCamera->pos[2] = 2211.345459f;
	pCamera->uaxis[0] = -0.487198f; pCamera->uaxis[1] = 0.873297f; pCamera->uaxis[2] = -0.000486f;
	pCamera->vaxis[0] = -0.004076f; pCamera->vaxis[1] = 0.000856f; pCamera->vaxis[2] = 0.999992f;
	pCamera->naxis[0] = 0.871406f; pCamera->naxis[1] = 0.486108f; pCamera->naxis[2] = -0.065845f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;
	
	set_current_camera(CAMERA_u);
}

void move_camera_20191559(Camera* camera, float dx, float dy, float dz) {
	if (current_camera_index == CAMERA_a) {
		camera->pos[0] += dx * camera->uaxis[0] - dy * camera->naxis[0] + dz * camera->vaxis[0];
		camera->pos[1] += dx * camera->uaxis[1] - dy * camera->naxis[1] + dz * camera->vaxis[1];
		camera->pos[2] += dx * camera->uaxis[2] - dy * camera->naxis[2] + dz * camera->vaxis[2];


		fprintf(stdout, "Camera moved to (%lf, %lf, %lf)\n", camera->pos[0], camera->pos[1], camera->pos[2]);
		// Update the view matrix
		set_ViewMatrix_from_camera_frame();
	}
}

void move_camera_vertical_20191559(Camera* camera, float dy) {
	if (current_camera_index == CAMERA_a) {
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

void update_tiger_eye_camera_20191559() {
	// Compute the world coordinates of the tiger's eye
	glm::vec4 tiger_eye_model(0.0f, -88.0f, 62.0f, 1.0f);
	glm::vec4 tiger_eye_world = ModelMatrix_tiger * tiger_eye_model;

	// Add a small nodding motion to the camera's y position
	float nod_speed = 0.1f;  // Adjust this value to control the speed of the nodding motion
	float nod_amplitude = 30.0f;  // Adjust this value to control the height of the nodding motion
	tiger_eye_world.z += nod_amplitude * sin(timestamp_scene * nod_speed);

	// Compute the world coordinates of the tiger's direction
	glm::vec4 tiger_direction_model(0.0f, -1.0f, 0.0f, 0.0f);
	glm::vec4 tiger_direction_world = ModelMatrix_tiger * tiger_direction_model;

	// Normalize the direction
	tiger_direction_world = glm::normalize(tiger_direction_world);

	// Create right vector (uaxis) by cross product of world's up vector (Z axis) and direction
	glm::vec3 tiger_right_world = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(tiger_direction_world));

	// Calculate up vector (vaxis) by cross product of direction and right
	glm::vec3 tiger_up_world = glm::cross(glm::vec3(tiger_direction_world), tiger_right_world);

	// Update the CAMERA_7 parameters in the camera_info array
	Camera* pCamera = &camera_info[CAMERA_t];

	pCamera->pos[0] = tiger_eye_world.x; pCamera->pos[1] = tiger_eye_world.y; pCamera->pos[2] = tiger_eye_world.z;
	pCamera->uaxis[0] = tiger_right_world.x; pCamera->uaxis[1] = tiger_right_world.y; pCamera->uaxis[2] = tiger_right_world.z;
	pCamera->vaxis[0] = tiger_up_world.x; pCamera->vaxis[1] = tiger_up_world.y; pCamera->vaxis[2] = tiger_up_world.z;
	pCamera->naxis[0] = -tiger_direction_world.x; pCamera->naxis[1] = -tiger_direction_world.y; pCamera->naxis[2] = -tiger_direction_world.z;

	pCamera->fovy = 0.7f;

	if (current_camera_index == CAMERA_t)
		current_camera = *pCamera;

	set_ViewMatrix_from_camera_frame();
}

void update_tiger_back_camera_20191559() {
	// Compute the world coordinates of the tiger's eye
	glm::vec4 tiger_eye_model(0.0f, 150.0f, 50.0f, 1.0f);
	glm::vec4 tiger_eye_world = ModelMatrix_tiger * tiger_eye_model;

	// Compute the world coordinates of the tiger's direction
	glm::vec4 tiger_direction_model(0.0f, -1.0f, 0.0f, 0.0f);
	glm::vec4 tiger_direction_world = ModelMatrix_tiger * tiger_direction_model;

	// Normalize the direction
	tiger_direction_world = glm::normalize(tiger_direction_world);

	// Create right vector (uaxis) by cross product of world's up vector (Z axis) and direction
	glm::vec3 tiger_right_world = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(tiger_direction_world));

	// Calculate up vector (vaxis) by cross product of direction and right
	glm::vec3 tiger_up_world = glm::cross(glm::vec3(tiger_direction_world), tiger_right_world);

	// Update the CAMERA_7 parameters in the camera_info array
	Camera* pCamera = &camera_info[CAMERA_g];

	pCamera->pos[0] = tiger_eye_world.x; pCamera->pos[1] = tiger_eye_world.y; pCamera->pos[2] = tiger_eye_world.z;
	pCamera->uaxis[0] = tiger_right_world.x; pCamera->uaxis[1] = tiger_right_world.y; pCamera->uaxis[2] = tiger_right_world.z;
	pCamera->vaxis[0] = tiger_up_world.x; pCamera->vaxis[1] = tiger_up_world.y; pCamera->vaxis[2] = tiger_up_world.z;
	pCamera->naxis[0] = -tiger_direction_world.x; pCamera->naxis[1] = -tiger_direction_world.y; pCamera->naxis[2] = -tiger_direction_world.z;

	pCamera->fovy = 0.7f;

	if (current_camera_index == CAMERA_g)
		current_camera = *pCamera;

	set_ViewMatrix_from_camera_frame();
}

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

	ShaderInfo shader_info_TXPBR[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Background/PBR_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Background/PBR_Tx.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_TXPBR = LoadShaders(shader_info_TXPBR);
	glUseProgram(h_ShaderProgram_TXPBR);

	loc_ModelViewProjectionMatrix_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewMatrixInvTrans");

	//loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_global_ambient_color");

	loc_lightCount = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_light_count");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPBR, string);
		sprintf(string, "u_light[%d].color", i);
		loc_light[i].color = glGetUniformLocation(h_ShaderProgram_TXPBR, string);
	}

	loc_cameraPos = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_camPos");

	//Textures
	loc_material.diffuseTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_albedoMap");
	loc_material.normalTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_normalMap");
	loc_material.specularTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_metallicRoughnessMap");
	loc_material.emissiveTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_emissiveMap");

	ShaderInfo shader_info_skybox[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Background/skybox.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Background/skybox.frag" },
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

// axes
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

// grid
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

void initialize_lights(void) { // follow OpenGL conventions for initialization
	glUseProgram(h_ShaderProgram_TXPBR);

	glUniform1i(loc_lightCount, scene.n_lights);

	for (int i = 0; i < scene.n_lights; i++) {
		glUniform4f(loc_light[i].position,
			scene.light_list[i].pos[0],
			scene.light_list[i].pos[1],
			scene.light_list[i].pos[2],
			0.0f);

		glUniform3f(loc_light[i].color,
			scene.light_list[i].color[0],
			scene.light_list[i].color[1],
			scene.light_list[i].color[2]);
	}

	glUseProgram(0);
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
	GLenum format, internalFormat;
	if (tx_bits_per_pixel == 32) {
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}
	else if (tx_bits_per_pixel == 24) {
		format = GL_BGR;
		internalFormat = GL_RGB;
	}
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap = FreeImage_ConvertTo32Bits(tx_pixmap);
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

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
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded sun temple textures into graphics memory.\n");

	free(sun_temple_vertices);
}

void bindTexture(GLint tex, int glTextureId, int texId) {
	if (INVALID_TEX_ID != texId) {
		glActiveTexture(GL_TEXTURE0 + glTextureId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);
		glUniform1i(tex, glTextureId);
	}
}

void draw_sun_temple(void) {
	glUseProgram(h_ShaderProgram_TXPBR);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPBR, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPBR, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPBR, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		int diffuseTexId = scene.material_list[materialIdx].diffuseTexId;
		int normalMapTexId = scene.material_list[materialIdx].normalMapTexId;
		int specularTexId = scene.material_list[materialIdx].specularTexId;;
		int emissiveTexId = scene.material_list[materialIdx].emissiveTexId;

		bindTexture(loc_material.diffuseTex, TEXTURE_INDEX_DIFFUSE, diffuseTexId);
		bindTexture(loc_material.normalTex, TEXTURE_INDEX_NORMAL, normalMapTexId);
		bindTexture(loc_material.specularTex, TEXTURE_INDEX_SPECULAR, specularTexId);
		bindTexture(loc_material.emissiveTex, TEXTURE_INDEX_EMISSIVE, emissiveTexId);

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
	// vertices enumerated clockwise
	// 6*2*3 * 2 (POS & NORM)

	// position
	-1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  1.0f, //right
	 1.0f,  1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f, -1.0f, //left
	 1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f,  1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //top
	 1.0f,  1.0f,  1.0f,    1.0f, -1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,   -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f, //bottom
	 1.0f, -1.0f, -1.0f,    1.0f,  1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f, -1.0f, -1.0f,   -1.0f,  1.0f, -1.0f, //back
	-1.0f,  1.0f, -1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //front
	 1.0f,  1.0f,  1.0f,    1.0f,  1.0f, -1.0f,    1.0f, -1.0f, -1.0f,

	 // normal
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,

	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,
	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,

	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,
	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,

	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,

	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,

	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f,
	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f
};

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
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20000, 20000, 20000));
	//ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(20000.0f, 20000.0f, 20000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
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
/*****************************  END: geometry setup *****************************/

/********************  START: callback function definitions *********************/
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_grid();
	draw_axes();
	draw_sun_temple();
	draw_skybox();
	draw_tiger_20191559();
	// update_tiger_eye_camera_20191559();
	draw_spider_20191559();
	
	draw_godzilla_20191559();
	draw_ben_20191559();
	draw_dragon_20191559();
	draw_optimus_20191559();
	draw_ironman_20191559();
	draw_tank_20191559();

	glutSwapBuffers();
}

void keyboard_20191559(unsigned char key, int x, int y) {
	switch (key) {
		// TODO
	case 'm': // toggle the animation effect.
		flag_tiger_animation = 1 - flag_tiger_animation;
		if (flag_tiger_animation) {
			glutTimerFunc(100, timer_scene_20191559, 0);
			fprintf(stdout, "^^^ Animation mode ON.\n");
		}
		else
			fprintf(stdout, "^^^ Animation mode OFF.\n");
		break;
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	case 'u':
		set_current_camera(CAMERA_u);
		glutPostRedisplay();
		break;
	case 'i':
		set_current_camera(CAMERA_i);
		glutPostRedisplay();
		break;
	case 'o':
		set_current_camera(CAMERA_o);
		glutPostRedisplay();
		break;
	case 'p':
		set_current_camera(CAMERA_p);
		glutPostRedisplay();
		break;
	case 'a':
		set_current_camera(CAMERA_a);
		glutPostRedisplay();
		break;
	case 't':
		fprintf(stdout, "Tiger's eye view\n");
		set_current_camera(CAMERA_t);
		glutPostRedisplay();
		break;
	case 'g':
		fprintf(stdout, "Tiger's back view\n");
		set_current_camera(CAMERA_g);
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

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
			current_camera.rotation_axis = U;
			prevx = x; prevy = y;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
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
	if (current_camera_index == CAMERA_a) {
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
	glDeleteTextures(scene.n_textures, sun_temple_texture_names);

	glDeleteVertexArrays(1, &skybox_VAO);
	glDeleteBuffers(1, &skybox_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);

	free(sun_temple_n_triangles);
	free(sun_temple_vertex_offset);

	free(sun_temple_VAO);
	free(sun_temple_VBO);

	free(sun_temple_texture_names);

	// free(tiger_n_triangles);
	// free(tiger_vertex_offset);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard_20191559);
	glutSpecialFunc(special_20191559);
	glutMouseFunc(mouse_20191559);
	glutMouseWheelFunc(mousewheel_20191559);
	glutMotionFunc(motion_20191559);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene_20191559, 0);
	glutCloseFunc(cleanup);
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

	prepare_spider();

	prepare_godzilla();
	prepare_ben();
	prepare_dragon();
	prepare_optimus();
	prepare_ironman();
	prepare_tank();
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

#define N_MESSAGE_LINES 10
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Sun Temple Scene";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used:",
		"		'm' : turn off/on tiger animation",
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
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
