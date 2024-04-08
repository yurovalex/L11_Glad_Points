
#include <glad/glad.h>

#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>


class guiDialog
{
public:
    guiDialog(GLFWwindow *w);
    void CreateGuiDialog();
    void ShowGui();
    auto GetParam() const
    {
        return objPar;
    }
    virtual ~guiDialog();
private:
    GLFWwindow* window;

    struct PropertiesObject
    {
        PropertiesObject(float a, float b, float c, int d ):
            valueRed(a), valueGreen(b), valueBlue(c), valueThickness(d)
        {}
        float valueRed;
        float valueGreen;
        float valueBlue;
        int valueThickness;
    };
    PropertiesObject objPar;
};

guiDialog::guiDialog(GLFWwindow *w):window(w), objPar{0.1,0.5,0.0,2}
{
    //Инициализация интерфейса ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void guiDialog::CreateGuiDialog()
{
    //Запуск нового окна ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //Элементы интерфейса в окне
    if (ImGui::Begin("Control propeties")) {
        ImGui::Text("Create a point's array");
        ImGui::SliderFloat("Red", &objPar.valueRed, 0.0f, 1.0f);
        ImGui::SliderFloat("Green", &objPar.valueGreen, 0.0f, 1.0f);
        ImGui::SliderFloat("Blue", &objPar.valueBlue, 0.0f, 1.0f);
        ImGui::SliderInt("Thickness", &objPar.valueThickness, 1, 50);
    }
    ImGui::End();

}

void guiDialog::ShowGui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

guiDialog::~guiDialog()
{
    //Освобождение ресурсов ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

class BaseOpenGL
{
//Шейдерная часть
    const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    uniform int vThickness;


    void main()
    {
        gl_Position = vec4(position, 1.0);
        gl_PointSize = vThickness;

    }
)";
    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform float vRed;
    uniform float vGreen;
    uniform float vBlue;
    void main()
    {
        FragColor = vec4(vRed, vGreen, vBlue, 1.0);
    }
)";

public:
    BaseOpenGL() = default;
    ~BaseOpenGL()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    //Инициализация шейдеров и создание шейдерной программмы
    void InitShaders()
    {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
    }

    //Добавить точку в контейнер
    bool AddPointXY(float _x, float _y, float _z)
    {
        if (!(_x >= -1 && _x <= 1) || !(_y >= -1 && _y <= 1))
        {
            return false;
        }
        points.push_back(_x);
        points.push_back(_y);
        points.push_back(_z);
        return true;
    }
    //Очистка контейнера с точками
    void ClearDataPoints()
    {
        points.clear();
    }

    //Получить адрес шейдерной программы
    GLuint&  GetShadersProgram()
    {
        return shaderProgram;
    }
    //Получить адрес VAO
    GLuint&  GetVAO()
    {
        return VAO;
    }

    //Получить количество точек
    int GetSize()
    {
        return points.size()/3;
    }

    //Инициализация буферов VAO и VBO
    void InitVBOVAO()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points.size(), points.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glEnable(GL_PROGRAM_POINT_SIZE);
    }

private:
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int VBO, VAO;
    unsigned int shaderProgram;
    std::vector <float> points;
};


void reshapeWindows (GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


double normalize(double value, double min, double max) {
    return (2 * (value - min) / (max - min)) - 1;
}


void curveButterfly(BaseOpenGL* obj, int& width, int& height)
{
    typedef struct {
        double x,y,z;
    } XYZ;
    constexpr size_t N = 20000;
    int i;
    double u;
    XYZ p,plast;
    double x_norm, y_norm, z_norm;

    for (i=0;i<N;i++) {
        u = i * 24.0 * M_PI / N;
        p.x = cos(u) * (exp(cos(u)) - 2 * cos(4 * u) - pow(sin(u / 12),5.0));
        p.y = sin(u) * (exp(cos(u)) - 2 * cos(4 * u) - pow(sin(u / 12),5.0));
        p.z = fabs(p.y) / 2;
        x_norm = normalize(height*.7 +50*p.y, 0, width); // Normalizing x to range [-1, 1]
        y_norm = normalize(width/3+50*p.x, 0, height);
        z_norm = normalize(p.z, 0, 10);
        obj->AddPointXY(x_norm,y_norm, z_norm);
        plast = p;
    }
}



int main()
{
    int width = 1024, height = 768;

    //Инициализация GLFW
    if (!glfwInit()) {
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(width, height, "L11 Example", NULL, NULL);
    glfwMakeContextCurrent(window);
    //Реакция на изменение размера окна
    glfwSetFramebufferSizeCallback(window, reshapeWindows);

    // Инициализация GLAD
    gladLoadGL();


    //Инициализация окна ImGui
    guiDialog objGui (window);

    //Работа с OpenGL 3.x
    BaseOpenGL obj;
    obj.InitShaders();

    //Удалить все точки
    obj.ClearDataPoints();

    //Построить кривую Butterfly Curve
    curveButterfly(&obj, width, height);


    obj.InitVBOVAO();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(shaderProgram);
        glUseProgram(obj.GetShadersProgram());
        glBindVertexArray(obj.GetVAO());
        glDrawArrays(GL_POINTS, 0, obj.GetSize());

        //Создание окна ImGui
        objGui.CreateGuiDialog();
        //Отображение ImGui
        objGui.ShowGui();

        auto tParamObj =  objGui.GetParam();
        glUniform1f(glGetUniformLocation(obj.GetShadersProgram(), "vRed"), tParamObj.valueRed);
        glUniform1f(glGetUniformLocation(obj.GetShadersProgram(), "vGreen"), tParamObj.valueGreen);
        glUniform1f(glGetUniformLocation(obj.GetShadersProgram(), "vBlue"), tParamObj.valueBlue);
        glUniform1i(glGetUniformLocation(obj.GetShadersProgram(), "vThickness"), tParamObj.valueThickness);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
