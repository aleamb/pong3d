#include "renderer.h"
#include <stdarg.h>
#include <stdio.h>

#define ERRORMSG_MAX_LENGTH 64

GLuint vertex_shader;
GLuint fragment_shader;

GLuint program;

GLuint projectionMatrixId;
GLuint viewMatrixId;
GLuint modelMatrixId;

GLuint alphaUniform;

GLchar errormsg[ERRORMSG_MAX_LENGTH ];

int program_created = 0;
int vertex_shader_created = 0;
int fragment_shader_created = 0;

const GLchar* vertex_shader_source =
"#version 400 core\n \
		in vec4 in_position;\n \
		in vec4 in_color;\n \
		in vec4 in_normal;\n \
		in vec2 in_uv;\n \
		in vec4 extra;\n \
		uniform mat4 projectionMatrix;\n \
		uniform mat4 viewMatrix;\n \
		uniform mat4 modelMatrix;\n \
		out vec4 outColor;\n \
		out vec2 outUV;\n \
		out vec4 outNormal;\n \
		out vec4 outExtra;\n \
		void main(void) {\n \
			gl_Position = projectionMatrix * viewMatrix * modelMatrix * in_position;\n \
				outColor = in_color;\n \
				outUV = in_uv;\n \
				outNormal = in_normal;\n \
				outExtra = extra;\n \
		}";

const GLchar* fragment_shader_source =
"#version 400 core\n \
		in vec4 outColor;\n \
		in vec2 outUV;\n \
		out vec4 color;\n \
		uniform bool stageWireframe;\n \
		uniform sampler2D tex;\n \
		uniform bool renderText;\n \
		uniform float alpha;\n \
		void main(void) {\n \
			if (stageWireframe) {\n \
				color = outColor + vec4(0.0, 0.0, 0.0, 0.2);\n \
			}\n \
			else if (renderText) {\n \
				color = vec4(1.0, 1.0, 1.0, texture(tex, outUV).r);\n \
			}\n \
			else {\n \
				color = vec4(outColor.xyz, outColor.w - alpha);\n \
			}\n \
		}";

float projection_matrix[16];
float view_matrix[16];


void renderer_clear_screen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void upload_to_renderer(PONG_ELEMENT* element) {
	glGenVertexArrays(1, &element->vao);
	glBindVertexArray(element->vao);

	glGenBuffers(1, &element->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, element->vbo);
	glBufferData(GL_ARRAY_BUFFER, element->vertex_count * VERTEX_SIZE * sizeof(float), element->vertex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * VERTEX_SIZE, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * VERTEX_SIZE, (char*)NULL + sizeof(float) * 4);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * VERTEX_SIZE, (char*)NULL + sizeof(float) * 8);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(float) * VERTEX_SIZE, (char*)NULL + sizeof(float) * 12);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * VERTEX_SIZE, (char*)NULL + sizeof(float) * 14);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	if (element->elements_count > 0) {
		glGenBuffers(1, &element->ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, element->elements_count * sizeof(unsigned int), element->elements, GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
}

void remove_to_renderer(PONG_ELEMENT* element) {
	glDeleteBuffers(1, &element->vbo);
	if (element->elements_count > 0) {
		glDeleteBuffers(1, &element->ebo);
	}
	glDeleteVertexArrays(1, &element->vao);
}


GLuint build_shader(GLenum type, const GLchar* source, GLint* result, GLchar *errormsg) {
	GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &source, NULL);
	glCompileShader(id);
	int params;
	glGetShaderiv(id, GL_COMPILE_STATUS, &params);
	if (params == GL_FALSE) {
		*result = -1;
		if (errormsg != NULL) {
			int maxLength;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
			glGetShaderInfoLog(id,
					(maxLength > ERRORMSG_MAX_LENGTH ? ERRORMSG_MAX_LENGTH : maxLength), &maxLength, errormsg);
		}
		return -1;
	} else {
		*result = 0;
	}
	return id;
}


GLuint build_shaders_program(int count, int* result, GLchar* errormsg, ...) {
	va_list ap;
	GLuint program_id = glCreateProgram();
	va_start(ap, errormsg);
	for(int j = 0; j < count; j++) {
		GLuint shader = va_arg(ap, GLuint);
		glAttachShader(program_id, shader);
	}
	va_end(ap);
	glLinkProgram(program_id);
	GLint params;
	glGetProgramiv(program_id, GL_LINK_STATUS, &params);
	if (params == GL_FALSE) {
		*result = -1;
		if (errormsg != NULL) {
			int maxLength;
			glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);
			glGetProgramInfoLog(program_id,
					(maxLength > ERRORMSG_MAX_LENGTH ? ERRORMSG_MAX_LENGTH : maxLength), &maxLength, errormsg);
		}
	} else {
		*result = 0;
	}
	return program_id;
}


int init_renderer(int width, int height) {

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "OpenGL error: %s\n", glewGetErrorString(err));
		return -1;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int result;
	vertex_shader = build_shader(GL_VERTEX_SHADER, vertex_shader_source, &result, errormsg);
	if (result != 0) {
		fprintf( stderr, "Compile vertex shader failed: %s\n", errormsg);
		return -1;
	}
	vertex_shader_created = 1;

	fragment_shader = build_shader(GL_FRAGMENT_SHADER, fragment_shader_source, &result, errormsg);
	if (result != 0) {
		fprintf( stderr, "Compile fragment shader failed: %s\n", errormsg);
		return -1;
	}
	fragment_shader_created = 1;
	program = build_shaders_program(2, &result, errormsg, vertex_shader, fragment_shader);
	if (result != 0) {
		fprintf( stderr, "Links shaders program failed: %s\n", errormsg);
		return -1;
	}
	program_created = 1;

	load_identity_matrix(view_matrix);

	// compensation of Z axis due perspective: 0.5 / tan(fov / 2) / aspect
	view_matrix[14] = -0.866f / ((float)width / height);
	create_projection_matrix(60.0f, (float)width / height, 0.001f, 10.0f, projection_matrix);

	glUseProgram(program);
	projectionMatrixId = glGetUniformLocation(program, "projectionMatrix");
	viewMatrixId = glGetUniformLocation(program, "viewMatrix");
	modelMatrixId = glGetUniformLocation(program, "modelMatrix");
	glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, projection_matrix);
	glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, view_matrix);
	alphaUniform = glGetUniformLocation(program, "alpha");
	glUniform1f(alphaUniform, 0.0f);


	return 0;
}

void dispose_renderer() {
	if (program_created) {
		glDeleteProgram(program_created);
	}
	if (vertex_shader_created) {
		glDeleteShader(vertex_shader);
	}
	if (fragment_shader_created) {
		glDeleteShader(fragment_shader);
	}
}

void render_pong_element(PONG_ELEMENT* element) {
	glBindVertexArray(element->vao);
	glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, element->model_matrix);
	if (element->elements_count > 0) {
		glDrawElements(element->vertexType, element->elements_count, GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(element->vertexType, 0, element->vertex_count);
	}
	glBindVertexArray(0);
}

void render_shadows() {

	load_identity_matrix(ball_shadow.model_matrix);

	ball_shadow.model_matrix[12] = -stage.width / 2.0f + 0.0001;
	ball_shadow.model_matrix[13] = ball.model_matrix[13];
	ball_shadow.model_matrix[14] = ball.model_matrix[14] - 0.011;
	render_pong_element(&ball_shadow);

	ball_shadow.model_matrix[12] = stage.width / 2.0f - 0.0001;
	ball_shadow.model_matrix[13] = ball.model_matrix[13];
	ball_shadow.model_matrix[14] = ball.model_matrix[14] - 0.011;;
	render_pong_element(&ball_shadow);

	ball_shadow.model_matrix[0] = 0.0f;
	ball_shadow.model_matrix[1] = 1.0f;
	ball_shadow.model_matrix[4] = -1.0f;
	ball_shadow.model_matrix[5] = 0.0f;

	ball_shadow.model_matrix[12] = ball.model_matrix[12];
	ball_shadow.model_matrix[13] = -stage.height / 2.0f + 0.0001;
	ball_shadow.model_matrix[14] = ball.model_matrix[14]- 0.016;
	render_pong_element(&ball_shadow);

	ball_shadow.model_matrix[12] = ball.model_matrix[12];
	ball_shadow.model_matrix[13] = stage.height / 2.0f - 0.0001;
	ball_shadow.model_matrix[14] = ball.model_matrix[14] - 0.016;;
	render_pong_element(&ball_shadow);

	stick_shadow.model_matrix[0] = 1.0f;
	stick_shadow.model_matrix[1] = 0.0f;
	stick_shadow.model_matrix[4] = 0.0f;
	stick_shadow.model_matrix[5] = 1.0f;

	stick_shadow.model_matrix[12] = player_stick.model_matrix[12];
	stick_shadow.model_matrix[13] = -stage.height / 2.0f;
	render_pong_element(&stick_shadow);


	stick_shadow.model_matrix[12] = player_stick.model_matrix[12];
	stick_shadow.model_matrix[13] = stage.height / 2.0f;
	render_pong_element(&stick_shadow);


	stick_shadow.model_matrix[0] = 0.0f;
	stick_shadow.model_matrix[1] = 1.0f;
	stick_shadow.model_matrix[4] = -1.0f;
	stick_shadow.model_matrix[5] = 0.0f;
	stick_shadow.model_matrix[10] = 0.0f;

	stick_shadow.model_matrix[12] = -stage.width / 2.0f;
	stick_shadow.model_matrix[13] = player_stick.model_matrix[13];
	render_pong_element(&stick_shadow);


	stick_shadow.model_matrix[12] = stage.width / 2.0f;
	stick_shadow.model_matrix[13] = player_stick.model_matrix[13];
	render_pong_element(&stick_shadow);

}

void render_stage() {
	glUniform1i(glGetUniformLocation(program, "stageWireframe"), 1);
	glPolygonMode(GL_FRONT, GL_LINE);
	render_pong_element(&stage);
	glUniform1i(glGetUniformLocation(program, "stageWireframe"), 0);
	glPolygonMode(GL_FRONT, GL_FILL);
	render_pong_element(&stage);
}

void render_sticks() {
	render_pong_element(&opponent_stick);
	render_pong_element(&player_stick);
}

void render_balls_counter(int balls) {
	ball_mark.model_matrix[13] = -stage.height / 2.0f + 0.05f;

	float gap = ball_mark.width * 3.0f;
	ball_mark.model_matrix[12] = -((float)(balls) * gap) / 2.0f;

	for (int i = 0; i < balls - 1; i++) {
		ball_mark.model_matrix[12] += gap;
		ball_mark.model_matrix[14] = 0.0f;
		render_pong_element(&ball_mark);
	}
}

void render_ball() {
	render_pong_element(&ball);
}

GLuint renderer_get_main_program() {
	return program;
}

void render_overlay() {
	render_pong_element(&overlay);
}

void reset_overlay() {
	glUniform1f(alphaUniform, 0.0f);
}
void render_fadeout_overlay(float pAlpha) {
	glUniform1f(alphaUniform, pAlpha);
	render_overlay();
	glUniform1f(alphaUniform, 0.0f);
}

void render_opponent_stick() {
	render_pong_element(&opponent_stick);	
}
void render_player_stick(){
	render_pong_element(&player_stick);
}

