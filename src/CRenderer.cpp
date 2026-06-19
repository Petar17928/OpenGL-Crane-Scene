#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "CRenderer.h"
#include <vector>
#include <iostream>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 materialDiffuse;
uniform vec3 materialAmbient;
uniform vec3 materialSpecular;
uniform float materialShininess;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // ambient
    vec3 ambient = materialAmbient;

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialDiffuse;

    // specular
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = spec * materialSpecular;

    // attenuation
    float distance = length(lightPos - FragPos);

    float constant = 1.0;
    float linear = 0.01;
    float quadratic = 0.001;

    float attenuation =
        1.0 / (constant +
               linear * distance +
               quadratic * distance * distance);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 lighting = ambient + diffuse + specular;
    vec4 texColor = texture(texture1, TexCoord);

    FragColor = vec4(lighting, 1.0) * texColor;
}
)";

CRenderer::CRenderer(GLFWwindow* window)
{
    CompileShaders();
    this->window = window;
}
void CRenderer::CompileShaders()
{
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // provera greske
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Vertex shader error:\n" << infoLog << std::endl;
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Fragment shader error:\n" << infoLog << std::endl;
    }

    // linkovanje u program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader link error:\n" << infoLog << std::endl;
    }

    // cleanup, vise ne trebaju
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void CRenderer::HandleInput()
{
    float speed = 0.02f;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) joint1 -= speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) joint1 += speed;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) joint2 -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) joint2 += speed;

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) joint3 += speed;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) joint3 -= speed;

    joint3 = glm::clamp(joint3, 0.0f, 1.0f);
}

void CRenderer::DrawRoller(float r, float h, int nPartXZ, int nPartY)
{
    SetMaterial(1.0f, 1.0f, 1.0f);
    //calculate vertices
    int totalVerts = 2 * (nPartXZ + 2) + nPartY * 2 * (nPartXZ + 1);
    std::vector<float> vertices;
    vertices.reserve(totalVerts * 8); // *8 jer su x,y,z + normale + uv;
    
    //base1
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);

    vertices.push_back(0.5f);
    vertices.push_back(0.5f);

    for (int i = 0; i <= nPartXZ; i++)
    {
        float angle = i * 2 * (float)M_PI / nPartXZ;
        vertices.push_back(r * cos(angle));
        vertices.push_back(0.0f);
        vertices.push_back(r * sin(angle));

        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);

        vertices.push_back(0.5f + 0.5f * cos(angle));
        vertices.push_back(0.5f + 0.5f * sin(angle));
    }

    //base2
    vertices.push_back(0.0f);
    vertices.push_back(h);
    vertices.push_back(0.0f);

    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

    vertices.push_back(0.5f);
    vertices.push_back(0.5f);

    for (int i = 0; i <= nPartXZ; i++)
    {
        float angle = i * 2 * (float)M_PI / nPartXZ;
        vertices.push_back(r * cos(angle));
        vertices.push_back(h);
        vertices.push_back(r * sin(angle));

        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);

        vertices.push_back(0.5f + 0.5f * cos(angle));
        vertices.push_back(0.5f + 0.5f * sin(angle));
    }
    
    //lateral
    for (int j = 0; j < nPartY; j++)
    {
        for (int i = 0; i <= nPartXZ; i++)
        {
            float angle = i * 2 * (float)M_PI / nPartXZ;
            float u = (float)i / nPartXZ;
            float v1 = (float)j / nPartY;
            float v2 = (float)(j  + 1)/ nPartY;
            //coordinates
            vertices.push_back(r * cos(angle));
            vertices.push_back(j * h / nPartY);
            vertices.push_back(r * sin(angle));

            //normals
            vertices.push_back(cos(angle));
            vertices.push_back(0.0f);
            vertices.push_back(sin(angle));

            //u and v
            vertices.push_back(u);
            vertices.push_back(v1);

            //coordinates
            vertices.push_back(r * cos(angle));
            vertices.push_back((j + 1) * h / nPartY);
            vertices.push_back(r * sin(angle));

            //normals
            vertices.push_back(cos(angle));
            vertices.push_back(0.0f);
            vertices.push_back(sin(angle));

            //u and v
            vertices.push_back(u);
            vertices.push_back(v2);
        }
    }
    
    
    //setup VBO and VAO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);

    // position - location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // normal - location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV - location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //draw
    int baseVerts = nPartXZ + 2;
    glDrawArrays(GL_TRIANGLE_FAN, 0, baseVerts);
    glDrawArrays(GL_TRIANGLE_FAN, baseVerts, baseVerts);

    int lateralStart = 2 * baseVerts;
    int vertsPerRow = 2 * (nPartXZ + 1);

    for (int j = 0; j < nPartY; j++) {
        glDrawArrays(GL_TRIANGLE_STRIP, lateralStart + j * vertsPerRow, vertsPerRow);
    }

    //cleanup
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

}

void CRenderer::DrawSphere(float r, int nPartXZ, int nPartY)
{
    SetMaterial(1.0f, 1.0f, 1.0f);

    std::vector<float> vertices;

    for (int j = 0; j < nPartY; j++)
    {
        float phi1 = (float)M_PI * j / nPartY;
        float phi2 = (float)M_PI * (j + 1) / nPartY;

        for (int i = 0; i <= nPartXZ; i++)
        {
            float theta = 2.0f * (float)M_PI * i / nPartXZ;

            float x1 = r * sin(phi1) * cos(theta);
            float y1 = r * cos(phi1);
            float z1 = r * sin(phi1) * sin(theta);

            float x2 = r * sin(phi2) * cos(theta);
            float y2 = r * cos(phi2);
            float z2 = r * sin(phi2) * sin(theta);

            float u = (float)i / nPartXZ;

            // top vertex
            vertices.insert(vertices.end(), {
                x1,y1,z1,
                x1 / r,y1 / r,z1 / r,
                u,(float)j / nPartY
                });

            // bottom vertex
            vertices.insert(vertices.end(), {
                x2,y2,z2,
                x2 / r,y2 / r,z2 / r,
                u,(float)(j + 1) / nPartY
                });
        }
    }

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    for (int j = 0; j < nPartY; j++)
    {
        int start = j * (nPartXZ + 1) * 2;

        glDrawArrays(GL_TRIANGLE_STRIP, start, (nPartXZ + 1) * 2);
    }
}
void CRenderer::SetMaterial(float r, float g, float b)
{
    glUseProgram(shaderProgram);

    float diffuse[3] = { r, g, b };
    float ambient[3] = { r * 0.25f, g * 0.25f, b * 0.25f };
    float specular[3] = { 1.0f, 1.0f, 1.0f }; 
    float shininess = 25.0f;

    glUniform3fv(glGetUniformLocation(shaderProgram, "materialDiffuse"), 1, diffuse);
    glUniform3fv(glGetUniformLocation(shaderProgram, "materialAmbient"), 1, ambient);
    glUniform3fv(glGetUniformLocation(shaderProgram, "materialSpecular"), 1, specular);
    glUniform1f(glGetUniformLocation(shaderProgram, "materialShininess"), shininess);
}

int CRenderer::PrepareTextures(std::string strTex)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(strTex.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << strTex << std::endl;
    }

    stbi_image_free(data);

    return textureID;
}

void CRenderer::DrawCrane(
    float r, float h,
    int nPartXZ, int nPartY,
    unsigned int textureID,
    float cameraAngle)
{
    glUseProgram(shaderProgram);

    //VIEW + PROJECTION

    float camX = 15.0f * cos(cameraAngle);
    float camZ = 15.0f * sin(cameraAngle);

    glm::mat4 view = glm::lookAt(
        glm::vec3(camX, 3.0f, camZ), 
        glm::vec3(0.0f, 2.5f, 0.0f),  
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f,
        100.0f
    );
    glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(glm::vec3(camX, 3.0f, camZ)));
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // TEXTURE SETUP

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    // MODEL + DRAW

    int modelLoc = glGetUniformLocation(shaderProgram, "model");

    // Model 1
    glm::mat4 model1 = glm::mat4(1.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
    DrawRoller(r, h, nPartXZ, nPartY);

    // Model 2
    glm::mat4 model2 = model1;

    // idi na vrh baze
    model2 = glm::translate(model2, glm::vec3(0.0f, h, 0.0f));

    // QW ROTACIJA (XZ joint)
    model2 = glm::rotate(model2, joint1, glm::vec3(0, 1, 0));

    // MODEL 2
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
    DrawRoller(r, h, nPartXZ, nPartY);

    // Model 3
    glm::mat4 model3 = model2;
    model3 = glm::translate(model3, glm::vec3(0.0f, h, 0.0f));

    glm::mat4 model3_scaled = glm::scale(
        model3,
        glm::vec3(0.5f, 0.5f, 0.5f)
    );

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3_scaled));
    DrawSphere(r, nPartXZ, nPartY);

    // Model 4
    glm::mat4 model4 = model3;

    // pivot na vrhu model3
    model4 = glm::translate(model4, glm::vec3(0.0f, h, 0.0f));

    // AS ROTACIJA (XY joint)
    model4 = glm::rotate(model4, joint2, glm::vec3(1, 0, 0));

    // tanki stub
    glm::mat4 model4_scaled = glm::scale(
        model4,
        glm::vec3(0.2f, 5.0f, 0.2f)
    );

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4_scaled));
    DrawRoller(r, h, nPartXZ, nPartY);

    // Model 5
    float maxLen = 5.0f * h;
    float slide = joint3;

    glm::mat4 model5 = model4;

    // UVLAČENJE - bez skaliranja, čista pozicija/translacija
    model5 = glm::translate(
        model5,
        glm::vec3(0.0f, (1.0f - slide) * maxLen, 0.0f)
    );

    // anchor = vrh OVOG dela kranа, PRE skaliranja
    //glm::vec3 anchorPos = glm::vec3(model5 * glm::vec4(0.0f, h, 0.0f, 1.0f));
    glm::vec3 anchorPos = glm::vec3(model5 * glm::vec4(0.0f, h * 4.5f, 0.0f, 1.0f));

    // SAD skaliraj za samo crtanje (mali vrh)
    glm::mat4 model5_draw = glm::scale(model5, glm::vec3(0.1f, 5.0f, 0.1f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model5_draw));
    DrawRoller(r, h, nPartXZ, nPartY);

    // STRUNA
    float ropeLen = 5.0f * h;
    glm::mat4 ropeModel = glm::mat4(1.0f);
    ropeModel = glm::translate(ropeModel, anchorPos);
    ropeModel = glm::scale(ropeModel, glm::vec3(0.02f, -ropeLen / h, 0.02f)); // NEGATIVNO Y
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ropeModel));
    DrawRoller(r, h, nPartXZ, nPartY);

    // TEG - na DNU strune
    glm::vec3 ropeBottomPos = anchorPos + glm::vec3(0.0f, -ropeLen, 0.0f);

    glm::mat4 weightModel = glm::mat4(1.0f);
    weightModel = glm::translate(weightModel, ropeBottomPos);
    weightModel = glm::scale(weightModel, glm::vec3(0.2f, 0.1f / h, 0.2f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(weightModel));
    DrawSphere(r, nPartXZ, nPartY);
}