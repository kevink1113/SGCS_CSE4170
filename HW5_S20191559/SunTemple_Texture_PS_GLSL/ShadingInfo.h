﻿//
//  ShadingInfo.h
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

// light and material definitions

typedef struct _Light_Parameters {
	int light_on;
	float position[4];
	float ambient_color[4], diffuse_color[4], specular_color[4];
	float spot_direction[3];
	float spot_exponent;
	float spot_cutoff_angle;
	float light_attenuation_factors[4]; // produce this effect only if .w != 0.0f
} Light_Parameters;

typedef struct _loc_LIGHT_Parameters {
	GLint light_on;
	GLint position;
	GLint ambient_color, diffuse_color, specular_color;
	GLint spot_direction;
	GLint spot_exponent;
	GLint spot_cutoff_angle;
	GLint light_attenuation_factors;
} loc_light_Parameters;

typedef struct _Material_Parameters {
	float ambient_color[4], diffuse_color[4], specular_color[4], emissive_color[4];
	float specular_exponent;

	unsigned int  diffuseTex, normalTex, emissiveTex;
} Material_Parameters;

typedef struct _loc_Material_Parameters {
	GLint ambient_color, diffuse_color, specular_color, emissive_color;
	GLint specular_exponent;

	GLuint diffuseTex, normalTex, emissiveTex;
} loc_Material_Parameters;