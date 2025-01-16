// uses framework Cocoa
// uses framework OpenGL
#define MAIN
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include "MicroGlut.h"
#include "noise1234.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


// ------------ DrawPatchModel: modified utility function DrawModel from LittleOBJLoader ---------------
static void ReportRerror(const char *caller, const char *name)
{
	static unsigned int draw_error_counter = 0;
   if(draw_error_counter < NUM_DRAWMODEL_ERROR)
   {
		    fprintf(stderr, "%s warning: '%s' not found in shader!\n", caller, name);
		    draw_error_counter++;
   }
   else if(draw_error_counter == NUM_DRAWMODEL_ERROR)
   {
		    fprintf(stderr, "%s: Number of error bigger than %i. No more vill be printed.\n", caller, NUM_DRAWMODEL_ERROR);
		    draw_error_counter++;
   }
}

// Same as DrawModel but with GL_PATCHES
void DrawPatchModel(Model *m, GLuint program, const char* vertexVariableName, const char* normalVariableName, const char* texCoordVariableName)
{
    #ifndef GL_PATCHES
        #define GL_PATCHES 0x0000000e
    #endif

	if (m != NULL)
	{
		GLint loc;

		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glBindVertexArray(m->vao);	// Select VAO

		glBindBuffer(GL_ARRAY_BUFFER, m->vb);
		loc = glGetAttribLocation(program, vertexVariableName);
		if (loc >= 0)
		{
			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(loc);
		}
		else
			ReportRerror("DrawModel", vertexVariableName);

		if (normalVariableName!=NULL)
		{
			loc = glGetAttribLocation(program, normalVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->nb);
				glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				ReportRerror("DrawModel", normalVariableName);
		}

		// VBO for texture coordinate data NEW for 5b
		if ((m->texCoordArray != NULL)&&(texCoordVariableName != NULL))
		{
			loc = glGetAttribLocation(program, texCoordVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->tb);
				glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				ReportRerror("DrawModel", texCoordVariableName);
		}

		glDrawElements(GL_PATCHES, m->numIndices, GL_UNSIGNED_INT, 0L);
	}
}

// Tesselation control variables
// Feel free to change these initializations!
GLint TessLevelInner = 11;
GLint TessLevelOuter1 = 11;
GLint TessLevelOuter2 = 11;
GLint TessLevelOuter3 = 11;


mat4 projectionMatrix;
mat4 worldToViewMatrix, modelToWorldMatrix, worldToViewMatrix2nd;

float latDeg = 10.0f;   // latitude in degrees
float lonDeg = 45.0f;   // longitude in degrees

// Convert degrees to radians
float lat = latDeg * (3.14159265359f / 180.0f);
float lon = lonDeg * (3.14159265359f / 180.0f);
float x = cos(lat) * cos(lon);
float y = cos(lat) * sin(lon);
float z = sin(lat);
float standingHeight = .5f; // e.g. a person’s height in the same units
float waterLevel = 100.0f;

float sizeX = 1600;
float sizeY = 1600;

Model *cube;
// Reference to shader program
GLuint shader;

inline float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

double floor(double v) {
	return v - (int)v;
}

inline vec3 floor(const vec3& v)
{
    return vec3(
        floor(v.x),
        floor(v.y),
        floor(v.z)
	);
}

inline vec3 fract(const vec3& v)
{
    return vec3(
        v.x - floor(v.x),
        v.y - floor(v.y),
        v.z - floor(v.z)
	);
}

vec3 random3(vec3 st)
{
    st = vec3( dot(st,vec3(127.1,311.7, 543.21)),
              dot(st,vec3(269.5,183.3, 355.23)),
              dot(st,vec3(846.34,364.45, 123.65)) ); // Haphazard additional numbers by IR
    return -1.0 + 2.0*fract(vec3(sin(st.x),sin(st.y),sin(st.z))*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
// Trivially extended to 3D by Ingemar
float noise(vec3 st)
{
    vec3 i = floor(st);
    vec3 f = fract(st);

    vec3 u = f*f*(3.0-2.0*f);

    return lerp(
    			lerp( lerp( dot( random3(i + vec3(0.0,0.0,0.0) ), f - vec3(0.0,0.0,0.0) ),
                     dot( random3(i + vec3(1.0,0.0,0.0) ), f - vec3(1.0,0.0,0.0) ), u.x),
                lerp( dot( random3(i + vec3(0.0,1.0,0.0) ), f - vec3(0.0,1.0,0.0) ),
                     dot( random3(i + vec3(1.0,1.0,0.0) ), f - vec3(1.0,1.0,0.0) ), u.x), u.y),

    			lerp( lerp( dot( random3(i + vec3(0.0,0.0,1.0) ), f - vec3(0.0,0.0,1.0) ),
                     dot( random3(i + vec3(1.0,0.0,1.0) ), f - vec3(1.0,0.0,1.0) ), u.x),
                lerp( dot( random3(i + vec3(0.0,1.0,1.0) ), f - vec3(0.0,1.0,1.0) ),
                     dot( random3(i + vec3(1.0,1.0,1.0) ), f - vec3(1.0,1.0,1.0) ), u.x), u.y), u.z

          	);
}


void init(void)
{
	// GL inits
	glClearColor(0.0,0.0,0.0,1.0);//(0.5,0.6,1.0,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Load and compile shader
	shader = loadShadersGT("lab4.vs", "lab4.fs", "lab4.gs",
							"lab4.tcs", "lab4.tes");
	glUseProgram(shader);

	// Upload geometry to the GPU:
	cube = LoadModelPlus("cube.obj");

	GLuint treeTextureID;
	SDL_Surface *surface;
	surface = IMG_Load("tree_billboard.png");
	glGenTextures(1,&treeTextureID);
	glBindTexture(GL_TEXTURE_2D, treeTextureID);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,surface->w,surface->h,0,GL_RGBA,GL_UNSIGNED_BYTE,surface->pixels);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	SDL_FreeSurface(surface);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, treeTextureID);
	glUniform1i(glGetUniformLocation(shader, "treeTexture"), 0);


	// Check water vs. land
	glUniform1i(glGetUniformLocation(shader, "waterLevel"), waterLevel);
    vec3 spherePos = normalize(vec3(x, y, z));


    float displacement = noise(spherePos * 5.0f) * 0.1f;
    float height = 1.0f + displacement; // '1.0' because spherePos is unit length
	printf("height: %f", height);
	if (height > waterLevel)
	{
		// spherePos += spherePos * displacement * 5.0 -> multiply radius
		spherePos *= (1.0f + 5.0f * displacement);
	}

	vec3 cameraPos = spherePos + normalize(spherePos) * standingHeight;

	vec3 up = normalize(spherePos); // local "up" = normal from center
	// Some direction tangent to 'up' (e.g. heading due east):
	vec3 forward = normalize(cross(vec3(0,1,0), up));
	// If the cross is near zero (e.g. if up ~ (0,1,0)), pick a fallback direction

	worldToViewMatrix = lookAt(cameraPos, cameraPos + forward, up);
	worldToViewMatrix2nd = lookAt(0,0,0);

	// Initialize transformations
	// projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 100.0);
	projectionMatrix = frustum(-0.1, 0.1, -0.5, 0.5, 0.05, 10.0);

	//glm::frustum(
	// 	float left,   float right,
	// 	float bottom, float top,
	// 	float nearVal, float farVal
	// );


	//worldToViewMatrix = lookAt(0, 0, 3, 0,0,0, 0,1,0); // change camPosLoc if first three values change
	// worldToViewMatrix = lookAt(0, 1, 0, //eye
	// 						   0, 1, 1, //center/looking at
	// 						   0, 1, 0); //up
	modelToWorldMatrix = IdentityMatrix();

	glUniform1i(glGetUniformLocation(shader, "TessLevelInner"), TessLevelInner);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter1"), TessLevelOuter1);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter2"), TessLevelOuter2);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter3"), TessLevelOuter3);

	// Upload matrices that we do not intend to change.
	glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, worldToViewMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(shader, "specMatrix"), 1, GL_TRUE, worldToViewMatrix2nd.m);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);

	glUniform3f(glGetUniformLocation(shader, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

}
void display(void)
{
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // First viewport (left half of the window)
    glViewport(0, 0, sizeX, sizeY);
    glUniformMatrix4fv(glGetUniformLocation(shader, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, worldToViewMatrix.m);

    float elapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Convert milliseconds to seconds
    GLint timeUniformLocation = glGetUniformLocation(shader, "u_time");
    glUniform1f(timeUniformLocation, elapsedTime);

    DrawPatchModel(cube, shader, "in_Position", "in_Normal", "in_TexCoord");

    // Clear depth buffer before drawing the second viewport
    glClear(GL_DEPTH_BUFFER_BIT);

    // Second viewport (right half of the window)
    glViewport(sizeX, 0, sizeX, sizeY);

    // Set up a new camera for the second view
    mat4 secondViewMatrix = lookAt(
        vec3(1.0f, 1.0f, 1.0f), // Camera position
        vec3(0.0f, 0.0f, 0.0f), // Look at the origin
        vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );
    glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, secondViewMatrix.m);

    // Render the scene from the second camera's perspective
    DrawPatchModel(cube, shader, "in_Position", "in_Normal", "in_TexCoord");

    glutSwapBuffers();
}



int prevx = 0, prevy = 0;

void mouseUpDown(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		prevx = x;
		prevy = y;
	}

	 // Print the matrix values
    printf("Matrix values:\n");
    printMat4(worldToViewMatrix);
}

void mouseDragged(int x, int y)
{
	vec3 p;
	mat4 m;

	// This is a simple and IMHO really nice trackball system:
	// You must have two things, the worldToViewMatrix and the modelToWorldMatrix
	// (and the latter is modified).

	// Use the movement direction to create an orthogonal rotation axis
	p.y = x - prevx;
	p.x = -(prevy - y);
	p.z = 0;

	// Transform the view coordinate rotation axis to world coordinates!
	mat3 wv3 = mat3(worldToViewMatrix); // mat4tomat3(worldToViewMatrix);
	p = inverse(wv3) * p; // MultMat3Vec3(InvertMat3(wv3), p);

	// Create a rotation around this axis and premultiply it on the model-to-world matrix
	m = ArbRotate(p, sqrt(p.x*p.x + p.y*p.y) / 50.0);
	modelToWorldMatrix = m * modelToWorldMatrix;

	prevx = x;
	prevy = y;

    // Check water vs. land
	// float waterLevel = 0.2f;
    vec3 spherePos = normalize(vec3(x, y, z));


    float displacement = noise(spherePos * 5.0f) * 0.1f;
    float height = 1.0f + displacement; // '1.0' because spherePos is unit length
	if (height > waterLevel)
	{
		// spherePos += spherePos * displacement * 5.0 -> multiply radius
		spherePos *= (1.0f + 5.0f * displacement);
	}

	vec3 cameraPos = spherePos + normalize(spherePos) * standingHeight;

	vec3 up = normalize(spherePos); // local "up" = normal from center
	// Some direction tangent to 'up' (e.g. heading due east):
	vec3 forward = normalize(cross(vec3(0,1,0), up));
	// If the cross is near zero (e.g. if up ~ (0,1,0)), pick a fallback direction

	worldToViewMatrix = lookAt(cameraPos, cameraPos + forward, up);
	glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, worldToViewMatrix.m);

	glUniform3f(glGetUniformLocation(shader, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	printVec3(cameraPos);

	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(sizeX, sizeY);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutCreateWindow("Projekt");
	glutDisplayFunc(display);
	glutMouseFunc(mouseUpDown);
	glutMotionFunc(mouseDragged);
	//glutKeyboardFunc(key);
	glutRepeatingTimer(10);
	init ();
	glutMainLoop();
	exit(0);
}
