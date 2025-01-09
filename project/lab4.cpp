
// uses framework Cocoa
// uses framework OpenGL
#define MAIN
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include "MicroGlut.h"
#include "noise1234.h"

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
GLint TessLevelInner = 1;
GLint TessLevelOuter1 = 1;
GLint TessLevelOuter2 = 1;
GLint TessLevelOuter3 = 1;


mat4 projectionMatrix;
mat4 worldToViewMatrix, modelToWorldMatrix;

Model *cube;
// Reference to shader program
GLuint shader;

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

	// Initialize transformations
	projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 100.0);
	worldToViewMatrix = lookAt(0, 0, 3, 0,0,0, 0,1,0); // change camPosLoc if first three values change
	modelToWorldMatrix = IdentityMatrix();

	GLint camPosLoc = glGetUniformLocation(shader, "cameraPosWS");
	glUniform3f(camPosLoc, 0.0f, 0.0f, 3.0f);

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

	glUniformMatrix4fv(glGetUniformLocation(shader, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);

	float elapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Convert milliseconds to seconds
    GLint timeUniformLocation = glGetUniformLocation(shader, "u_time");
    glUniform1f(timeUniformLocation, elapsedTime);

	DrawPatchModel(cube, shader, "in_Position", "in_Normal", "in_TexCoord");

	glutSwapBuffers();
}

void key(unsigned char key, int x, int y)
{
	glUseProgram(shader);

	if (key == '+')
		TessLevelInner++;
	if (key == '-')
		if (TessLevelInner > 1)
			TessLevelInner--;
	if (key == '.')
	{
		TessLevelOuter1++;
		TessLevelOuter2++;
		TessLevelOuter3++;
	}
	if (key == ',')
	{
		if (TessLevelOuter1 > 1)
			TessLevelOuter1--;
		if (TessLevelOuter2 > 1)
			TessLevelOuter2--;
		if (TessLevelOuter3 > 1)
			TessLevelOuter3--;
	}

	if (key == '2')
		TessLevelOuter1++;
	if (key == '1')
		if (TessLevelOuter1 > 1)
			TessLevelOuter1--;
	if (key == '4')
		TessLevelOuter2++;
	if (key == '3')
		if (TessLevelOuter2 > 1)
			TessLevelOuter2--;
	if (key == '6')
		TessLevelOuter3++;
	if (key == '5')
		if (TessLevelOuter3 > 1)
			TessLevelOuter3--;

	printf("TessLevelInner = %d TessLevelOuter1 = %d TessLevelOuter2 = %d TessLevelOuter3 = %d\n", TessLevelInner, TessLevelOuter1, TessLevelOuter2, TessLevelOuter3);
	glUniform1i(glGetUniformLocation(shader, "TessLevelInner"), TessLevelInner);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter1"), TessLevelOuter1);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter2"), TessLevelOuter2);
	glUniform1i(glGetUniformLocation(shader, "TessLevelOuter3"), TessLevelOuter3);
	glutPostRedisplay();
}


int prevx = 0, prevy = 0;

void mouseUpDown(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		prevx = x;
		prevy = y;
	}
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
	glutMouseFunc(mouseUpDown);
	glutMotionFunc(mouseDragged);
	glutKeyboardFunc(key);
	glutRepeatingTimer(10);
	init ();
	glutMainLoop();
	exit(0);
}
