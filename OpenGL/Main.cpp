#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

const double pi = 3.14159265358979323846;

// error reporting

static void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "):" << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

// shaders

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& vertex, const std::string& fragment) {
    char current;

    std::ifstream vertexstream(vertex);
    std::string vertexout;
    if (vertexstream.is_open()) {
        while (vertexstream) {
            current = vertexstream.get();
            vertexout += current;
        }
    }

    std::ifstream fragmentstream(fragment);
    std::string fragmentout;
    if (fragmentstream.is_open()) {
        while (fragmentstream) {
            current = fragmentstream.get();
            fragmentout += current;
        }
    }

    vertexout.resize(vertexout.size() - 1); //removes the last character from both strings because it was a weird character causing problems
    fragmentout.resize(fragmentout.size() - 1);

    return { vertexout, fragmentout };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << 
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// function to find the points of a regular polygon

static float FindPoints(float size, unsigned int sides, unsigned int index, bool coord) {
    if (coord) {
        return (float) (size * cos(2 * pi * index / sides));
    }
    else {
        return (float) (size * sin(2 * pi * index / sides) * 16 / 9);
    }
}

const float size = 0.02f; // size of the ball

float positions[] = {
    -0.98f, -0.20f, // player 1
    -0.96f, -0.20f,
    -0.96f,  0.20f,
    -0.98f,  0.20f,
     FindPoints(size, 8, 0, true), FindPoints(size, 8, 0, false), // 4 08, 09 ball
     FindPoints(size, 8, 1, true), FindPoints(size, 8, 1, false), // 5 10, 11
     FindPoints(size, 8, 2, true), FindPoints(size, 8, 2, false), // 6 12, 13
     FindPoints(size, 8, 3, true), FindPoints(size, 8, 3, false), // 7 14, 15
     FindPoints(size, 8, 4, true), FindPoints(size, 8, 4, false), // 8 16, 17
     FindPoints(size, 8, 5, true), FindPoints(size, 8, 5, false), // 9 18, 19
     FindPoints(size, 8, 6, true), FindPoints(size, 8, 6, false), // 10 20, 21
     FindPoints(size, 8, 7, true), FindPoints(size, 8, 7, false), // 11 22, 23
     0.98f, -0.20f, // player 2
     0.96f, -0.20f,
     0.96f,  0.20f,
     0.98f,  0.20f,
    -1.00f, -1.00f, // background
     1.00f, -1.00f,
     1.00f,  1.00f,
    -1.00f,  1.00f
};

float start[] = {
    -0.98f, -0.20f, // player 1
    -0.96f, -0.20f,
    -0.96f,  0.20f,
    -0.98f,  0.20f,
     FindPoints(size, 8, 0, true), FindPoints(size, 8, 0, false), // 4 08, 09 ball
     FindPoints(size, 8, 1, true), FindPoints(size, 8, 1, false), // 5 10, 11
     FindPoints(size, 8, 2, true), FindPoints(size, 8, 2, false), // 6 12, 13
     FindPoints(size, 8, 3, true), FindPoints(size, 8, 3, false), // 7 14, 15
     FindPoints(size, 8, 4, true), FindPoints(size, 8, 4, false), // 8 16, 17
     FindPoints(size, 8, 5, true), FindPoints(size, 8, 5, false), // 9 18, 19
     FindPoints(size, 8, 6, true), FindPoints(size, 8, 6, false), // 10 20, 21
     FindPoints(size, 8, 7, true), FindPoints(size, 8, 7, false), // 11 22, 23
     0.98f, -0.20f, // player 2
     0.96f, -0.20f,
     0.96f,  0.20f,
     0.98f,  0.20f,
    -1.00f, -1.00f, // background
     1.00f, -1.00f,
     1.00f,  1.00f,
    -1.00f,  1.00f
};

// handles key presses
float vert;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        vert += 0.01f;
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
        vert -= 0.01f;
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        vert -= 0.01f;
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
        vert += 0.01f;
}

// a function that returns true if a point is in a rectangle and false if it isn't
static bool RectCollision(float x1, float y1, float x2, float y2, float x, float y) {
    if (x > x1 and x < x2 and y > y1 and y < y2)
        return true;
    return false;
}

// checks collision for player 1
static bool Player1Collision() {
    for (int i = 8; i < 23; i += 2) {
        if (RectCollision(positions[0], positions[1], positions[4], positions[5], positions[i], positions[i + 1]))
            return true;
    }
    return false;
}

// checks collision for player 2
static bool Player2Collision() {
    for (int i = 8; i < 23; i += 2) {
        if (RectCollision(positions[26], positions[27], positions[30], positions[31], positions[i], positions[i + 1]))
            return true;
    }
    return false;
}

static void SetUniformColor(int location, unsigned char r, unsigned char g, unsigned char b) {
    glUniform4f(location, (static_cast<GLfloat>(r) / 255), (static_cast<GLfloat>(g) / 255), (static_cast<GLfloat>(b) / 255), 1.0f);
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current 
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "Error" << std::endl;

    std::srand(static_cast<unsigned int>(std::time(nullptr))); // initializing rand with current time

    rand(); // wasing the first rand call

    const float speedInc = 0.0001f;
    float variance = 0.15f; // ammount of angle variance during a bounce
    float ballAngle = (float)((pi * ((2 * rand()) + 98301)) / 131068);

    if ((ballAngle < (3 * pi / 4)) || (ballAngle > (5 * pi / 4)))
        ballAngle = 3.0f;

    float ballSpeed = 0.005f;
    bool collision = false;

    unsigned int timer = 0;

    unsigned int background[] = {
        16, 17, 18,
        18, 19, 16
    };

    unsigned int ball[] = {
        4, 6, 5,
        4, 7, 6,
        4, 8, 7,
        4, 9, 8,
        4, 10, 9,
        4, 11, 10
    };

    unsigned int player1[] = { // must be unsigned
        0, 1, 2,
        2, 3, 0
    };

    unsigned int player2[] = {
        12, 13, 14,
        14, 15, 12
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 20 * 2 * sizeof(float), positions, GL_DYNAMIC_DRAW); // change to dynamic when moving

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    unsigned int backgroundibo;
    glGenBuffers(1, &backgroundibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(unsigned int), background, GL_STATIC_DRAW); // change to dynamic when moving

    unsigned int ballibo;
    glGenBuffers(1, &ballibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 3 * sizeof(unsigned int), ball, GL_STATIC_DRAW); // change to dynamic when moving

    unsigned int player1ibo;
    glGenBuffers(1, &player1ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player1ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(unsigned int), player1, GL_STATIC_DRAW); // change to dynamic when moving

    unsigned int player2ibo;
    GLCall(glGenBuffers(1, &player2ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player2ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(unsigned int), player2, GL_STATIC_DRAW)); // change to dynamic when moving

    ShaderProgramSource source = ParseShader("res/shaders/Vertex.shader", "res/shaders/Fragment.shader"); // get shaders from files

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource); // compile shaders
    glUseProgram(shader);

    int location = glGetUniformLocation(shader, "u_Color");
    ASSERT(location != -1); //location of -1 means it couldn't be found
    SetUniformColor(location, 0, 0, 0); // init to black
    //glUniform4f(location, (static_cast<GLfloat>(0) / 255), (static_cast<GLfloat>(29) / 255), (static_cast<GLfloat>(102) / 255), 1.0f); // 0, 29, 102 or #001d66

    // unbind everything
    //glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glfwSetKeyCallback(window, key_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Render here
        glClear(GL_COLOR_BUFFER_BIT);

        // moving player
        for (int i = 1; i < 8; i += 2) {
            positions[i] += vert;
        }

        // preventing player from going offscreen
        while (positions[5] > 1.0f) {
            for (int i = 1; i < 8; i += 2) {
                positions[i] -= 0.01f;
            }
        }
        while (positions[1] < -1.0f) {
            for (int i = 1; i < 8; i += 2) {
                positions[i] += 0.01f;
            }
        }

        // moving bot
        if (((((positions[25] + positions[29]) / 2) > positions[9]) && positions[8] > 0) && ((ballAngle < (pi / 2)) || (ballAngle > (3 * pi / 2)))) {
            for (int i = 25; i < 32; i += 2) {
                positions[i] -= 0.01f;
            }
        }

        if (((((positions[25] + positions[29]) / 2) < positions[9]) && positions[8] > 0) && ((ballAngle < (pi / 2)) || (ballAngle > (3 * pi / 2)))) {
            for (int i = 25; i < 32; i += 2) {
                positions[i] += 0.01f;
            }
        }

        // clamp player
        while (positions[5] > 1.0f) {
            for (int i = 1; i < 8; i += 2) {
                positions[i] -= 0.01f;
            }
        }
        while (positions[1] < -1.0f) {
            for (int i = 1; i < 8; i += 2) {
                positions[i] += 0.01f;
            }
        }

        //clamp bot
        while (positions[29] > 1.0f) {
            for (int i = 25; i < 32; i += 2) {
                positions[i] -= 0.01f;
            }
        }
        while (positions[25] < -1.0f) {
            for (int i = 25; i < 32; i += 2) {
                positions[i] += 0.01f;
            }
        }

        // ball collision
        // check top and bottom
        collision = false;
        for (int i = 9; i < 24; i += 2) {
            if (positions[i] > 1.0f || positions[i] < -1.0f) {
                collision = true;
            }
        }
        if (collision) {
            ballAngle = (float)(2 * pi) - ballAngle;
            ballAngle += (rand() / (32767 / variance)) - (variance / 2);
        }

        collision = false;

        //clamp angle
        if (ballAngle > 2 * pi)
            ballAngle -= (float)(2 * pi);
        if (ballAngle < 0)
            ballAngle += (float)(2 * pi);

        // check paddle
        if ((Player1Collision() && (ballAngle > (pi / 2)) && (ballAngle < (3 * pi / 2))) || (Player2Collision() && ((ballAngle < (pi / 2)) || (ballAngle > (3 * pi / 2))))) {
            ballAngle = (float) pi - ballAngle;
            ballAngle += (rand() / (32767 / variance)) - (variance / 2);
            ballSpeed += speedInc;
        }

        // clamp angle
        if (ballAngle > 2 * pi)
            ballAngle -= (float)(2 * pi);
        if (ballAngle < 0)
            ballAngle += (float)(2 * pi);

        //check oob
        collision = false;
        for (int i = 8; i < 23; i += 2) {
            if (positions[i] > 1.0f || positions[i] < -1.0f) {
                timer = 0;
            }
        }

        // reset if oob
        if (timer == 0) {
            for (int i = 0; i < 32; i++) {
                positions[i] = start[i];
            }
            ballAngle = (float) (((rand() + 16383.5) * pi) / 32767);
            ballSpeed = 0.005f;
        }

        // start
        if (timer > 100) {
            for (int i = 8; i < 23; i += 2) {
                positions[i] += cos(ballAngle) * ballSpeed;
            }
            for (int i = 9; i < 24; i += 2) {
                positions[i] += sin(ballAngle) * ballSpeed;
            }
        }

        //glUseProgram(shader);
        //glUniform4f(location, (static_cast<GLfloat>(0) / 255), (static_cast<GLfloat>(29) / 255), (static_cast<GLfloat>(102) / 255), 1.0f); // 0, 29, 102 or #001d66

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, 20 * 2 * sizeof(float), positions, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundibo);
        SetUniformColor(location, 0, 29, 102);

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballibo);
        SetUniformColor(location, 255, 255, 255);

        GLCall(glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, nullptr));


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player1ibo);
        SetUniformColor(location, 0, 140, 255);

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player2ibo);

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();

        timer++;
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}