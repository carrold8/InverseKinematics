#define no_init_all deprecated

// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.
#include<list>
#include<algorithm>

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define LOWER_BONE "Lower_Bone.dae"
#define UPPER_BONE "Upper_Bone.dae"
#define BOX "box.dae"
#define SKYBOX "skybox.dae"
#define GROUND "ground_plane.dae"
#define DUMMY "dummy.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#define MAX_BONES 10


vec3 light_pos = vec3(1.2f, 1.0f, 2.0f);
vec3 blue = vec3(0.0f, 0.0f, 1.0f);
vec3 red = vec3(1.0f, 0.0f, 0.0f);
vec3 white = vec3(1.0f, 1.0f, 1.0f);


vec3 cameraPos = vec3(0.0f, 0.0f, 40.0f);
vec3 cameraTarget = vec3(0.0f, 0.0f, 0.0f);
vec3 cameraDirec = normalise(cameraPos - cameraTarget);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
vec3 camRight = normalise(cross(up, cameraDirec));
//vec3 cameraUp = cross(cameraDirec, camRight);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// mouse state
bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;


float camX = 0.0f;


#pragma region SimpleTypes
typedef struct ModelData
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;


class World_BoneData
{
public:
	double x;
	double y;
	double angle;
	double cosAngle;
	double sinAngle;

};

class World_BoneData1
{
public:
	double x;
	double y;
	double z;
	double angle_z;
	double cosAngle_z;
	double sinAngle_z;
	double angle_y;
	double cosAngle_y;
	double sinAngle_y;

};

class local_BoneData
{
public:
	double x;
	double y;
	double angle;

};

class local_BoneData1
{
public:
	double x;
	double y;
	double z;
	double angle_y;
	double angle_z;

};

enum CCDResult
{
	
	Success,
	Processing,
	Failure,
};


local_BoneData boneArray[MAX_BONES];
local_BoneData1 boneArray1[MAX_BONES];

//BoneData bone1, bone2; 


bool doSpline = false;
bool doWave = false;
bool moveLeft = false;

struct splinePoint2D {

	float x;
	float y;
};

struct spline {

	vector<splinePoint2D> points;

	splinePoint2D GetSplinePoint(float t) {

		int p0, p1, p2, p3;

		p1 = (int)t + 1;
		p2 = p1 + 1;
		p3 = p2 + 1;
		p0 = p1 - 1;

		t = t - (int)t;

		float t2 = t * t;
		float t3 = t2 * t;

		float q1 = -t3 + 2.0f * t2 - t;
		float q2 = 3.0f * t3 - 5.0f * t2 + 2.0f;
		float q3 = -3.0f * t3 + 4.0f * t2 + t;
		float q4 = t3 - t2;

		float tx = 0.5 * (points[p0].x * q1 + points[p1].x * q2 + points[p2].x * q3 + points[p3].x * q4);
		float ty = 0.5 * (points[p0].y * q1 + points[p1].y * q2 + points[p2].y * q3 + points[p3].y * q4);


		return { tx,ty };
	}

};

float splineIncrease = 0.0f;
float splineWave = 0.0f;

spline animationPath;
spline wavePath;

GLuint shaderProgramID;
GLuint shaderProgramRED;
GLuint shaderProgramBLUE;
unsigned int skyboxShader;


// Textures
unsigned int skyboxTexture;

ModelData lower, upper, box, ground, dummy;
GLuint lower_vao, upper_vao, box_vao, skybox_vao, skybox_vbo, ground_vao, dummy_vao;
float skybox;

vector<std::string> facesSkyBox;

vector<std::string> faces1
{
	("skybox/right.jpg"),
	("skybox/left.jpg"),
	("skybox/top.jpg"),
	("skybox/bottom.jpg"),
	("skybox/front.jpg"),
	("skybox/back.jpg")
};


versor QuatRotation3 = quat_from_axis_deg(0, 0, 0, 1);
versor QuatRotation2 = quat_from_axis_deg(0, 0, 0, 1);
versor QuatRotation1 = quat_from_axis_deg(0, 0, 0, 1);
versor QuatRotation = quat_from_axis_deg(0, 0, 0, 1);
//versor QuatRotation1 = quat_from_axis_deg(46.88, 0, -128, 48);

//ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

GLuint loc1, loc2, loc3;
GLfloat rotate_x = 0.0f;
GLfloat update_x = 0.0f;
GLfloat rotate_y = 0.0f;
GLfloat update_y = 0.0f;
GLfloat rotate_z = 0.0f;
GLfloat update_z = 0.0f;
GLfloat update_lower = 0.0f;
GLfloat update_upper = 0.0f;

float target_x = 17.0f;
float target_y = 0.0f;
float target_z = 0.0f;

vec3 targetPos;

//Bone Rotation variables
float theta3_deg_z;
float theta3_deg_y;
float theta2_deg;
float theta1_deg_z;
float theta1_deg_y;


// bone info
float theta_1_z = 0;
float theta_1_y = 0;
float current_theta_1_z = 0;
float current_theta_1_y = 0;
float theta_2 = 0;
float current_theta_2 = 0;
float theta_3_z = 0;
float theta_3_y = 0;
float current_theta_3_z = 0;
float current_theta_3_y = 0;
float L1 = 8;
float L2 = 4;

// End Effector position
float ePos_x = 16;
float ePos_y = 0;
float ePos_z = 0;

vec3 ePos;

float B3_Pos_x = 12;
float B3_Pos_y = 0;
float B3_Pos_z = 0;

float B2_Pos_x = 8;
float B2_Pos_y = 0;

vec3 B3_Pos;
vec2 B2_Pos;
vec3 B1_Pos = vec3(0, 0, 0);

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {

	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertex, const char* fragment)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgram = glCreateProgram();
	if (shaderProgram == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}


	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgram, vertex, GL_VERTEX_SHADER);
	AddShader(shaderProgram, fragment, GL_FRAGMENT_SHADER);


	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgram);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgram);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgram);
	return shaderProgram;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
GLuint generateObjectBufferMesh(ModelData mesh_data, GLuint shader) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.



	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shader, "vertex_position");
	loc2 = glGetAttribLocation(shader, "vertex_normal");
	loc3 = glGetAttribLocation(shader, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);



	//	This is for texture coordinates which you don't currently need, so I have commented it out
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//	This is for texture coordinates which you don't currently need, so I have commented it out

	return vao;

}
#pragma endregion VBO_FUNCTIONS



float degrees(float radians) {
	float pi = 3.14159265359;
	return (radians * (180 / pi));
}

double radians(double degree) {
	double pi = 3.14159265359;
	return (degree * (pi / 180));
}



void update1Bone(vec3 Bone_Pos) {

	// Get angle to rotate bone 1
	vec3 ej = vec3((ePos.v[0] - Bone_Pos.v[0]), (ePos.v[1] - Bone_Pos.v[1]), (ePos.v[2] - Bone_Pos.v[2]));
	vec3 tj = vec3((target_x - Bone_Pos.v[0]), (target_y - Bone_Pos.v[1]), (target_z - Bone_Pos.v[2]));

//	float numerator = ((ePos_x - Bone_Pos.v[0]) * (target_y - Bone_Pos.v[1])) - ((ePos_y - Bone_Pos.v[1]) * (target_x - Bone_Pos.v[0]));
	float numerator = dot(ej,tj);


	float ej_mag = sqrt(pow(ej.v[0], 2) + pow(ej.v[1], 2) + pow(ej.v[2], 2));
	float tj_mag = sqrt(pow(tj.v[0], 2) + pow(tj.v[1], 2) + pow(tj.v[2], 2));

	float denominator = ej_mag * tj_mag;


//	theta_1_z = acos(numerator / denominator);
	float rotationAngle = degrees(acos(numerator / denominator));

	float ePos_scale = ej_mag / tj_mag;

	vec3 next_ePos = vec3((ePos_scale * tj.v[0]), (ePos_scale * tj.v[1]), (ePos_scale * tj.v[2]));

	vec3 rotationAxis = cross(tj, ej);

	QuatRotation = QuatRotation * quat_from_axis_deg(rotationAngle, rotationAxis.v[0], rotationAxis.v[1], rotationAxis.v[2]);

	cout << "\n\nCalcualtion info : \n";

	cout << "\nB1 : (" << Bone_Pos.v[0] << ", " << Bone_Pos.v[1] << ", " << Bone_Pos.v[2] << ")		";
	cout << "ePos : (" << ePos.v[0] << ", " << ePos.v[1] << ", " << ePos.v[2] << ")		";
	cout << "target : (" << target_x << ", " << target_y << ", " << target_z << ")\n";

	cout << "\n EJ : (" << ej.v[0] << ", " << ej.v[1] << ", " << ej.v[2] << ")		";
	cout << "TJ : (" << tj.v[0] << ", " << tj.v[1] << ", " << tj.v[2] << ")\n";

	cout << "\nEJ Mag : " << ej_mag << "	";
	cout << "TJ Mag : " << tj_mag << "\n";

	cout << "\nNum : " << numerator << "	";
	cout << "Den : " << denominator << "\n";


	cout << "Theta : " << rotationAngle << endl;
	cout << "Cross Product : (" << rotationAxis.v[0] << ", " << rotationAxis.v[1] << ", " << rotationAxis.v[2] << ")\n";
	cout << "Next ePos : (" << next_ePos.v[0] << ", " << next_ePos.v[1] << ", " << next_ePos.v[2] << ")	\n";



	versor quat = quat_from_axis_deg(35.3, 0, -144, 128);
	versor quat_con = quat_from_axis_deg(35.3, 0, -144, 128);

//	vec3 quatMultiplyReuslt = quat * vec3(16, 0, 0) * quat_con;


	ePos = next_ePos;

	cout << "ePos adjusted: (" << ePos.v[0] << ", " << ePos.v[1] << ", " << ePos.v[2] << ")	\n";




}




void updatePositions() {


	ePos_x = (L1 * cos(current_theta_1_z)) + (L2 * cos(current_theta_1_z + current_theta_2) + (L2 * cos(current_theta_1_z + current_theta_2 + current_theta_3_z)));
	ePos_y = (L1 * sin(current_theta_1_z)) + (L2 * sin(current_theta_1_z + current_theta_2) + (L2 * sin(current_theta_1_z + current_theta_2 + current_theta_3_z)));

//	ePos_z = ((L1 + L2) * sin(current_theta_1_y)) + (L2 * sin(current_theta_1_y + current_theta_3_y));
	ePos_z =(L2 * sin( current_theta_3_y));


	// Update x and y positions for the end effector
	B3_Pos_x = (L1 * cos(current_theta_1_z)) + (L2 * cos(current_theta_1_z + current_theta_2));
	B3_Pos_y = (L1 * sin(current_theta_1_z)) + (L2 * sin(current_theta_1_z + current_theta_2));

	B3_Pos_z = 0;// (L1 * sin(current_theta_1)) + (L2 * sin(current_theta_1 + current_theta_2));


	// Update x and y positions for the base of bone 2
	B2_Pos_x = L1 * cos(current_theta_1_z);
	B2_Pos_y = L1 * sin(current_theta_1_z);

}

void updateBonePositions() {



	// Update x and y positions for the end effector
//	ePos_x = (L1 * cos(bone1.currentAngle)) + (L2 * cos(bone2.currentAngle));
//	ePos_y = (L1 * sin(bone1.currentAngle)) + (L2 * sin(bone2.currentAngle));


	// Update x and y positions for the base of bone 2
//	B2_Pos_x = L1 * cos(bone1.currentAngle);
//	B2_Pos_y = L1 * sin(bone1.currentAngle);

}

void updateAngles2(vec2 Bone_Pos, float bone_angle) {

	// Get angle to rotate bone 2
	vec3 ej = vec3((ePos_x - Bone_Pos.v[0]), (ePos_y - Bone_Pos.v[1]), 0);
	vec3 tj = vec3((target_x - Bone_Pos.v[0]), (target_y - Bone_Pos.v[1]), 0);

	float numerator = ((ePos_x - Bone_Pos.v[0]) * (target_y - Bone_Pos.v[1])) - ((ePos_y - Bone_Pos.v[1]) * (target_x - Bone_Pos.v[0]));
	float ej_mag = sqrt(pow(ej.v[0], 2) + pow(ej.v[1], 2) + pow(ej.v[2], 2));
	float tj_mag = sqrt(pow(tj.v[0], 2) + pow(tj.v[1], 2) + pow(tj.v[2], 2));

	float denominator = ej_mag * tj_mag;


	theta_2 = asin(numerator / denominator);
	current_theta_2 = current_theta_2 + theta_2;
//	theta_2 = degrees(acos(numerator / denominator));

//	theta_2 = acos(numerator / denominator);
//	bone_angle = bone_angle + theta_2;

	theta2_deg = degrees(theta_2);





}

void updateAngles1(vec3 Bone_Pos, float bone_angle) {

	// Get angle to rotate bone 1
	vec3 ej_z = vec3((ePos_x - Bone_Pos.v[0]), (ePos_y - Bone_Pos.v[1]), 0);
	vec3 tj_z = vec3((target_x - Bone_Pos.v[0]), (target_y - Bone_Pos.v[1]), 0);

	float numerator_z = ((ePos_x - Bone_Pos.v[0]) * (target_y - Bone_Pos.v[1])) - ((ePos_y - Bone_Pos.v[1]) * (target_x - Bone_Pos.v[0]));


	float ej_mag_z = sqrt(pow(ej_z.v[0], 2) + pow(ej_z.v[1], 2) + pow(ej_z.v[2], 2));
	float tj_mag_z = sqrt(pow(tj_z.v[0], 2) + pow(tj_z.v[1], 2) + pow(tj_z.v[2], 2));

	float denominator_z = ej_mag_z * tj_mag_z;


	theta_1_z = asin(numerator_z / denominator_z);

	
	current_theta_1_z = current_theta_1_z + theta_1_z;
	theta1_deg_z = degrees(theta_1_z);


//	updatePositions();

	// Get angle to rotate bone 3 about yt (x, z plane calculation)
	vec3 ej_y = vec3((ePos_x - Bone_Pos.v[0]), (ePos_z - Bone_Pos.v[2]), 0);
	vec3 tj_y = vec3((target_x - Bone_Pos.v[0]), (target_z - Bone_Pos.v[2]), 0);

	float numerator_y = ((ePos_x - Bone_Pos.v[0]) * (target_z - Bone_Pos.v[2])) - ((ePos_z - Bone_Pos.v[2]) * (target_x - Bone_Pos.v[0]));

	//	float ej_mag_y = sqrt(pow(ej_z.v[0], 2) + pow(ej_z.v[1], 2) + pow(ej_z.v[2], 2));
	//	float tj_mag_y = sqrt(pow(tj_z.v[0], 2) + pow(tj_z.v[1], 2) + pow(tj_z.v[2], 2));

	float ej_mag_y = sqrt(pow(ej_y.v[0], 2) + pow(0, 2) + pow(ej_y.v[1], 2));
	float tj_mag_y = sqrt(pow(tj_y.v[0], 2) + pow(0, 2) + pow(tj_y.v[1], 2));

	float denominator_y = ej_mag_y * tj_mag_y;

	theta_1_y = asin(numerator_y / denominator_y);

//	cout << "Numerator : " << numerator_y << endl;
//	cout << "Denominator : " << denominator_y << endl;

	current_theta_1_y = current_theta_1_y + theta_1_y;
	theta1_deg_y = degrees(theta_1_y);




}

void updateAngles3(vec3 Bone_Pos, float bone_angle) {

	// Get angle to rotate bone 1 about z (x, y plane calculation)
	vec3 ej_z = vec3((ePos_x - Bone_Pos.v[0]), (ePos_y - Bone_Pos.v[1]), 0);
	vec3 tj_z = vec3((target_x - Bone_Pos.v[0]), (target_y - Bone_Pos.v[1]), 0);

	float numerator_z = ((ePos_x - Bone_Pos.v[0]) * (target_y - Bone_Pos.v[1])) - ((ePos_y - Bone_Pos.v[1]) * (target_x - Bone_Pos.v[0]));

	float ej_mag_z = sqrt(pow(ej_z.v[0], 2) + pow(ej_z.v[1], 2) + pow(ej_z.v[2], 2));
	float tj_mag_z = sqrt(pow(tj_z.v[0], 2) + pow(tj_z.v[1], 2) + pow(tj_z.v[2], 2));

	float denominator_z = ej_mag_z * tj_mag_z;

	theta_3_z = asin(numerator_z / denominator_z);

	current_theta_3_z = current_theta_3_z + theta_3_z;
	theta3_deg_z = degrees(theta_3_z);


//	updatePositions();

	// Get angle to rotate bone 3 about yt (x, z plane calculation)
	vec3 ej_y = vec3((ePos_x - Bone_Pos.v[0]), (ePos_z - Bone_Pos.v[2]), 0);
	vec3 tj_y = vec3((target_x - Bone_Pos.v[0]), (target_z - Bone_Pos.v[2]), 0);

	float numerator_y = ((ePos_x - Bone_Pos.v[0]) * (target_z - Bone_Pos.v[2])) - ((ePos_z - Bone_Pos.v[2]) * (target_x - Bone_Pos.v[0]));

//	float ej_mag_y = sqrt(pow(ej_z.v[0], 2) + pow(ej_z.v[1], 2) + pow(ej_z.v[2], 2));
//	float tj_mag_y = sqrt(pow(tj_z.v[0], 2) + pow(tj_z.v[1], 2) + pow(tj_z.v[2], 2));

	float ej_mag_y = sqrt(pow(ej_y.v[0], 2) + pow(0, 2) + pow(ej_y.v[1], 2));
	float tj_mag_y = sqrt(pow(tj_y.v[0], 2) + pow(0, 2) + pow(tj_y.v[1], 2));

	float denominator_y = ej_mag_y * tj_mag_y;

	theta_3_y = asin(numerator_y / denominator_y);

//	cout << "Numerator : " << numerator_y << endl;
//	cout << "Denominator : " << denominator_y << endl;

	current_theta_3_y = current_theta_3_y + theta_3_y;
	theta3_deg_y = degrees(theta_3_y);




}






unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);


	cout << "Lenght of faces array: " << faces.size() << endl;

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}



// Start CCD using Classes -------------------------------------------------------------------------------------

double SimplifyAngle(double angle) {
	double pi = 3.14159265359;
	angle = fmodf(angle, 2.0 * pi);
	if (angle < -pi) {
		angle += (2.0 * pi);
	}
	else if (angle > pi) {
		angle -= (2.0 * pi);
	}
	return angle;
}


//void CCDResult(list<local_BoneData> bones) {

//}


CCDResult CCDIteration(local_BoneData bones[], double targetX, double targetY, double arrivalDist) {


	// Set epsilon to stop division by small numbers
	const double epsilon = 0.0001;

	// Set max arc length a bone can move the end effector an be considered no motion
	// so that we can detect a failure state.
	const double trivialArcLength = 0.00001;


//	int numBones = sizeof(bones);
//	const int numBones = 2;
	

	double arrivalDistSqr = arrivalDist * arrivalDist;

	// Create world space bone data
	World_BoneData worldBones[MAX_BONES]; 

	int numBones = sizeof(bones);

//	cout << "numbones : " << numBones << endl;

	// Start with root bone
	World_BoneData rootWorldBone;
	rootWorldBone.x = bones[0].x;
	rootWorldBone.y = bones[0].y;
	rootWorldBone.angle = bones[0].angle;
	rootWorldBone.cosAngle = cos(rootWorldBone.angle);
	rootWorldBone.sinAngle = sin(rootWorldBone.angle);
	worldBones[0] = rootWorldBone;

	for (int boneIdk = 1; boneIdk < numBones; boneIdk++) {

		World_BoneData prevWorldBone = worldBones[boneIdk - 1];
		local_BoneData curLocalBone = bones[boneIdk];

		World_BoneData newWorldBone;
		newWorldBone.x = prevWorldBone.x + prevWorldBone.cosAngle * curLocalBone.x
										- prevWorldBone.sinAngle * curLocalBone.y;
		newWorldBone.y = prevWorldBone.y + prevWorldBone.sinAngle * curLocalBone.x
										+ prevWorldBone.cosAngle * curLocalBone.y;
		newWorldBone.angle = prevWorldBone.angle + curLocalBone.angle;
		newWorldBone.cosAngle = cos(newWorldBone.angle);
		newWorldBone.sinAngle = sin(newWorldBone.angle);
		// Add local bone to world bone array
		worldBones[boneIdk] = newWorldBone;


	}

	// Get end effector position
	double endX = worldBones[numBones - 1].x;
	double endY = worldBones[numBones - 1].y;
	

	bool modifiedBones = false;
	// Now do CCD 
	for (int boneIdx = numBones - 2; boneIdx >= 0; --boneIdx)
	{
		// Get the vector from the current bone to the end effector position.
		double curToEndX = endX - worldBones[boneIdx].x;
		double curToEndY = endY - worldBones[boneIdx].y;
		double curToEndMag = sqrt(curToEndX * curToEndX + curToEndY * curToEndY);

		// Get the vector from the current bone to the target position.
		double curToTargetX = targetX - worldBones[boneIdx].x;
		double curToTargetY = targetY - worldBones[boneIdx].y;
		double curToTargetMag = sqrt(curToTargetX * curToTargetX
			+ curToTargetY * curToTargetY);

		// Get rotation to place the end effector on the line from the current
		// joint position to the target postion.
		double cosRotAng;
		double sinRotAng;
		double endTargetMag = (curToEndMag * curToTargetMag);
		if (endTargetMag <= epsilon)
		{
			cosRotAng = 1;
			sinRotAng = 0;
		}
		else
		{
			cosRotAng = (curToEndX * curToTargetX + curToEndY * curToTargetY) / endTargetMag;
			sinRotAng = (curToEndX * curToTargetY - curToEndY * curToTargetX) / endTargetMag;
		}

		// Clamp the cosine into range when computing the angle (might be out of range
		// due to floating point error).
		double rotAng = acos(max(-1.0, min(1.0, cosRotAng)));

		if (sinRotAng < 0.0)
			rotAng = -rotAng;

		// Rotate the end effector position.
		endX = worldBones[boneIdx].x + cosRotAng * curToEndX - sinRotAng * curToEndY;
		endY = worldBones[boneIdx].y + sinRotAng * curToEndX + cosRotAng * curToEndY;

		// Rotate the current bone in local space (this value is output to the user)
		bones[boneIdx].angle = SimplifyAngle(bones[boneIdx].angle + rotAng);

		// Check for termination
		double endToTargetX = (targetX - endX);
		double endToTargetY = (targetY - endY);
		if (endToTargetX * endToTargetX + endToTargetY * endToTargetY <= arrivalDistSqr)
		{
			// We found a valid solution.
			return Success;
		}

		// Track if the arc length that we moved the end effector was
		// a nontrivial distance.
		if (!modifiedBones && abs(rotAng) * curToEndMag > trivialArcLength)
		{
			modifiedBones = true;
		}
	}

	if (modifiedBones)
		return Processing;
	else
		return Failure;

}


CCDResult CCDIteration1(local_BoneData1 bones[], vec3 target_vector, double arrivalDist) {


	// Set epsilon to stop division by small numbers
	const double epsilon = 0.0001;

	// Set max arc length a bone can move the end effector an be considered no motion
	// so that we can detect a failure state.
	const double trivialArcLength = 0.00001;


	//	int numBones = sizeof(bones);
	//	const int numBones = 2;


	double arrivalDistSqr = arrivalDist * arrivalDist;

	// Create world space bone data
	World_BoneData1 worldBones[MAX_BONES];

	int numBones = sizeof(bones);

//	cout << "numbones : " << numBones << endl;

	// Start with root bone
	World_BoneData1 rootWorldBone;
	rootWorldBone.x = bones[0].x;
	rootWorldBone.y = bones[0].y;
	rootWorldBone.z = bones[0].z;
	rootWorldBone.angle_y = bones[0].angle_y;
	rootWorldBone.cosAngle_y = cos(rootWorldBone.angle_y);
	rootWorldBone.sinAngle_y = sin(rootWorldBone.angle_y);
	rootWorldBone.angle_z = bones[0].angle_z;
	rootWorldBone.cosAngle_z = cos(rootWorldBone.angle_z);
	rootWorldBone.sinAngle_z = sin(rootWorldBone.angle_z);
	worldBones[0] = rootWorldBone;

	for (int boneIdk = 1; boneIdk < numBones; boneIdk++) {

		World_BoneData1 prevWorldBone = worldBones[boneIdk - 1];
		local_BoneData1 curLocalBone = bones[boneIdk];

		World_BoneData1 newWorldBone;


		newWorldBone.x = prevWorldBone.x + (curLocalBone.x * prevWorldBone.cosAngle_z) * (prevWorldBone.cosAngle_y);

		newWorldBone.y = prevWorldBone.y + (curLocalBone.x * prevWorldBone.sinAngle_z);

		newWorldBone.z = prevWorldBone.x + (curLocalBone.x * prevWorldBone.cosAngle_z) * (prevWorldBone.sinAngle_y);

		newWorldBone.angle_y = prevWorldBone.angle_y + curLocalBone.angle_y;
		newWorldBone.cosAngle_y = cos(newWorldBone.angle_y);
		newWorldBone.sinAngle_y = sin(newWorldBone.angle_y);
		

		newWorldBone.angle_z = prevWorldBone.angle_z + curLocalBone.angle_z;
		newWorldBone.cosAngle_z = cos(newWorldBone.angle_z);
		newWorldBone.sinAngle_z = sin(newWorldBone.angle_y);


/*		newWorldBone.x = prevWorldBone.x + prevWorldBone.cosAngle_z * curLocalBone.x
			- prevWorldBone.sinAngle_z * curLocalBone.y;
		newWorldBone.y = prevWorldBone.y + prevWorldBone.sinAngle_z * curLocalBone.x
			+ prevWorldBone.cosAngle_z * curLocalBone.y;
		newWorldBone.angle_z = prevWorldBone.angle_z + curLocalBone.angle_z;
		newWorldBone.cosAngle_z = cos(newWorldBone.angle_z);
		newWorldBone.sinAngle_z = sin(newWorldBone.angle_z);
*/		// Add local bone to world bone array
		worldBones[boneIdk] = newWorldBone;

	}

	// Get end effector position
	double endX = worldBones[numBones - 1].x;
	double endY = worldBones[numBones - 1].y;
	double endZ = worldBones[numBones - 1].z;


	double total_z_rotation = 0;
	double total_y_rotation = 0;

	bool modifiedBones = false;
	// Now do CCD 
	for (int boneIdx = numBones - 2; boneIdx >= 0; --boneIdx)
	{
		// Get the vector from the current bone to the end effector position.
		// E - J
		double curToEndX = endX - worldBones[boneIdx].x;
		double curToEndY = endY - worldBones[boneIdx].y;
		double curToEndZ = endY - worldBones[boneIdx].z;
		// Mag EJ
		double curToEndMag = sqrt(curToEndX * curToEndX + curToEndY * curToEndY + curToEndZ * curToEndZ);

		// Get the vector from the current bone to the target position.
		// T - J
		double curToTargetX = target_vector.v[0] - worldBones[boneIdx].x;
		double curToTargetY = target_vector.v[1] - worldBones[boneIdx].y;
		double curToTargetZ = target_vector.v[2] - worldBones[boneIdx].z;
		// Mag TJ
		double curToTargetMag = sqrt(curToTargetX * curToTargetX
			+ curToTargetY * curToTargetY + curToTargetZ * curToTargetZ);


		float ejOverTj = curToEndMag / curToTargetMag;

		// Multiple TJ by ej/tj and add previous joint position to get end position

		double end_X = (curToTargetX * ejOverTj) + worldBones[boneIdx - 1].x;
		double end_Y = (curToTargetY * ejOverTj) + worldBones[boneIdx - 1].y;
		double end_Z = (curToTargetZ * ejOverTj) + worldBones[boneIdx - 1].z;

		if (end_X == 0) {
			end_X = 0.00001;
		}

		double worldTheta_y_axis = atan(end_Z / end_X) - total_y_rotation;
		total_y_rotation += worldTheta_y_axis;

		double worldTheta_z_axis = asin(end_Y / bones[boneIdx].x) - total_z_rotation;
		total_z_rotation += worldTheta_z_axis;

		bones[boneIdx].angle_z = worldTheta_z_axis;
		bones[boneIdx].angle_z = -worldTheta_y_axis;



		// ===================================
		// Get rotation to place the end effector on the line from the current
		// joint position to the target postion.
/*		double cosRotAng_y;
		double sinRotAng_y;
		double endTargetMag = (curToEndMag * curToTargetMag);
		if (endTargetMag <= epsilon)
		{
			cosRotAng_y = 1;
			sinRotAng_y = 0;
		}
		else
		{
			cosRotAng_y = (curToEndX * curToTargetX + curToEndZ * curToTargetY) / endTargetMag;
			sinRotAng_y = (curToEndX * curToTargetZ - curToEndZ * curToTargetX) / endTargetMag;
		}

		// Clamp the cosine into range when computing the angle (might be out of range
		// due to floating point error).
		double rotAng_y = acos(max(-1.0, min(1.0, cosRotAng_y)));

		if (sinRotAng_y > 0.0)
			rotAng_y = -rotAng_y;

		// Rotate the end effector position.
		endX = worldBones[boneIdx].x + cosRotAng_y * curToEndX - sinRotAng_y * curToEndY;
		endZ = worldBones[boneIdx].z + sinRotAng_y * curToEndX + cosRotAng_y * curToEndY;

		// Rotate the current bone in local space (this value is output to the user)
		bones[boneIdx].angle_y = SimplifyAngle(bones[boneIdx].angle_y + rotAng_y);
*/
		// Now have a y axis rotation number
		// need to update x and z positions in world space


		// ================================


		// Check for termination
		double endToTargetX = (target_vector.v[0] - endX);
		double endToTargetY = (target_vector.v[1] - endY);
		double endToTargetZ = (target_vector.v[2] - endZ);

		if (endToTargetX * endToTargetX + endToTargetY * endToTargetY <= arrivalDistSqr)
		{
			// We found a valid solution.
			return Success;
		}

		// Track if the arc length that we moved the end effector was
		// a nontrivial distance.
	//	if (!modifiedBones && abs(rotAng_z) * curToEndMag > trivialArcLength)
//		{
	//		modifiedBones = true;
	//	}
	}

	if (modifiedBones)
		return Processing;
	else
		return Failure;

}



// End CCD using Classes -------------------------------------------------------------------------------------


void display() {




	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramRED);



	//Declare your uniform variables that will be used in your shader
	// Vertex Shader Uniforms
	int matrix_location = glGetUniformLocation(shaderProgramBLUE, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramBLUE, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramBLUE, "proj");


	float theta = atan(target_y / target_x);
	//theta_1 = theta;
	theta = degrees(theta);


//	float theta1_deg = degrees(theta_1);

	

	// Root of the Hierarchy
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(fov, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 lower_bone = identity_mat4();

//	lower_bone = rotate_z_deg(lower_bone, theta1_deg);
	view = look_at(cameraPos, cameraPos + cameraFront, cameraUp);
//	view = translate(view, vec3(0.0, 0.0, -20));

	mat4 LBrotationMatrix = quat_to_mat4(QuatRotation1);
//	mat4 LBrotationMatrix = quat_to_mat4(QuatRotation);
//	LBrotationMatrix = glm::rotate(LBrotationMatrix, glm::radians(46.88), glm::vec3(0.0f, -128.0f, 48.0f));

	lower_bone = lower_bone * LBrotationMatrix;

//	lower_bone = rotate_y_deg(lower_bone, 16.35);
//	lower_bone = rotate_z_deg(lower_bone, -15.47);
	


		// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, lower_bone.m);


	glBindVertexArray(lower_vao);
	glDrawArrays(GL_TRIANGLES, 0, lower.mPointCount);

	



	// Trying to use second shader
	int matrix_location1 = glGetUniformLocation(shaderProgramRED, "model");
	int view_mat_location1 = glGetUniformLocation(shaderProgramRED, "view");
	int proj_mat_location1 = glGetUniformLocation(shaderProgramRED, "proj");

//	bone2.Quaternion = QuatRotation;

	mat4 UBrotationMatrix = quat_to_mat4(QuatRotation2);
//	mat4 UBrotationMatrix = quat_to_mat4(bone2.Quaternion);

	// Set up the child matrix
	mat4 upper_bone = identity_mat4();
//	upper_bone = rotate_z_deg(upper_bone, rotate_z);
//	upper_bone = rotate_z_deg(upper_bone, theta_2_deg);
	upper_bone = upper_bone * UBrotationMatrix;
	upper_bone = translate(upper_bone, vec3(8.0f, 0.0f, 0.0f));
	upper_bone = lower_bone * upper_bone;
	

	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, upper_bone.m);
	glBindVertexArray(upper_vao);
	glDrawArrays(GL_TRIANGLES, 0, upper.mPointCount);




	// Third Bone

	mat4 TBrotationMatrix = quat_to_mat4(QuatRotation3);

		// Set up the child matrix
	mat4 third_bone = identity_mat4();
	third_bone = third_bone * TBrotationMatrix;
	third_bone = translate(third_bone, vec3(4.0f, 0.0f, 0.0f));
	third_bone = upper_bone * third_bone;


	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, third_bone.m);
	glBindVertexArray(upper_vao);
	glDrawArrays(GL_TRIANGLES, 0, upper.mPointCount);


	glUseProgram(shaderProgramBLUE);

	mat4 target_model = identity_mat4();
	target_model = translate(target_model, vec3(target_x, target_y, target_z));

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, target_model.m);
	glBindVertexArray(box_vao);
	glDrawArrays(GL_TRIANGLES, 0, box.mPointCount);


	// Draw the skybox

	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader);

	glm::vec3 camPosGLM = glm::vec3(cameraPos.v[0], cameraPos.v[1], cameraPos.v[2]);
	glm::vec3 camFrontGLM = glm::vec3(cameraFront.v[0], cameraFront.v[1], cameraFront.v[2]);
	glm::vec3 camUpGLM = glm::vec3(cameraUp.v[0], cameraUp.v[1], cameraUp.v[2]);

	glm::mat4 viewSky = glm::mat4(1.0f);
	mat4 modelSky = identity_mat4();
	viewSky = glm::mat4(glm::mat3(glm::lookAt(camPosGLM, camPosGLM + camFrontGLM, camUpGLM)));

	int matrix_location_sky = glGetUniformLocation(skyboxShader, "model");
	int view_mat_location_sky = glGetUniformLocation(skyboxShader, "view");
	int proj_mat_location_sky = glGetUniformLocation(skyboxShader, "projection");
	int sampler_cube = glGetUniformLocation(skyboxShader, "skybox");

	glUniformMatrix4fv(proj_mat_location_sky, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location_sky, 1, GL_FALSE, &viewSky[0][0]);
	glUniformMatrix4fv(matrix_location_sky, 1, GL_FALSE, modelSky.m);
	glUniform1f(sampler_cube, 0);

	glBindVertexArray(skybox_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);




	glutSwapBuffers();
}


void test_z(vec3 target,  vec3 Bone ) {

	float new_x;
	float theta_z;
	float theta_y;
	
	vec3 ej = vec3((ePos_x - Bone.v[0]), (ePos_y - Bone.v[1]), (ePos_z - Bone.v[2]));
	vec3 tj = vec3((target_x - Bone.v[0]), (target_y - Bone.v[1]), (target_z - Bone.v[2]));


	float ej_mag = sqrt(pow(ej.v[0], 2) + pow(ej.v[1], 2) + pow(ej.v[2], 2));
	float tj_mag = sqrt(pow(tj.v[0], 2) + pow(tj.v[1], 2) + pow(tj.v[2], 2));

	
	if (tj.v[0] == 0) {
		tj.v[0] = 0.00001;
	}

	theta_z = atan(tj.v[1] / tj.v[0]);

	new_x = sqrt(pow(tj.v[0] , 2) + pow(tj.v[1], 2));

	if (new_x == 0) {
		new_x = 0.00001;
	}

	theta_y = atan(tj.v[2] / new_x);

	rotate_z = degrees(theta_z);
	rotate_y = -degrees(theta_y);

	versor quat_y_2 = quat_from_axis_deg(rotate_y, 0, 1, 0);
	versor quat_z_2 = quat_from_axis_deg(rotate_z, 0, 0, 1);
	QuatRotation = quat_from_axis_deg(rotate_z, 0, 0, 1);
	QuatRotation = QuatRotation * quat_y_2;

	// update ePos
	ePos_x = (ej_mag / tj_mag) * (tj.v[0]);
	ePos_y = (ej_mag / tj_mag) * (tj.v[1]);
	ePos_z = (ej_mag / tj_mag) * (tj.v[2]);

	
}


void display1() {




	CCDResult test = CCDIteration1(boneArray1, vec3(target_x, target_y, target_z), 0.0000001);

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramRED);



	//Declare your uniform variables that will be used in your shader
	// Vertex Shader Uniforms
	int matrix_location = glGetUniformLocation(shaderProgramBLUE, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramBLUE, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramBLUE, "proj");



		// Root of the Hierarchy
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(fov, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 lower_bone = identity_mat4();

	//	lower_bone = rotate_z_deg(lower_bone, theta1_deg);
	view = look_at(cameraPos, cameraPos + cameraFront, cameraUp);

//	mat4 rotationMatrix = quat_to_mat4(QuatRotation1);

//	lower_bone = lower_bone * rotationMatrix;
//	test_z(targetPos, vec3(0, 0, 0));

//	lower_bone = rotate_z_deg(lower_bone, rotate_z);
//	lower_bone = rotate_y_deg(lower_bone, rotate_y);

// Test of online thing ==================================================================================
//	lower_bone = rotate_y_deg(lower_bone, -degrees(boneArray[0].angle));

// Test of modified online thing ===================================================================
	lower_bone = rotate_y_deg(lower_bone, degrees(boneArray1[0].angle_y));
	lower_bone = rotate_z_deg(lower_bone, degrees(boneArray1[0].angle_z));

//	lower_bone = rotate_z_deg(lower_bone, 38.66);
//	lower_bone = rotate_y_deg(lower_bone,-30.96);
//	lower_bone = rotate_z_deg(lower_bone, 38.66);

	//cout << "Root Bone Angle : " << degrees(boneArray[0].angle) << endl;



			// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, lower_bone.m);


	glBindVertexArray(lower_vao);
	glDrawArrays(GL_TRIANGLES, 0, lower.mPointCount);





	// Trying to use second shader
	int matrix_location1 = glGetUniformLocation(shaderProgramRED, "model");
	int view_mat_location1 = glGetUniformLocation(shaderProgramRED, "view");
	int proj_mat_location1 = glGetUniformLocation(shaderProgramRED, "proj");

	//	bone2.Quaternion = QuatRotation;


		// Set up the child matrix
	mat4 upper_bone = identity_mat4();

//	test_z(targetPos, vec3(8,0,0));

//	mat4 rotationMatrix1 = quat_to_mat4(QuatRotation);

//	upper_bone = upper_bone * rotationMatrix1;

	// Test of online thing ==================================================================================
//	upper_bone = rotate_y_deg(upper_bone, -degrees(boneArray[1].angle));

	// Test of modified online thing ===================================================================

	upper_bone = rotate_y_deg(upper_bone, degrees(boneArray1[1].angle_y));
	upper_bone = rotate_z_deg(upper_bone, degrees(boneArray1[1].angle_z));


	upper_bone = translate(upper_bone, vec3(8.0f, 0.0f, 0.0f));
	upper_bone = lower_bone * upper_bone;


	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, upper_bone.m);
	glBindVertexArray(upper_vao);
	glDrawArrays(GL_TRIANGLES, 0, upper.mPointCount);




	// Third Bone

	// Set up the child matrix
	mat4 third_bone = identity_mat4();

//	mat4 rotationMatrix2 = quat_to_mat4(QuatRotation2);

//	third_bone = third_bone * rotationMatrix2;

// Test of online thing ==================================================================================
//	third_bone = rotate_y_deg(third_bone, -degrees(boneArray[2].angle));

	// Test of modified online thing ===================================================================
	third_bone = rotate_y_deg(third_bone, degrees(boneArray1[2].angle_y));
	third_bone = rotate_z_deg(third_bone, degrees(boneArray1[2].angle_z));

	third_bone = translate(third_bone, vec3(4.0f, 0.0f, 0.0f));


	third_bone = upper_bone * third_bone;



	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, third_bone.m);
	glBindVertexArray(upper_vao);
	glDrawArrays(GL_TRIANGLES, 0, upper.mPointCount);


	glUseProgram(shaderProgramBLUE);

	mat4 target_model = identity_mat4();
	target_model = translate(target_model, vec3(target_x, target_y, target_z));

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, target_model.m);
	glBindVertexArray(box_vao);
	glDrawArrays(GL_TRIANGLES, 0, box.mPointCount);

	mat4 ground_model = identity_mat4();
	
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, ground_model.m);
	glBindVertexArray(ground_vao);
	glDrawArrays(GL_TRIANGLES, 0, ground.mPointCount);




	// Draw the skybox

	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader);

	glm::vec3 camPosGLM = glm::vec3(cameraPos.v[0], cameraPos.v[1], cameraPos.v[2]);
	glm::vec3 camFrontGLM = glm::vec3(cameraFront.v[0], cameraFront.v[1], cameraFront.v[2]);
	glm::vec3 camUpGLM = glm::vec3(cameraUp.v[0], cameraUp.v[1], cameraUp.v[2]);

	glm::mat4 viewSky = glm::mat4(1.0f);
	mat4 modelSky = identity_mat4();
	viewSky = glm::mat4(glm::mat3(glm::lookAt(camPosGLM, camPosGLM + camFrontGLM, camUpGLM)));

	int matrix_location_sky = glGetUniformLocation(skyboxShader, "model");
	int view_mat_location_sky = glGetUniformLocation(skyboxShader, "view");
	int proj_mat_location_sky = glGetUniformLocation(skyboxShader, "projection");
	int sampler_cube = glGetUniformLocation(skyboxShader, "skybox");

	glUniformMatrix4fv(proj_mat_location_sky, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location_sky, 1, GL_FALSE, &viewSky[0][0]);
	glUniformMatrix4fv(matrix_location_sky, 1, GL_FALSE, modelSky.m);
	glUniform1f(sampler_cube, 0);

	glBindVertexArray(skybox_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);




	glutSwapBuffers();
}


void display2DSpline() {




	

	


		CCDResult test = CCDIteration(boneArray, target_x, target_y, 0.0000001);

		// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramRED);



	//Declare your uniform variables that will be used in your shader
	// Vertex Shader Uniforms
	int matrix_location = glGetUniformLocation(shaderProgramBLUE, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramBLUE, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramBLUE, "proj");



	// Root of the Hierarchy
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(fov, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 lower_bone = identity_mat4();

	//	lower_bone = rotate_z_deg(lower_bone, theta1_deg);
	view = look_at(cameraPos, cameraPos + cameraFront, cameraUp);


	// Test of online thing ==================================================================================
		lower_bone = rotate_z_deg(lower_bone, degrees(boneArray[0].angle));

				// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, lower_bone.m);


	glBindVertexArray(lower_vao);
	glDrawArrays(GL_TRIANGLES, 0, lower.mPointCount);

	// Trying to use second shader
	int matrix_location1 = glGetUniformLocation(shaderProgramRED, "model");
	int view_mat_location1 = glGetUniformLocation(shaderProgramRED, "view");
	int proj_mat_location1 = glGetUniformLocation(shaderProgramRED, "proj");



		// Set up the child matrix
	mat4 upper_bone = identity_mat4();

	// Test of online thing ==================================================================================
	upper_bone = rotate_z_deg(upper_bone, degrees(boneArray[1].angle));

	upper_bone = translate(upper_bone, vec3(8.0f, 0.0f, 0.0f));
	upper_bone = lower_bone * upper_bone;


	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, upper_bone.m);
	glBindVertexArray(upper_vao);
	glDrawArrays(GL_TRIANGLES, 0, upper.mPointCount);




	// Third Bone

	// Set up the child matrix
	mat4 third_bone = identity_mat4();


	// Test of online thing ==================================================================================
	third_bone = rotate_z_deg(third_bone, degrees(boneArray[2].angle));
	third_bone = translate(third_bone, vec3(4.0f, 0.0f, 0.0f));
	third_bone = upper_bone * third_bone;



	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, third_bone.m);
	glBindVertexArray(upper_vao);
//	glDrawArrays(GL_TRIANGLES, 0, upper.mPointCount);


	glUseProgram(shaderProgramBLUE);

	mat4 target_model = identity_mat4();
	target_model = translate(target_model, vec3(target_x, target_y, target_z));

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, target_model.m);
	glBindVertexArray(box_vao);
	glDrawArrays(GL_TRIANGLES, 0, box.mPointCount);

	mat4 ground_model = identity_mat4();

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, ground_model.m);
	glBindVertexArray(ground_vao);
	glDrawArrays(GL_TRIANGLES, 0, ground.mPointCount);

	mat4 dummy_model = identity_mat4();

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, dummy_model.m);
	glBindVertexArray(dummy_vao);
//	glDrawArrays(GL_TRIANGLES, 0, dummy.mPointCount);


	// Draw the skybox

	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShader);

	glm::vec3 camPosGLM = glm::vec3(cameraPos.v[0], cameraPos.v[1], cameraPos.v[2]);
	glm::vec3 camFrontGLM = glm::vec3(cameraFront.v[0], cameraFront.v[1], cameraFront.v[2]);
	glm::vec3 camUpGLM = glm::vec3(cameraUp.v[0], cameraUp.v[1], cameraUp.v[2]);

	glm::mat4 viewSky = glm::mat4(1.0f);
	mat4 modelSky = identity_mat4();
	viewSky = glm::mat4(glm::mat3(glm::lookAt(camPosGLM, camPosGLM + camFrontGLM, camUpGLM)));

	int matrix_location_sky = glGetUniformLocation(skyboxShader, "model");
	int view_mat_location_sky = glGetUniformLocation(skyboxShader, "view");
	int proj_mat_location_sky = glGetUniformLocation(skyboxShader, "projection");
	int sampler_cube = glGetUniformLocation(skyboxShader, "skybox");

	glUniformMatrix4fv(proj_mat_location_sky, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location_sky, 1, GL_FALSE, &viewSky[0][0]);
	glUniformMatrix4fv(matrix_location_sky, 1, GL_FALSE, modelSky.m);
	glUniform1f(sampler_cube, 0);

	glBindVertexArray(skybox_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);




	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	deltaTime = delta;
	lastFrame = last_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
//	rotate_x += 20.0f * delta;
//	rotate_x = fmodf(update_x, 360.0f);

//	rotate_y += 20.0f * delta;
//	rotate_y = fmodf(update_y, 360.0f);

	//rotate_z += 20.0f * delta;
	//rotate_z = fmodf(update_z, 360.0f);

	if (rotate_z > 360)
		rotate_z = 0;

	targetPos = vec3(target_x, target_y, target_x);
	ePos = vec3(ePos_x, ePos_y, ePos_z);
	B3_Pos = vec3(B3_Pos_x, B3_Pos_y, B3_Pos_z);
	B2_Pos = vec2(B2_Pos_x, B2_Pos_y);
	B1_Pos = vec3(0, 0, 0);

	
	//Rotate end bone
	updatePositions();
	updateAngles3(B3_Pos, current_theta_3_z);
//	updatePositions();
	versor quat_z3 = quat_from_axis_deg(theta3_deg_z, 0, 0, 1);
	versor quat_y3 = quat_from_axis_deg(-theta3_deg_y, 0, 1, 0);
	QuatRotation3 = QuatRotation3 * quat_z3;
	QuatRotation3 = QuatRotation3 * quat_y3;


	
	//Rotate middle bone
	updatePositions();
	updateAngles2(B2_Pos, current_theta_2);
//	updatePositions();
//	versor quat_z2 = quat_from_axis_deg(theta2_deg, 0, 0, 1);
//	QuatRotation2 = QuatRotation2 * quat_z2;

	// Rotate base bone
	updatePositions();
	updateAngles1(B1_Pos, current_theta_1_z);
//	updatePositions();
	versor quat_z_1 = quat_from_axis_deg(theta1_deg_z, 0, 0, 1);
//	versor quat_y_1 = quat_from_axis_deg(-theta1_deg_y, 0, 1, 0);
//	QuatRotation1 = QuatRotation1 * quat_z_1;
//	QuatRotation1 = QuatRotation1 * quat_y_1;



	if (doSpline == true && doWave != true) {

		splinePoint2D pos = animationPath.GetSplinePoint(splineIncrease);

		target_x = pos.x;
		target_y = pos.y;


		splineIncrease +=  0.01f;
		if (splineIncrease >= (float)animationPath.points.size() - 3.0f) {
			splineIncrease = 0;
			doSpline = false;
		}

	}


	if (doWave == true && doSpline != true) {

		splinePoint2D pos = wavePath.GetSplinePoint(splineWave);

		target_x = pos.x;
		target_y = pos.y;

		if (moveLeft == false) {
			splineWave = splineWave + 0.01f;
		}

		if (moveLeft == true) {
			splineWave = splineWave - 0.01f;
		}
		if (splineWave >= (float)wavePath.points.size() - 3.0f) {
			moveLeft = true;
		}
		if (splineWave <= 0) {
			moveLeft = false;
		}

	}




	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	shaderProgramBLUE = CompileShaders("Phong_Vertex.txt", "Phong_Fragment.txt");
	shaderProgramRED = CompileShaders("Toon_Vertex.txt", "Toon_Fragment.txt");
	lower = load_mesh(LOWER_BONE);
	lower_vao = generateObjectBufferMesh(lower, shaderProgramBLUE);
	// load mesh into a vertex buffer array
	upper = load_mesh(UPPER_BONE);
	upper_vao = generateObjectBufferMesh(upper, shaderProgramRED);
	box = load_mesh(BOX);
	box_vao = generateObjectBufferMesh(box, shaderProgramRED);

	ground = load_mesh(GROUND);
	ground_vao = generateObjectBufferMesh(ground, shaderProgramRED);

	dummy = load_mesh(DUMMY);
	dummy_vao = generateObjectBufferMesh(dummy, shaderProgramRED);
	

	// Skybox 
	skyboxShader = CompileShaders("Sky_Vertex.txt", "Sky_Fragment.txt");

	float skybox[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	
	unsigned int location = glGetAttribLocation(skyboxShader, "vertex_position");

	glGenVertexArrays(1, &skybox_vao);
	glGenBuffers(1, &skybox_vbo);
	glBindVertexArray(skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox), &skybox, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	skyboxTexture = loadCubemap(faces1);
	


	ePos = vec3(ePos_x, ePos_y, ePos_z);


	// Spline Setup
	animationPath.points = { {7,7}, {10, 12}, {16, 7}, {12, 1}, {10, 5}, {8.5, 2}, {7,7}, {10,12}, {16,7} };
	wavePath.points = { {2,3}, {7, 7}, {13, 7}, {18, 3} };


	local_BoneData rootBone;
	rootBone.angle = 0;
	rootBone.x = 0;
	rootBone.y = 0;

	local_BoneData secondBone;
	secondBone.angle = 0;
	secondBone.x = 8;
	secondBone.y = 0;


	local_BoneData thirdBone;
	thirdBone.angle = 0;
	thirdBone.x = 4;
	thirdBone.y = 0;


	local_BoneData fourthBone;
	fourthBone.angle = 0;
	fourthBone.x = 4;
	fourthBone.y = 0;

	local_BoneData fifthBone;
	fifthBone.angle = 0;
	fifthBone.x = 4;
	fifthBone.y = 0;


	boneArray[0] = rootBone;
	boneArray[1] = secondBone;
	boneArray[2] = thirdBone;
//	boneArray[3] = fourthBone;
//	boneArray[4] = fifthBone;


//	versor quat_y = quat_from_axis_deg(rotate_y, 0, 1, 0);
//	versor quat_z = quat_from_axis_deg(rotate_z, 0, 0, 1);
//	QuatRotation = QuatRotation * quat_z;
//	QuatRotation = QuatRotation * quat_y;


	local_BoneData1 rootBone1;
	rootBone1.angle_z = 0;
	rootBone1.angle_y = 0;
	rootBone1.x = 0;
	rootBone1.y = 0;
	rootBone1.z = 0; 

	local_BoneData1 secondBone1;
	secondBone1.angle_z = 0;
	secondBone1.angle_y = 0;
	secondBone1.x = 8;
	secondBone1.y = 0;
	secondBone1.z = 0;


	local_BoneData1 thirdBone1;
	thirdBone1.angle_z = 0;
	thirdBone1.angle_y = 0;
	thirdBone1.x = 4;
	thirdBone1.y = 0;
	thirdBone1.z = 0;


	local_BoneData1 fourthBone1;
	fourthBone1.angle_z = 0;
	fourthBone1.angle_y = 0;
	fourthBone1.x = 4;
	fourthBone1.y = 0;
	fourthBone1.z = 0;



	boneArray1[0] = rootBone1;
	boneArray1[1] = secondBone1;
	boneArray1[2] = thirdBone1;
	boneArray1[3] = fourthBone1;



			 


}



vec3 vecXfloat(float f, vec3 v1) {

	vec3 result = vec3((v1.v[0] * f), (v1.v[1] * f), (v1.v[2] * f));
	return result;
}



// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {


	float cameraSpeed = 20 * deltaTime;


	if (key == 'h') {
		cameraPos += vecXfloat(cameraSpeed, cameraFront);
	}
	if (key == 'n') {
		cameraPos -= vecXfloat(cameraSpeed, cameraFront);
	}
	if (key == 'b') {
		cameraPos -= normalise(cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (key == 'm') {
		cameraPos += normalise(cross(cameraFront, cameraUp)) * cameraSpeed;
	}


	// Roll Controls
	if (key == 'a') {
		target_x -= 0.5f;
	}
	if (key == 'd') {
		target_x += 0.5f;
	}
	if (key == 'w') {
		target_y += 0.5f;
	}
	if (key == 's') {
		target_y -= 0.5f;
	}

	if (key == 'f') {
		target_z += 0.5f;
	}
	if (key == 'r') {
		target_z -= 0.5f;
	}

	float t = 0.0f;

	if (key == 'i') {
		
		doSpline = true;

	}

	//Rotates end bone
	if (key == 'o') {
		target_x = 0;
		target_y = 0;
		doWave = !doWave;

	}

	// Rotates middle bone
	if (key == 'p') {
		
	//	target_x = 9.94;
	//	target_y = 2.91;
	//	target_z = 1.94;
		
		rotate_z = 38.7;
		rotate_y = -19.04;
		versor quat_y = quat_from_axis_deg(rotate_y, 0, 1, 0);
		versor quat_z = quat_from_axis_deg(rotate_z, 0, 0, 1);
	//	QuatRotation = QuatRotation * quat_z;
		QuatRotation = quat_from_axis_deg(rotate_y, 0, 1, 0);
		QuatRotation = QuatRotation * quat_z;
	}

	//rotates base bone
	if (key == 'u') {

	//	target_x = 8.34;
	//	target_y = 5.36;
	//	target_z = 3.57;

		
		rotate_z = 38.71;
		rotate_y = -36.66;
		versor quat_y = quat_from_axis_deg(rotate_y, 0, 1, 0);
		versor quat_z = quat_from_axis_deg(rotate_z, 0, 0, 1);
		//	QuatRotation = QuatRotation * quat_z;
		//QuatRotation1 = quat_from_axis_deg(rotate_z, 0, 0, 1);
		QuatRotation1 = quat_from_axis_deg(rotate_y, 0, 1, 0);
		QuatRotation1 = QuatRotation1 * quat_z;
	//	test_z(targetPos, vec3(8,0,0));

	//	versor quat_y_2 = quat_from_axis_deg(rotate_y, 0, 1, 0);
	//	versor quat_z_2 = quat_from_axis_deg(rotate_z, 0, 0, 1);
	//	QuatRotation = quat_from_axis_deg(rotate_z, 0, 0, 1);
	//	QuatRotation = QuatRotation * quat_y_2;

	}


	if (key == 'q') {
		update_y += 20.0f;
		versor quat_y = quat_from_axis_deg(1, 0, 1, 0);
		QuatRotation1 = QuatRotation1 * quat_y;
	}
	if (key == 'e') {
		update_y -= 20.0f;
		versor quat_y = quat_from_axis_deg(-1, 0, 1, 0);
		QuatRotation1 = QuatRotation1 * quat_y;
	}

	if (key == 't') {
		versor quat_z = quat_from_axis_deg(1, 0, 0, 1);
		QuatRotation1 = QuatRotation1 * quat_z;
	}
	if (key == 'g') {
		versor quat_z = quat_from_axis_deg(-1, 0, 0, 1);
		QuatRotation1 = QuatRotation1 * quat_z;
	}
}



void mouseCallback(int x, int y) {


	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	float xoffset = x - lastX;
	float yoffset = lastY - y;
	lastX = x;
	lastY = y;

	float sensitivity = 0.5f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	vec3 front;
	front.v[0] = cos(radians(yaw)) * cos(radians(pitch));
	front.v[1] = sin(radians(pitch));
	front.v[2] = sin(radians(yaw)) * cos(radians(pitch));
	cameraFront = normalise(front);


 }

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("2D IK");

	// Tell glut where the display function is
	glutDisplayFunc(display1);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutPassiveMotionFunc(mouseCallback);
//	glutMouseFunc(OnMouseClick);
	glutSetCursor(GLUT_CURSOR_NONE);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}