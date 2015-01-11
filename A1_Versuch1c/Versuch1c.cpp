// Versuch1c.cpp
// Ausgangssoftware des 1. Praktikumsversuchs
// zur Vorlesung Echtzeit-3D-Computergrahpik
// von Prof. Dr. Alfred Nischwitz
// Programm umgesetzt mit der GLTools Library und Vertex-Arrays
#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLFrustum.h>
#include <math.h>
#include <math3d.h>
#include <GL/glut.h>
#include <AntTweakBar.h>

void CreateSphere3(int xOff, int yOff, int zOff, int radius);
void CreateSphere2(int xOff, int yOff, int zOff, int radius);
void CreateSphere(int xOff, int yOff, int zOff, int radius);
void CreateQuad(int id, float xOff, float yOff, float zOff, float length, float width, float height);
void CreateKone(int id, int xOff, int yOff, int zOff, int radius, bool back);
void CreateKone2(int id, int xOff, int yOff, int zOff, int radius, bool back);
void CreateKone3(int id, int xOff, int yOff, int zOff, int radius, bool back);
//void CreateCuboid(int xOff, int yOff, int zOff, int length, int width, int height);
//void CreateCircle(int id, int xOff, int yOff, int zOff, int radius);
//void CreatePipe(int xOff, int yOff, int zOff, int length, int radius);
//void CreateCylinder(int xOff, int yOff, int zOff, int length, int radius);
void CreateSphereMantle(int xOff, int yOff, int zOff, int radius);
void CreateSphereMantle2(int xOff, int yOff, int zOff, int radius);
void CreateSphereMantle3(int xOff, int yOff, int zOff, int radius);

void RenderScene(void);
void CreateGeometry(void);

GLShaderManager shaderManager;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;

// Cuboid
GLBatch quad[6];

// Zylinder
GLBatch rohr;
GLBatch kreis[2];

// Sphere
GLBatch sphereStripes[64];
GLBatch konus[10];

// Sphere2
GLBatch sphereStripes2[64];
GLBatch konus2[10];

// mond
GLBatch sphereStripes3[64];
GLBatch konus3[10];

// Definition der Kreiszahl
#define GL_PI 3.1415f

// Rotationsgroessen
static float rotation[] = { 0, 0, 0, 0 };

// Flags fuer Schalter
bool bCull = false;
bool bOutline = false;
bool bDepth = true;

unsigned int tesselation = 50;
float scaling = 1.0f;

int angle = 150;


//Set Funktion für GUI, wird aufgerufen wenn Variable im GUI geändert wird
void TW_CALL SetTesselation(const void *value, void *clientData) {
	//Pointer auf gesetzten Typ casten (der Typ der bei TwAddVarCB angegeben wurde)
	const unsigned int* uintptr = static_cast<const unsigned int*>(value);

	//Setzen der Variable auf neuen Wert
	tesselation = *uintptr;

	//Hier kann nun der Aufruf gemacht werden um die Geometrie mit neuem Tesselationsfaktor zu erzeugen
	CreateGeometry();
}

//Get Funktion für GUI, damit GUI Variablen Wert zum anzeigen erhält
void TW_CALL GetTesselation(void *value, void *clientData) {
	//Pointer auf gesetzten Typ casten (der Typ der bei TwAddVarCB angegeben wurde)
	unsigned int* uintptr = static_cast<unsigned int*>(value);

	//Variablen Wert and GUI weiterreichen
	*uintptr = tesselation;
}


//Set Funktion für GUI, wird aufgerufen wenn Variable im GUI geändert wird
void TW_CALL SetScaling(const void *value, void *clientData) {
	//Pointer auf gesetzten Typ casten (der Typ der bei TwAddVarCB angegeben wurde)
	const float* uintptr = static_cast<const float*>(value);

	//Setzen der Variable auf neuen Wert
	scaling = *uintptr;

	//Hier kann nun der Aufruf gemacht werden um die Geometrie mit neuem Tesselationsfaktor zu erzeugen
	CreateGeometry();
}

//Get Funktion für GUI, damit GUI Variablen Wert zum anzeigen erhält
void TW_CALL GetScaling(void *value, void *clientData) {
	//Pointer auf gesetzten Typ casten (der Typ der bei TwAddVarCB angegeben wurde)
	float* uintptr = static_cast<float*>(value);

	//Variablen Wert and GUI weiterreichen
	*uintptr = scaling;
}


//GUI
TwBar *bar;
void InitGUI() {
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar size='200 400'");
	TwAddVarRW(bar, "Model Rotation", TW_TYPE_QUAT4F, &rotation, "");
	TwAddVarRW(bar, "Depth Test?", TW_TYPE_BOOLCPP, &bDepth, "");
	TwAddVarRW(bar, "Culling?", TW_TYPE_BOOLCPP, &bCull, "");
	TwAddVarRW(bar, "Backface Wireframe?", TW_TYPE_BOOLCPP, &bOutline, "");
	//Hier weitere GUI Variablen anlegen. Für Farbe z.B. den Typ TW_TYPE_COLOR4F benutzen

	//Tesselation Faktor als unsigned 32 bit integer definiert
	TwAddVarCB(bar, "Tesselation", TW_TYPE_UINT32, SetTesselation, GetTesselation, NULL, "");

	TwAddVarCB(bar, "Skalierung", TW_TYPE_FLOAT, SetScaling, GetScaling, NULL, "");
}

void CreateGeometry() {
	//CreateCylinder(0, 0, 0, 20, 5);
	CreateSphere(0, 0, 0, 2.5);
	CreateSphere2(0, 0, 0, 5);
	CreateSphere3(0, 0, 0, 1);
	//CreateCuboid(0, 0, 0, 10, 30, 50);

}

// Malt Circle mit ID, falls zOff > 0 Front
void CreateCircle(int id, int xOff, int yOff, int zOff, int radius) {

	float size = 2 + 0.5 * scaling;

	int minTriangles = 4;
	float actualTriangles = tesselation * minTriangles;
	int numberOfVertices = actualTriangles + 2;

	// Erzeuge einen weiteren Triangle_Fan um den kreis zu bedecken
	// Manuel verändern... numberOfVertices
	M3DVector3f* kreisVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* kreisColors = new M3DVector4f[numberOfVertices]();

	// Das Zentrum des Triangle_Fans ist im Ursprung
	m3dLoadVector3(kreisVertices[0], xOff, yOff, zOff*size);
	m3dLoadVector4(kreisColors[0], 1, 0, 0, 1);
	int iPivot = 1 * size;
	int i = 1;

	for (int j = 0; j < numberOfVertices - 1; j++)
	{
		float angle;
		if (zOff > 0)
			angle = (2.0f*GL_PI) + j * (2.0f * GL_PI / actualTriangles);
		else
			angle = (2.0f*GL_PI) - j * (2.0f * GL_PI / actualTriangles);

		// Berechne x und y Positionen des naechsten Vertex
		float x = radius*size*cos(angle);
		float y = radius*size*sin(angle);


		// Alterniere die Farbe zwischen Rot und Gruen
		if ((iPivot % 2) == 0)
			m3dLoadVector4(kreisColors[i], 1, 0.8, 0.2, 1);
		else
			m3dLoadVector4(kreisColors[i], 0, 0.8, 0, 1);

		// Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
		iPivot++;

		// Spezifiziere den naechsten Vertex des Triangle_Fans
		m3dLoadVector3(kreisVertices[i], xOff + x, yOff + y, zOff*size);
		i++;
	}

	kreis[id].Begin(GL_TRIANGLE_FAN, numberOfVertices);
	kreis[id].CopyVertexData3f(kreisVertices);
	kreis[id].CopyColorData4f(kreisColors);
	kreis[id].End();
}


void CreateKone(int id, int xOff, int yOff, int zOff, int radius, bool back) {
	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());

	float size = 2 + 0.5 * scaling;

	int minTriangles = 2;
	float stripeAngle = GL_PI / tesselation;
	int actualTriangles = minTriangles * tesselation;
	int numberOfVertices = actualTriangles + 2;

	if (!back) {
		stripeAngle = (GL_PI * (tesselation - 1)) / tesselation;
	}

	//18 Vertices anlegen
	M3DVector3f* konusVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* konusColors = new M3DVector4f[numberOfVertices]();
	// Die Spitze des Konus ist ein Vertex, den alle Triangles gemeinsam haben;
	// um einen Konus anstatt einen Kreis zu produzieren muss der Vertex einen positiven z-Wert haben
	float peak;
	if (back){
		peak = radius * size;
	}
	else{
		peak = -radius * size;
	}


	m3dLoadVector3(konusVertices[0], xOff, yOff, peak + zOff);
	m3dLoadVector4(konusColors[0], 0, 1, 0, 1);

	// Kreise um den Mittelpunkt und spezifiziere Vertices entlang des Kreises
	// um einen Triangle_Fan zu erzeugen
	int iPivot = 1;
	int i = 1;

	for (int stripepart = 0; stripepart <= actualTriangles; stripepart++) {
		float angle;
		if (back) {
			angle = (2.0f*GL_PI) - stripepart * (2.0f*GL_PI / (actualTriangles));
		}
		else {
			angle = (2.0f*GL_PI) + stripepart * (2.0f*GL_PI / (actualTriangles));
		}

		float xNew = radius*size * cos(angle) * sin(stripeAngle);
		float yNew = radius*size * sin(angle) * sin(stripeAngle);
		float zNew = radius*size * cos(stripeAngle);

		// Alterniere die Farbe zwischen Rot und Gruen
		if ((iPivot % 2) == 0)
			m3dLoadVector4(konusColors[i], 1, 0.8, 0.2, 1);
		else
			m3dLoadVector4(konusColors[i], 0, 0.8, 0, 1);

		// Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
		iPivot++;

		m3dLoadVector3(konusVertices[i], xOff + xNew, yOff + yNew, zOff + zNew);
		i++;

	}

	konus[id].Begin(GL_TRIANGLE_FAN, numberOfVertices);
	konus[id].CopyVertexData3f(konusVertices);
	konus[id].CopyColorData4f(konusColors);
	konus[id].End();

}

void CreateKone2(int id, int xOff, int yOff, int zOff, int radius, bool back) {
	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());

	float size = 2 + 0.5 * scaling;

	int minTriangles = 2;
	float stripeAngle = GL_PI / tesselation;
	int actualTriangles = minTriangles * tesselation;
	int numberOfVertices = actualTriangles + 2;

	if (!back) {
		stripeAngle = (GL_PI * (tesselation - 1)) / tesselation;
	}

	//18 Vertices anlegen
	M3DVector3f* konusVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* konusColors = new M3DVector4f[numberOfVertices]();
	// Die Spitze des Konus ist ein Vertex, den alle Triangles gemeinsam haben;
	// um einen Konus anstatt einen Kreis zu produzieren muss der Vertex einen positiven z-Wert haben
	float peak;
	if (back){
		peak = radius * size;
	}
	else{
		peak = -radius * size;
	}


	m3dLoadVector3(konusVertices[0], xOff, yOff, peak + zOff);
	m3dLoadVector4(konusColors[0], 0, 1, 0, 1);

	// Kreise um den Mittelpunkt und spezifiziere Vertices entlang des Kreises
	// um einen Triangle_Fan zu erzeugen
	int iPivot = 1;
	int i = 1;

	for (int stripepart = 0; stripepart <= actualTriangles; stripepart++) {
		float angle;
		if (back) {
			angle = (2.0f*GL_PI) - stripepart * (2.0f*GL_PI / (actualTriangles));
		}
		else {
			angle = (2.0f*GL_PI) + stripepart * (2.0f*GL_PI / (actualTriangles));
		}

		float xNew = radius*size * cos(angle) * sin(stripeAngle);
		float yNew = radius*size * sin(angle) * sin(stripeAngle);
		float zNew = radius*size * cos(stripeAngle);

		// Alterniere die Farbe zwischen Rot und Gruen
		if ((iPivot % 2) == 0)
			m3dLoadVector4(konusColors[i], 1, 0.8, 0.2, 1);
		else
			m3dLoadVector4(konusColors[i], 0, 0.8, 0, 1);

		// Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
		iPivot++;

		m3dLoadVector3(konusVertices[i], xOff + xNew, yOff + yNew, zOff + zNew);
		i++;

	}

	konus2[id].Begin(GL_TRIANGLE_FAN, numberOfVertices);
	konus2[id].CopyVertexData3f(konusVertices);
	konus2[id].CopyColorData4f(konusColors);
	konus2[id].End();

}
void CreateKone3(int id, int xOff, int yOff, int zOff, int radius, bool back) {
	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());

	float size = 2 + 0.5 * scaling;

	int minTriangles = 2;
	float stripeAngle = GL_PI / tesselation;
	int actualTriangles = minTriangles * tesselation;
	int numberOfVertices = actualTriangles + 2;

	if (!back) {
		stripeAngle = (GL_PI * (tesselation - 1)) / tesselation;
	}

	//18 Vertices anlegen
	M3DVector3f* konusVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* konusColors = new M3DVector4f[numberOfVertices]();
	// Die Spitze des Konus ist ein Vertex, den alle Triangles gemeinsam haben;
	// um einen Konus anstatt einen Kreis zu produzieren muss der Vertex einen positiven z-Wert haben
	float peak;
	if (back){
		peak = radius * size;
	}
	else{
		peak = -radius * size;
	}


	m3dLoadVector3(konusVertices[0], xOff, yOff, peak + zOff);
	m3dLoadVector4(konusColors[0], 0, 1, 0, 1);

	// Kreise um den Mittelpunkt und spezifiziere Vertices entlang des Kreises
	// um einen Triangle_Fan zu erzeugen
	int iPivot = 1;
	int i = 1;

	for (int stripepart = 0; stripepart <= actualTriangles; stripepart++) {
		float angle;
		if (back) {
			angle = (2.0f*GL_PI) - stripepart * (2.0f*GL_PI / (actualTriangles));
		}
		else {
			angle = (2.0f*GL_PI) + stripepart * (2.0f*GL_PI / (actualTriangles));
		}

		float xNew = radius*size * cos(angle) * sin(stripeAngle);
		float yNew = radius*size * sin(angle) * sin(stripeAngle);
		float zNew = radius*size * cos(stripeAngle);

		// Alterniere die Farbe zwischen Rot und Gruen
		if ((iPivot % 2) == 0)
			m3dLoadVector4(konusColors[i], 1, 0.8, 0.2, 1);
		else
			m3dLoadVector4(konusColors[i], 0, 0.8, 0, 1);

		// Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
		iPivot++;

		m3dLoadVector3(konusVertices[i], xOff + xNew, yOff + yNew, zOff + zNew);
		i++;

	}

	konus3[id].Begin(GL_TRIANGLE_FAN, numberOfVertices);
	konus3[id].CopyVertexData3f(konusVertices);
	konus3[id].CopyColorData4f(konusColors);
	konus3[id].End();

}

void CreateSphere(int xOff, int yOff, int zOff, int radius){
	CreateSphereMantle(xOff, yOff, zOff, radius);
	CreateKone(0, xOff, yOff, zOff, radius, true);
	CreateKone(1, xOff, yOff, zOff, radius, false);
}

void CreateSphere2(int xOff, int yOff, int zOff, int radius){
	CreateSphereMantle2(xOff, yOff, zOff, radius);
	CreateKone2(0, xOff, yOff, zOff, radius, true);
	CreateKone2(1, xOff, yOff, zOff, radius, false);
}

void CreateSphere3(int xOff, int yOff, int zOff, int radius){
	CreateSphereMantle3(xOff, yOff, zOff, radius);
	CreateKone3(0, xOff, yOff, zOff, radius, true);
	CreateKone3(1, xOff, yOff, zOff, radius, false);
}
// Malt Sphere mit Radius radius.
void CreateSphereMantle(int xOff, int yOff, int zOff, int radius) {
	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());
	float size = 2 + 0.5 * scaling;

	int minTriangles = 4;
	float stripeAngle = GL_PI / tesselation;
	int numberOfStripes = tesselation - 2;
	int trianglesPerStrip = minTriangles * tesselation;
	int numberOfVertices = trianglesPerStrip + 2;

	// Erzeuge einen Triangle_Strip um den Mantel zu erzeugen
	// Manuel verändern... numberOfVertices
	M3DVector3f* sphereVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* sphereColors = new M3DVector4f[numberOfVertices]();

	// Stripes
	for (int stripenumber = 0; stripenumber < numberOfStripes; stripenumber++) {
		int i = 0;

		// TeilQuad
		for (int stripepart = 0; stripepart <= trianglesPerStrip / 2; stripepart++){
			float angle = (2.0f*GL_PI) - stripepart * (2.0f*GL_PI / (trianglesPerStrip / 2.0f));

			float x = radius*size * cos(angle) *sin(stripeAngle);
			float y = radius*size * sin(angle) *sin(stripeAngle);
			float z = radius*size * cos(stripeAngle);

			float tmpAngle = stripeAngle + GL_PI / tesselation;
			float xNew = radius*size * cos(angle) * sin(tmpAngle);
			float yNew = radius*size * sin(angle) * sin(tmpAngle);
			float zNew = radius*size * cos(tmpAngle);

			m3dLoadVector4(sphereColors[i], 0, 0, 1, 1);
			m3dLoadVector4(sphereColors[i + 1], 0, 0, 1, 1);


			m3dLoadVector3(sphereVertices[i], xOff + x, yOff + y, zOff + z);
			m3dLoadVector3(sphereVertices[i + 1], xOff + xNew, yOff + yNew, zOff + zNew);
			i += 2;

		}
		stripeAngle += GL_PI / tesselation;

		sphereStripes[stripenumber].Begin(GL_TRIANGLE_STRIP, numberOfVertices);
		sphereStripes[stripenumber].CopyVertexData3f(sphereVertices);
		sphereStripes[stripenumber].CopyColorData4f(sphereColors);
		sphereStripes[stripenumber].End();
	}
}

void CreateSphereMantle2(int xOff, int yOff, int zOff, int radius) {
	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());
	float size = 2 + 0.5 * scaling;

	int minTriangles = 4;
	float stripeAngle = GL_PI / tesselation;
	int numberOfStripes = tesselation - 2;
	int trianglesPerStrip = minTriangles * tesselation;
	int numberOfVertices = trianglesPerStrip + 2;

	// Erzeuge einen Triangle_Strip um den Mantel zu erzeugen
	// Manuel verändern... numberOfVertices
	M3DVector3f* sphereVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* sphereColors = new M3DVector4f[numberOfVertices]();

	// Stripes
	for (int stripenumber = 0; stripenumber < numberOfStripes; stripenumber++) {
		int i = 0;

		// TeilQuad
		for (int stripepart = 0; stripepart <= trianglesPerStrip / 2; stripepart++){
			float angle = (2.0f*GL_PI) - stripepart * (2.0f*GL_PI / (trianglesPerStrip / 2.0f));

			float x = radius*size * cos(angle) *sin(stripeAngle);
			float y = radius*size * sin(angle) *sin(stripeAngle);
			float z = radius*size * cos(stripeAngle);

			float tmpAngle = stripeAngle + GL_PI / tesselation;
			float xNew = radius*size * cos(angle) * sin(tmpAngle);
			float yNew = radius*size * sin(angle) * sin(tmpAngle);
			float zNew = radius*size * cos(tmpAngle);

			// Alterniere die Farbe zwischen Rot und Gruen
			m3dLoadVector4(sphereColors[i], 1, 1, 0, 1);
			m3dLoadVector4(sphereColors[i + 1], 1, 1, 0, 1);

			m3dLoadVector3(sphereVertices[i], xOff + x, yOff + y, zOff + z);
			m3dLoadVector3(sphereVertices[i + 1], xOff + xNew, yOff + yNew, zOff + zNew);
			i += 2;

		}
		stripeAngle += GL_PI / tesselation;

		sphereStripes2[stripenumber].Begin(GL_TRIANGLE_STRIP, numberOfVertices);
		sphereStripes2[stripenumber].CopyVertexData3f(sphereVertices);
		sphereStripes2[stripenumber].CopyColorData4f(sphereColors);
		sphereStripes2[stripenumber].End();
	}
}

void CreateSphereMantle3(int xOff, int yOff, int zOff, int radius) {
	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());
	float size = 2 + 0.5 * scaling;

	int minTriangles = 4;
	float stripeAngle = GL_PI / tesselation;
	int numberOfStripes = tesselation - 2;
	int trianglesPerStrip = minTriangles * tesselation;
	int numberOfVertices = trianglesPerStrip + 2;

	// Erzeuge einen Triangle_Strip um den Mantel zu erzeugen
	// Manuel verändern... numberOfVertices
	M3DVector3f* sphereVertices = new M3DVector3f[numberOfVertices]();
	M3DVector4f* sphereColors = new M3DVector4f[numberOfVertices]();

	// Stripes
	for (int stripenumber = 0; stripenumber < numberOfStripes; stripenumber++) {
		int i = 0;

		// TeilQuad
		for (int stripepart = 0; stripepart <= trianglesPerStrip / 2; stripepart++){
			float angle = (2.0f*GL_PI) - stripepart * (2.0f*GL_PI / (trianglesPerStrip / 2.0f));

			float x = radius*size * cos(angle) *sin(stripeAngle);
			float y = radius*size * sin(angle) *sin(stripeAngle);
			float z = radius*size * cos(stripeAngle);

			float tmpAngle = stripeAngle + GL_PI / tesselation;
			float xNew = radius*size * cos(angle) * sin(tmpAngle);
			float yNew = radius*size * sin(angle) * sin(tmpAngle);
			float zNew = radius*size * cos(tmpAngle);

			// Alterniere die Farbe zwischen Rot und Gruen
			m3dLoadVector4(sphereColors[i], 1, 1, 1, 1);
			m3dLoadVector4(sphereColors[i + 1], 0, 0, 0, 1);

			m3dLoadVector3(sphereVertices[i], xOff + x, yOff + y, zOff + z);
			m3dLoadVector3(sphereVertices[i + 1], xOff + xNew, yOff + yNew, zOff + zNew);
			i += 2;

		}
		stripeAngle += GL_PI / tesselation;

		sphereStripes3[stripenumber].Begin(GL_TRIANGLE_STRIP, numberOfVertices);
		sphereStripes3[stripenumber].CopyVertexData3f(sphereVertices);
		sphereStripes3[stripenumber].CopyColorData4f(sphereColors);
		sphereStripes3[stripenumber].End();
	}
}

// Aufruf draw scene
void RenderScene(void) {

	// TODO: push pop order

	// Clearbefehle für den color buffer und den depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Schalte culling ein falls das Flag gesetzt ist
	if (bCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	// Schalte depth testing ein falls das Flag gesetzt ist
	if (bDepth)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	// Zeichne die Rückseite von Polygonen als Drahtgitter falls das Flag gesetzt ist
	if (bOutline)
		glPolygonMode(GL_BACK, GL_LINE);
	else
		glPolygonMode(GL_BACK, GL_FILL);

	// Speichere den matrix state und führe die Rotation durch
	modelViewMatrix.PushMatrix();

	// rotiert ganze scene per tweakBar
	M3DMatrix44f rot;
	m3dQuatToRotationMatrix(rot, rotation);
	modelViewMatrix.MultMatrix(rot);

	// -------------------------------------------------------


	modelViewMatrix.Rotate(angle, 0, 1, 0);

	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());

	//Zeichne Sonne
	for (int iterator = 0; iterator < tesselation - 2; iterator++) {
		sphereStripes2[iterator].Draw();
	}
	konus2[0].Draw();
	konus2[1].Draw();

	// TODO: Scale()

	if (angle < 180) {
		modelViewMatrix.Translate(-50 - 80.f*((angle / 180.f)*(angle / 180.f)), -2, -0.3);
	}
	else {
		modelViewMatrix.Translate(-50 - 80.f * (((360.f - angle) / 180.f)*((360.f - angle) / 180.f)), -2, -0.3);
	}

	// TODO: replace by Rotate()
	//M3DMatrix44f blau;
	//const float rotation2[] = { rotation[0] * 2, rotation[1] * 1, rotation[2] * 1, rotation[3] * 1 };
	//m3dQuatToRotationMatrix(blau, rotation2);
	//modelViewMatrix.MultMatrix(rot);

	//setze den Shader für das Rendern
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());


	modelViewMatrix.PushMatrix();

	modelViewMatrix.Rotate(3*angle, 0, 1, 0);
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());

	// Zeichne Erde
	for (int iterator = 0; iterator < tesselation - 2; iterator++) {
		sphereStripes[iterator].Draw();
	}
	konus[0].Draw();
	konus[1].Draw();

	modelViewMatrix.Translate(-15, -2, -0.3);
	shaderManager.UseStockShader(GLT_SHADER_FLAT_ATTRIBUTES, transformPipeline.GetModelViewProjectionMatrix());


	// Zeichne Mond
	for (int iterator = 0; iterator < tesselation - 2; iterator++) {
		sphereStripes3[iterator].Draw();
	}
	konus3[0].Draw();
	konus3[1].Draw();


	// Hole die im Stack gespeicherten Transformationsmatrizen wieder zurück
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();


	TwDraw();

	// Vertausche Front- und Backbuffer
	glutSwapBuffers();
	glutPostRedisplay();
}

// Initialisierung des Rendering Kontextes
void SetupRC() {
	// Schwarzer Hintergrund
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// In Uhrzeigerrichtung zeigende Polygone sind die Vorderseiten.
	// Dies ist umgekehrt als bei der Default-Einstellung weil wir Triangle_Fans benützen
	glFrontFace(GL_CW);

	//initialisiert die standard shader
	shaderManager.InitializeStockShaders();
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	//erzeuge die geometrie
	CreateGeometry();
	InitGUI();
}

void SpecialKeys(int key, int x, int y) {
	TwEventKeyboardGLUT(key, x, y);
	// Zeichne das Window neu
	glutPostRedisplay();
}


void ChangeSize(int w, int h) {
	GLfloat nRange = 100.0f;

	// Verhindere eine Division durch Null
	if (h == 0)
		h = 1;
	// Setze den Viewport gemaess der Window-Groesse
	glViewport(0, 0, w, h);
	// Ruecksetzung des Projection matrix stack
	projectionMatrix.LoadIdentity();

	// Definiere das viewing volume (left, right, bottom, top, near, far)
	if (w <= h)
		viewFrustum.SetOrthographic(-nRange, nRange, -nRange*h / w, nRange*h / w, -nRange, nRange);
	else
		viewFrustum.SetOrthographic(-nRange*w / h, nRange*w / h, -nRange, nRange, -nRange, nRange);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	// Ruecksetzung des Model view matrix stack
	modelViewMatrix.LoadIdentity();

	TwWindowSize(w, h);
}

void ShutDownRC() {
	TwTerminate();
}

// animation
void Step(int n) {
	angle += 2.0f;
	if (angle > 360.f) {
		angle -= 360;
	}
	glutPostRedisplay();
	glutTimerFunc(25, Step, 0);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Versuch1");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Veralteter Treiber etc.
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
		return 1;
	}

	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); // same as MouseMotion
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);

	glutReshapeFunc(ChangeSize);
	glutSpecialFunc(SpecialKeys);
	glutDisplayFunc(RenderScene);
	glutTimerFunc(25, Step, 0);

	TwInit(TW_OPENGL, NULL);
	SetupRC();
	glutMainLoop();
	ShutDownRC();

	return 0;
}