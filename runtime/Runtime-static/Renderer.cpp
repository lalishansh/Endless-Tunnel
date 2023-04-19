//
// Created by Omen on 04-12-2022.
//

#include "Renderer.hpp"
#include "Components.hpp"

static_assert(PLATFORM_ANDROID);
#include <OpenGL/Module.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

/* 3D data. Vertex range -0.5..0.5 in all axes.
* Z -0.5 is near, 0.5 is far. */
const float vertices[] =
{
    /* Front face. */
    /* Bottom left */
    -0.5,  0.5, -0.5,//
    0.5, -0.5, -0.5,//
    -0.5, -0.5, -0.5,//
    /* Top right */
    -0.5,  0.5, -0.5,//
    0.5,  0.5, -0.5,//
    0.5, -0.5, -0.5,//
    /* Left face */
    /* Bottom left */
    -0.5,  0.5,  0.5,//
    -0.5, -0.5, -0.5,//
    -0.5, -0.5,  0.5,//
    /* Top right */
    -0.5,  0.5,  0.5,//
    -0.5,  0.5, -0.5,//
    -0.5, -0.5, -0.5,//
    /* Top face */
    /* Bottom left */
    -0.5,  0.5,  0.5,//
    0.5,  0.5, -0.5,//
    -0.5,  0.5, -0.5,//
    /* Top right */
    -0.5,  0.5,  0.5,//
    0.5,  0.5,  0.5,//
    0.5,  0.5, -0.5,//
    /* Right face */
    /* Bottom left */
    0.5,  0.5, -0.5,//
    0.5, -0.5,  0.5,//
    0.5, -0.5, -0.5,//
    /* Top right */
    0.5,  0.5, -0.5,//
    0.5,  0.5,  0.5,//
    0.5, -0.5,  0.5,//
    /* Back face */
    /* Bottom left */
    0.5,  0.5,  0.5,//
    -0.5, -0.5,  0.5,//
    0.5, -0.5,  0.5,//
    /* Top right */
    0.5,  0.5,  0.5,//
    -0.5,  0.5,  0.5,//
    -0.5, -0.5,  0.5,//
    /* Bottom face */
    /* Bottom left */
    -0.5, -0.5, -0.5,//
    0.5, -0.5,  0.5,//
    -0.5, -0.5,  0.5,//
    /* Top right */
    -0.5, -0.5, -0.5,//
    0.5, -0.5, -0.5,//
    0.5, -0.5,  0.5,//
};

/* 3D data. Vertex range -0.5..0.5 in all axes.
* Z -0.5 is near, 0.5 is far. */
const float wall_vertices[] =
{
	/* Front face. */
	/* Bottom left */
	-0.5,-0.5, -0.5,  //
	-0.5, -0.5, -0.5, //
	-0.5, -0.5, -0.5, //
	/* Top right */
	-0.5, -0.5, -0.5, //
	-0.5, -0.5, -0.5, //
	-0.5, -0.5, -0.5, //
    /* Left face */
    /* Bottom left */
    -0.5, -0.5,  0.5,//
    -0.5, -0.5, -0.5,//
    -0.5,  0.5,  0.5,//
    /* Top right */
    -0.5, -0.5, -0.5,//
    -0.5,  0.5, -0.5,//
    -0.5,  0.5,  0.5,//
    /* Top face */
    /* Bottom left */
    -0.5,  0.5, -0.5,//
    0.5,  0.5, -0.5,//
    -0.5,  0.5,  0.5,//
    /* Top right */
    0.5,  0.5, -0.5,//
    0.5,  0.5,  0.5,//
    -0.5,  0.5,  0.5,//
    /* Right face */
    /* Bottom left */
    0.5, -0.5, -0.5,//
    0.5, -0.5,  0.5,//
    0.5,  0.5, -0.5,//
    /* Top right */
    0.5, -0.5,  0.5,//
    0.5,  0.5,  0.5,//
    0.5,  0.5, -0.5,//
    /* Back face */
    /* Bottom left */
    0.5, -0.5,  0.5,//
    -0.5, -0.5,  0.5,//
    0.5,  0.5,  0.5,//
    /* Top right */
    -0.5, -0.5,  0.5,//
    -0.5,  0.5,  0.5,//
    0.5,  0.5,  0.5,//
    /* Bottom face */
    /* Bottom left */
    -0.5, -0.5,  0.5,//
    0.5, -0.5,  0.5,//
    -0.5, -0.5, -0.5,//
    /* Top right */
    0.5, -0.5,  0.5,//
    0.5, -0.5, -0.5,//
    -0.5, -0.5, -0.5,//
};

const float colors[36*3] = {
	/* Front face */
	/* Bottom left */
	0.7, 0.0, 0.0, /* red */
	0.0, 0.0, 0.7, /* blue */
	0.0, 0.7, 0.0, /* green */
	/* Top right */
	0.7, 0.0, 0.0, /* red */
	0.7, 0.7, 0.0, /* yellow */
	0.0, 0.0, 0.7, /* blue */
	/* Left face */
	/* Bottom left */
	0.7, 0.7, 0.7, /* white */
	0.0, 0.7, 0.0, /* green */
	0.0, 0.7, 0.7, /* cyan */
	0.7, 0.7, 0.7, /* white */
	0.7, 0.0, 0.0, /* red */
	0.0, 0.7, 0.0, /* green */
	/* Top face */
	/* Bottom left */
	0.7, 0.7, 0.7, /* white */
	0.7, 0.7, 0.0, /* yellow */
	0.7, 0.0, 0.0, /* red */
	/* Top right */
	0.7, 0.7, 0.7, /* white */
	0.0, 0.0, 0.0, /* black */
	0.7, 0.7, 0.0, /* yellow */
	/* Right face */
	/* Bottom left */
	0.7, 0.7, 0.0, /* yellow */
	0.7, 0.0, 0.7, /* magenta */
	0.0, 0.0, 0.7, /* blue */
	/* Top right */
	0.7, 0.7, 0.0, /* yellow */
	0.0, 0.0, 0.0, /* black */
	0.7, 0.0, 0.7, /* magenta */
	/* Back face */
	/* Bottom left */
	0.0, 0.0, 0.0, /* black */
	0.0, 0.7, 0.7, /* cyan */
	0.7, 0.0, 0.7, /* magenta */
	/* Top right */
	0.0, 0.0, 0.0, /* black */
	0.7, 0.7, 0.7, /* white */
	0.0, 0.7, 0.7, /* cyan */
	/* Bottom face */
	/* Bottom left */
	0.0, 0.7, 0.0, /* green */
	0.7, 0.0, 0.7, /* magenta */
	0.0, 0.7, 0.7, /* cyan */
	/* Top right */
	0.0, 0.7, 0.0, /* green */
	0.0, 0.0, 0.7, /* blue */
	0.7, 0.0, 0.7, /* magenta */

};

auto gVertexShader
	= R"(
attribute vec4 av4position;
attribute vec3 av3colour;

uniform mat4 mv;
uniform mat4 p;

varying vec3 vv3colour;
varying vec3 vv3position;
varying vec3 vv3posn_m;

void main()
{
    vv3colour = av3colour;
	vec4 position = mv*av4position;
	vv3posn_m = vec3(mv[3]);
	vv3position = position.xyz;
    gl_Position = p*position;
}
)";
auto gFragmentShader
	= R"(
precision lowp float;

uniform int my_texture_type;

varying vec3 vv3colour;
varying vec3 vv3position;
varying vec3 vv3posn_m;

//constant function
const float zoom = 10.;
const vec3 brickColor = vec3(0.45,0.29,0.23);
const vec3 lineColor = vec3(0.845);
const float edgePos = 1.5;

////random noise function
//float nrand( vec2 n )
//{
//	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
//}
//
//float sincosbundle(float val)
//{
//	return sin(cos(2.*val) + sin(4.*val)- cos(5.*val) + sin(3.*val))*0.05;
//}
////color function
//vec3 color(in vec2 uv)
//{
//    //grid and coord inside each cell
//    vec2 coord = floor(uv);
//    vec2 gv = fract(uv);
//
//    //for randomness in brick pattern, it could be better to improve the color randomness
//    //you can try to make your own
//   	float movingValue = -sincosbundle(coord.y)*2.;
//
//    //for the offset you can also make it more fuzzy by changing both
//    //the modulo value and the edge pos value
//    float offset = floor(mod(uv.y,2.0))*(edgePos);
//    float verticalEdge = abs(cos(uv.x + offset));
//
//    //color of the bricks
//    vec3 brick = brickColor - movingValue;
//
//
//    bool vrtEdge = step( 1. - 0.01, verticalEdge) == 1.;
//    bool hrtEdge = gv.y > (0.9) || gv.y < (0.1);
//
//    if(hrtEdge || vrtEdge)
//        return lineColor;
//    return brick;
//}

void main()
{
	vec3 fog_colour = vec3(0,0,0);
	float dist = length(vv3position);
	float fog_factor = (120.0 - dist)/119.0;
	vec3 final_color;
	//if (texture_type == 1)
	//	final_color = color(vv3position.zy - vv3posn_m.zy);
	//else if(texture_type == 2)
	//	final_color = color(vv3position.zx + vv3posn_m.zx);
	//else
		final_color = vv3colour;

    gl_FragColor = vec4(mix(fog_colour, final_color, fog_factor), 1.0);
}
)";

/*WARNING: 0:118: '#extension' : extension not supported: GL_ARB_texture_query_levels
WARNING: 0:119: '#extension' : extension not supported: GL_ARB_fragment_layer_viewport
ERROR: 0:121: '' :  syntax error, unexpected IDENTIFIER
ERROR: 1 compilation errors.  No code generated.*/

GLuint loadShader (GLenum shaderType, const char *pSource) {
	GLuint shader = glCreateShader (shaderType);
	if (shader) {
		glShaderSource (shader, 1, &pSource, NULL);
		glCompileShader (shader);
		GLint compiled = 0;
		glGetShaderiv (shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			GLint infoLen = 0;
			glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				char *buf = (char *)malloc (infoLen + 1);
				buf[infoLen] = '\0';
				if (buf) {
					glGetShaderInfoLog (shader, infoLen, NULL, buf);

					fprintf(stderr, "%s", buf);

					free (buf);
				}
				glDeleteShader (shader);
				shader = 0;
			}
		}
	}
	return shader;
}

GLuint createProgram (const char *pVertexSource, const char *pFragmentSource) {
	GLuint vertexShader = loadShader (GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) return 0;

	GLuint pixelShader = loadShader (GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) return 0;

	GLuint program = glCreateProgram();
	if (program) {
		glAttachShader (program, vertexShader);
		glAttachShader (program, pixelShader);
		glLinkProgram (program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv (program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint bufLength = 0;
			glGetProgramiv (program, GL_INFO_LOG_LENGTH, &bufLength);
			if (bufLength) {
				char *buf = (char *)malloc (bufLength + 1);
				buf[bufLength] = '\0';
				if (buf) {
					glGetProgramInfoLog (program, bufLength, NULL, buf);
					free (buf);
				}
			}
			glDeleteProgram (program);
			program = 0;
		}
	}
	return program;
}

#define GL_CHECK(x) x

void GL_APIENTRY MessageCallback (
	GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
	const void *userParam
) {
	fprintf (
		stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message
	);
}
namespace EndlessTunnel
{
	i32  Renderer::texture_type = 0;

	GLuint programID;
	GLint  iLocPosition;
	GLint  iLocColor;
	GLint  iLocMV;
	GLint  iLocP;
	GLint  iLocTextureType;
	void Renderer::OnAwake (const SETU::ECS::Registry *registry) {
        // During init, enable debug output
        glEnable (GL_DEBUG_OUTPUT);
        glDebugMessageCallback (MessageCallback, 0);

		programID = createProgram (gVertexShader, gFragmentShader);

		/* Initialize OpenGL ES. */
		GL_CHECK (glEnable (GL_BLEND));
		/* Should do: src * (src alpha) + dest * (1-src alpha). */
		GL_CHECK (glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        /* Get attribute locations of non-fixed attributes like colour and texture coordinates. */
        iLocPosition = GL_CHECK (glGetAttribLocation (programID, "av4position"));
        iLocColor    = GL_CHECK (glGetAttribLocation (programID, "av3colour"));

        /* Get uniform locations */
        iLocMV = GL_CHECK (glGetUniformLocation (programID, "mv"));
        iLocP = GL_CHECK (glGetUniformLocation (programID, "p"));
        iLocTextureType = GL_CHECK (glGetUniformLocation (programID, "my_texture_type"));

        GL_CHECK (glEnable (GL_CULL_FACE));
        GL_CHECK (glEnable (GL_DEPTH_TEST));

        /* Set clear screen color. */
        GL_CHECK (glClearColor (1.0f, 0.0f, 0.0f, 0.0f));
	}

	void Renderer::OnUpdate (const SETU::ECS::Registry *registry) {
		GL_CHECK (glUseProgram (programID));

		/* Enable attributes for position, color and texture coordinates etc. */
		GL_CHECK (glEnableVertexAttribArray (iLocPosition));
		GL_CHECK (glEnableVertexAttribArray (iLocColor));

		/* Populate attributes for position, color and texture coordinates etc. */
		GL_CHECK (glVertexAttribPointer (iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices));
		GL_CHECK (glVertexAttribPointer (iLocColor, 3, GL_FLOAT, GL_FALSE, 0, colors));

		const glm::mat4 perspective = [&] {
			glm::mat4 perspective;
			registry->view<DataComponents::Camera>().each (
				[&] (SETU::ECS::Entity entity, const DataComponents::Camera &cam) {
					perspective = cam.GetProjectionView();
				}
			);
			return perspective;
		}();

        GL_CHECK (glClearColor (1.0f, 1.0f, 0.0f, 0.0f));
		GL_CHECK (glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        GL_CHECK (glClearColor (1.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK (glUniformMatrix4fv (iLocP, 1, GL_FALSE, &perspective[0][0]));
		GL_CHECK (glUniform1i (iLocTextureType, texture_type));
		registry->view<DataComponents::Cube, DataComponents::Transform>().each (
			[&] (
				const SETU::ECS::Entity entity, const DataComponents::Cube &cube,
				const DataComponents::Transform &transform
			) {
				GL_CHECK (glUniformMatrix4fv (iLocMV, 1, GL_FALSE, &transform.GetTransform()[0][0])
			    );
				u32 num_vertices = 36;
				if (Renderer::texture_type == 2)
					GL_CHECK (glUniform1i (
						iLocTextureType, cube.Type == DataComponents::Cube::TYPE::BLOCKADE ? 1 : 2
					));
				switch (cube.Type) {
				case DataComponents::Cube::TYPE::BLOCKADE:
					GL_CHECK (
						glVertexAttribPointer (iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices)
					);
					;
					break;
				case DataComponents::Cube::TYPE::WALL:
					GL_CHECK (glVertexAttribPointer (
						iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, wall_vertices
					));
					;
					break;
				default: return;
				};

				GL_CHECK (glDrawArrays (GL_TRIANGLES, 0, num_vertices));
			}
		);
	}

	b8 Renderer::OnEvent (SETU::Event_Base &e, const SETU::ECS::Registry *registry) {
		return false;
	}

	void Renderer::OnPostUpdate (const SETU::ECS::Registry *registry) {}

	void Renderer::OnSleep (const SETU::ECS::Registry *registry) {}
} // namespace EndlessTunnel
