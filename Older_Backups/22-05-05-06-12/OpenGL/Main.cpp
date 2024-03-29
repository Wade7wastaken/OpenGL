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
        char* message = (char*)alloca(length * sizeof(char));
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
        return (size * cos(2 * pi * index / sides));
    }
    else {
        return (size * sin(2 * pi * index / sides) * 16 / 9);
    }
}

const float size = 0.03f; // size of the ball

float positions[] = {
    -0.95f, -0.20f, // player 1
    -0.90f, -0.20f,
    -0.90f,  0.20f,
    -0.95f,  0.20f,
     FindPoints(size, 8, 0, true), FindPoints(size, 8, 0, false), // 4 08, 09 ball
     FindPoints(size, 8, 1, true), FindPoints(size, 8, 1, false), // 5 10, 11
     FindPoints(size, 8, 2, true), FindPoints(size, 8, 2, false), // 6 12, 13
     FindPoints(size, 8, 3, true), FindPoints(size, 8, 3, false), // 7 14, 15
     FindPoints(size, 8, 4, true), FindPoints(size, 8, 4, false), // 8 16, 17
     FindPoints(size, 8, 5, true), FindPoints(size, 8, 5, false), // 9 18, 19
     FindPoints(size, 8, 6, true), FindPoints(size, 8, 6, false), // 10 20, 21
     FindPoints(size, 8, 7, true), FindPoints(size, 8, 7, false), // 11 22, 23
     0.95f, -0.20f, // player 2
     0.90f, -0.20f,
     0.90f,  0.20f,
     0.95f,  0.20f
};

float start[] = { // a backup to restore to when a game is lost or won
    -0.95f, -0.20f, // player 1
    -0.90f, -0.20f,
    -0.90f,  0.20f,
    -0.95f,  0.20f,
     FindPoints(size, 8, 0, true), FindPoints(size, 8, 0, false), // 4 08, 09 ball
     FindPoints(size, 8, 1, true), FindPoints(size, 8, 1, false), // 5 10, 11
     FindPoints(size, 8, 2, true), FindPoints(size, 8, 2, false), // 6 12, 13
     FindPoints(size, 8, 3, true), FindPoints(size, 8, 3, false), // 7 14, 15
     FindPoints(size, 8, 4, true), FindPoints(size, 8, 4, false), // 8 16, 17
     FindPoints(size, 8, 5, true), FindPoints(size, 8, 5, false), // 9 18, 19
     FindPoints(size, 8, 6, true), FindPoints(size, 8, 6, false), // 10 20, 21
     FindPoints(size, 8, 7, true), FindPoints(size, 8, 7, false), // 11 22, 23
     0.95f, -0.20f, // player 2
     0.90f, -0.20f,
     0.90f,  0.20f,
     0.95f,  0.20f
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

    const float speedInc = 0.001f;
    const float variance = 0.2; // ammount of angle variance during a bounce
    float ballAngle = (((rand() + 16383.5) * pi) / 32767);
    float ballSpeed = 0.005f;
    bool collision = false;

    unsigned int timer = 0;

    unsigned int player1[] = { // must be unsigned
        0, 1, 2,
        2, 3, 0
    };

    unsigned int ball[] = {
        4, 6, 5,
        4, 7, 6,
        4, 8, 7,
        4, 9, 8,
        4, 10, 9,
        4, 11, 10
    };

    unsigned int player2[] = {
        12, 13, 14,
        14, 15, 12
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 16 * 2 * sizeof(float), positions, GL_DYNAMIC_DRAW); // change to dynamic when moving

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

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
    glUniform4f(location, 1.0f, 1.0f, 1.0f, 1.0f);

    // unbind everything
    glUseProgram(0);
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
            ballAngle = (2 * pi) - ballAngle;
            ballAngle += (rand() / (32767 / variance)) - (variance / 2);
        }

        collision = false;

        //clamp angle
        if (ballAngle > 2 * pi)
            ballAngle -= (2 * pi);
        if (ballAngle < 0)
            ballAngle += (2 * pi);

        // check paddle
        if ((Player1Collision() && (ballAngle > (pi / 2) && ballAngle) < (3 * pi / 2)) || (Player2Collision() && (ballAngle < (pi / 2) || ballAngle >(3 * pi / 2)))) {
            ballAngle = pi - ballAngle;
            ballAngle += (rand() / (32767 / variance)) - (variance / 2);
            ballSpeed += speedInc;
        }

        // clamp angle
        if (ballAngle > 2 * pi)
            ballAngle -= (2 * pi);
        if (ballAngle < 0)
            ballAngle += (2 * pi);

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
            ballAngle = (((rand() + 16383.5) * pi) / 32767);
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

        glUseProgram(shader);
        glUniform4f(location, 1.0f, 1.0f, 1.0f, 1.0f);

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, 16 * 2 * sizeof(float), positions, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballibo);

        GLCall(glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, nullptr));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player1ibo);
        glUniform4f(location, 0.0f, 0.3f, 0.8f, 1.0f);

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

/*std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    //std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }*/