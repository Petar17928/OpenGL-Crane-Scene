#pragma once

#include <string>

class CRenderer
{
private:
	unsigned int shaderProgram;
	float joint1 = 0.0f;   // QW (rotacija baze / model2+)
	float joint2 = 0.0f;   // AS (rotacija ruke / model4+)
	float joint3 = 0.0f;   // ZX (uvlačenje model5)
	GLFWwindow* window;
	void CompileShaders();
public:
	CRenderer(GLFWwindow *window);
	void HandleInput();
	void DrawRoller(float r, float h, int nPartXZ, int nPartY);
	void DrawSphere(float r, int nPartXZ, int nPartY);
	void SetMaterial(float r, float g, float b);
	int PrepareTextures(std::string strTex);
	void DrawCrane(float r, float h, int nPartXZ, int nPartY, unsigned int textureID, float cameraAngle);
};

