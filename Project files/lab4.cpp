// uses framework Cocoa
// uses framework OpenGL
#define MAIN
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include "MicroGlut.h"

#define PI 3.14159265358979323846

float radians(float degrees) {
    return degrees * (PI / 180.0f);
}

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
mat4 worldToViewMatrix, modelToWorldMatrix;
vec3 cameraPosition = {0.0, 0.0, 5.0};  // Start 5 units away from the object
vec3 cameraDirection = {0.0, 0.0, -1.0}; // Initially pointing toward the -Z direction
vec3 cameraUp = {0.0, 1.0, 0.0};         // Up vector
float cameraSpeed = 0.1;                 // Speed of movement
float sensitivity = 0.01;                // Mouse sensitivity
float yaw = -90.0f, pitch = 0.0f;        // Initial orientation

Model *cube;
// Reference to shader program
GLuint shader;

void init(void)
{
	// GL inits
	glClearColor(0.5,0.6,1.0,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_TRUE);

	// Load and compile shader
	shader = loadShadersGT("lab4.vs", "lab4.fs", "lab4.gs",
							"lab4.tcs", "lab4.tes");
	glUseProgram(shader);

	// Upload geometry to the GPU:
	cube = LoadModelPlus("cube.obj");

	// Initialize transformations
	//projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 100.0);
	projectionMatrix = perspective(45.0f, 1.0f, 0.1f, 100.0f);
	worldToViewMatrix = lookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z, cameraPosition.x + cameraDirection.x, cameraPosition.y + cameraDirection.y, cameraPosition.z + cameraDirection.z, cameraUp.x, cameraUp.y, cameraUp.z);
	modelToWorldMatrix = IdentityMatrix();

	glUniform1i(glGetUniformLocation(shader, "TessLevelInner"), TessLevelInner);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter1"), TessLevelOuter1);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter2"), TessLevelOuter2);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter3"), TessLevelOuter3);

	// Upload matrices that we do not intend to change.
	glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, worldToViewMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

void display(void)
{
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update the view matrix based on the camera's position and direction
    worldToViewMatrix = lookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
                               cameraPosition.x + cameraDirection.x, cameraPosition.y + cameraDirection.y, cameraPosition.z + cameraDirection.z,
                               cameraUp.x, cameraUp.y, cameraUp.z);

    // Upload the updated view matrix to the shader
    glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, worldToViewMatrix.m);

    glUniformMatrix4fv(glGetUniformLocation(shader, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);

    DrawPatchModel(cube, shader, "in_Position", "in_Normal", "in_TexCoord");

    glutSwapBuffers();
}

void key(unsigned char key, int x, int y)
{
    vec3 right = Normalize(CrossProduct(cameraDirection, cameraUp));
    switch (key)
    {
    case 'w':  // Forward
        cameraPosition = VectorAdd(cameraPosition, ScalarMult(cameraDirection, cameraSpeed));
        break;
    case 's':  // Backward
        cameraPosition = VectorSub(cameraPosition, ScalarMult(cameraDirection, cameraSpeed));
        break;
    case 'a':  // Left
        cameraPosition = VectorSub(cameraPosition, ScalarMult(right, cameraSpeed));
        break;
    case 'd':  // Right
        cameraPosition = VectorAdd(cameraPosition, ScalarMult(right, cameraSpeed));
        break;
    case 'q':  // Up
        cameraPosition = VectorAdd(cameraPosition, ScalarMult(cameraUp, cameraSpeed));
        break;
    case 'e':  // Down
        cameraPosition = VectorSub(cameraPosition, ScalarMult(cameraUp, cameraSpeed));
        break;
    }
    glutPostRedisplay();
}

int prevx = 0, prevy = 0;

void mouseDragged(int x, int y)
{
    yaw += (x - prevx) * sensitivity;
    pitch += (prevy - y) * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    cameraDirection.x = cos(radians(yaw)) * cos(radians(pitch));
    cameraDirection.y = sin(radians(pitch));
    cameraDirection.z = sin(radians(yaw)) * cos(radians(pitch));
    cameraDirection = Normalize(cameraDirection);

    prevx = x;
    prevy = y;

    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(800, 800);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutCreateWindow ("Lab 4");
	glutDisplayFunc(display);
	//glutMouseFunc(mouseUpDown);
	glutMotionFunc(mouseDragged);
	glutKeyboardFunc(key);
	glutRepeatingTimer(20);
	init ();
	glutMainLoop();
	exit(0);
}
