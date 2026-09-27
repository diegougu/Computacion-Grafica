//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n" // Variable que controlaremos desde C++
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
	

	
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
	

	
	
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
	
	
// 5 unique vertices
	float vertices[] = {
		// Square base
		-0.5f, -0.5f, 0.0f, // 0: bottom-left
		0.5f, -0.5f, 0.0f, // 1: bottom-right
		0.5f,  0.5f, 0.0f, // 2: top-right
		-0.5f,  0.5f, 0.0f, // 3: top-left
		// Triangle top
		0.0f,  1.0f, 0.0f,  // 4: peak
		//Square door
		-0.2f, -0.5f, 0.0f, //5
		0.2f, -0.5f, 0.0f, //6
		-0.2f, 0.1f, 0.0f, //7
		0.2f, 0.1f, 0.0f, ///8
		
		//first window
		-0.3f, 0.2f, 0.0f, //9
		-0.4f, 0.2f, 0.0f, //10
		-0.3f, 0.3f, 0.0f, //11
		-0.4f, 0.3f, 0.0f, //12
		//second window
		0.3f, 0.2f, 0.0f, //13
		0.4f, 0.2f, 0.0f, //14
		0.3f, 0.3f, 0.0f, //15
		0.4f, 0.3f, 0.0f, //16
		
		
		//cruz de la ventana 1
		-0.35f, 0.2f, 0.0f, //17
		-0.35, 0.3f, 0.0f, //18
		-0.4f, 0.25f, 0.0f, //19
		-0.3f, 0.25f, 0.0f, //20
		//cruz de la ventana 2
		0.35f, 0.2f, 0.0f, //21
		0.35, 0.3f, 0.0f, //22
		0.4f, 0.25f, 0.0f, //23
		0.3f, 0.25f, 0.0f //24
	};

//cada indices osea eso de 0, 1 , 2 ... se refiere a las posiciones de los vertices osea (x,y,z) 0 , de ahi van de 3 en 3 sabes?

	unsigned int indices[] = {
		// --- ÍNDICES PARA EL RELLENO (Triángulos) ---
		// Base de la casa (6 índices)
		0, 1, 2,
		0, 2, 3,
		// Techo (3 índices)
		3, 2, 4,
		// Puerta (6 índices)
		5, 6, 8,
		5, 8, 7,
		//ventana 1 (6 indices)
		10, 9, 11,
		10, 11, 12,
		//ventana 2 (6 indices)
		13, 14, 16,
		13, 16, 15,
		
		
		// --- ÍNDICES PARA LAS LÍNEAS EXTERIORES (GL_LINE_LOOP) ---
		// Contorno de la base 
		0, 1, 2, 3, 
		// Contorno del techo 
		3, 2, 4,    
		// Contorno de la puerta 
		5, 6, 8, 7,  
		//Contorno de la ventana 1
		10, 9, 11, 12,
		//Contorno de la ventana 2
		13, 14, 16, 15,
		
		//cruz ventana 1
		17,18,
		19, 20,
		//cruz ventana 2
		21, 22,
		23, 24
		
	};
	
    unsigned int VBO, VAO, EBO;
	
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
	
	

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	
	
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glPointSize(15.0f); //GROSOR PARA LOS PUNTOS 
	glLineWidth(3.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		
		
		
		
		int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor"); //para tener cualquier color sin necesidad de hacer toda la vaina
		
		//relleno
		// la funcion es tipo (GL_TRIANGLES, INDICES osea que componen la figura como el fin, GL_UNSIGNED_INT, inicio del array)
		//el GL triangles puede cambiar dependiendo del que 
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //siempre poner esto
		
		glUniform4f(vertexColorLocation, 0.9f, 0.8f, 0.7f, 1.0f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  //casa base
		
		glUniform4f(vertexColorLocation, 0.8f, 0.2f, 0.2f, 1.0f);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int))); //techo
		
		glUniform4f(vertexColorLocation, 0.4f, 0.2f, 0.1f, 1.0f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(9 * sizeof(unsigned int))); //puerta
		
		glUniform4f(vertexColorLocation, 0.0f, 0.7f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void*)(15 * sizeof(unsigned int))); //ventana 1 y ventana 2

		
		//lineas
		// Contorno Base 
		glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); // Negro para todas las líneas
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(27 * sizeof(unsigned int)));
        
        // Contorno Techo 
        glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(31 * sizeof(unsigned int)));
        
        // Contorno Puerta 
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(34 * sizeof(unsigned int)));
		//Contorno ventana 1 y 2
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(38 * sizeof(unsigned int)));
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(42 * sizeof(unsigned int)));
		
		
		glDrawElements(GL_LINE_LOOP, 2, GL_UNSIGNED_INT, (void*)(46 * sizeof(unsigned int)));
		glDrawElements(GL_LINE_LOOP, 2, GL_UNSIGNED_INT, (void*)(48 * sizeof(unsigned int)));
		glDrawElements(GL_LINE_LOOP, 2, GL_UNSIGNED_INT, (void*)(50 * sizeof(unsigned int)));
		glDrawElements(GL_LINE_LOOP, 2, GL_UNSIGNED_INT, (void*)(52 * sizeof(unsigned int)));

		
		//puntos
		glUniform4f(vertexColorLocation, 0.1f, 0.3f, 1.0f, 1.0f);
		glDrawElements(GL_POINTS, 27, GL_UNSIGNED_INT, 0);
		
		//glDrawArrays(GL_POINTS, 0, 25); // Dibuja los 25 vértices de tu arreglo 'vertices'
		
		 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}