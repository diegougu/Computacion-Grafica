//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 1500;

// =========================================================================
// 1. VERTEX SHADER ACTUALIZADO CON MATRIZ DE TRANSFORMACIÓN
// =========================================================================
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform;\n" // Recibe la matriz de transformacion
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(aPos, 1.0);\n" // Multiplica la matriz por la posicion
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n" // Variable que controlaremos desde C++
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";

// =========================================================================
// 2. CLASE TRANSFORMADOR2D (Puesta directamente en el main.cpp)
// =========================================================================

class Transformador2D {
private:
    unsigned int shaderID;
    int transformLoc;

    void enviarAlShader(const float* matriz) {
        glUseProgram(shaderID);
        // GL_TRUE transpone la matriz para coincidir con la notación de las diapositivas
        glUniformMatrix4fv(transformLoc, 1, GL_TRUE, matriz);
    }

public:
    Transformador2D(unsigned int shaderProgram) {
        shaderID = shaderProgram;
        transformLoc = glGetUniformLocation(shaderID, "transform");
    }

    // Traslación (Mover en X, Y)
    void trasladar(float tx, float ty) {
        float matriz[16] = {
            1.0f, 0.0f, 0.0f, tx,
            0.0f, 1.0f, 0.0f, ty,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        enviarAlShader(matriz);
    }

    // Escalado (Aumentar / Disminuir)
    void escalar(float sx, float sy) {
        float matriz[16] = {
            sx,   0.0f, 0.0f, 0.0f,
            0.0f, sy,   0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        enviarAlShader(matriz);
    }

    // Rotación en grados
    void rotar(float grados) {
        float radianes = grados * 3.14159265f / 180.0f;
        float c = cosf(radianes);
        float s = sinf(radianes);
        float matriz[16] = {
             c,   -s,   0.0f, 0.0f,
             s,    c,   0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        enviarAlShader(matriz);
    }

    // Vuelve el objeto a su estado normal (Sin transformación)
    void normal() {
        float matriz[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        enviarAlShader(matriz);
    }
	
	
	// Aplica Traslación, Rotación y Escalado todo al mismo tiempo
    void aplicarTodo(float tx, float ty, float sx, float sy, float grados) {
        float radianes = grados * 3.14159265f / 180.0f;
        float c = cosf(radianes);
        float s = sinf(radianes);
        
        // Esta matriz combina matemáticamente (Traslación * Rotación * Escala)
        float matriz[16] = {
            c * sx, -s * sy, 0.0f, tx,
            s * sx,  c * sy, 0.0f, ty,
            0.0f,    0.0f,   1.0f, 0.0f,
            0.0f,    0.0f,   0.0f, 1.0f
        };
        enviarAlShader(matriz);
    }
	
	
};

int main()
{
    // =========================================================================
    // AQUÍ PEDIMOS EL NÚMERO DE PEDAZOS ANTES DE INICIAR LA VENTANA
    // =========================================================================
    int pedazos;
    cout << "--- CREADOR DE PIZZAS OPENGL ---" << endl;
    cout << "¿En cuantos pedazos quieres cortar la pizza?: ";
    cin >> pedazos;
    
    // Evitar que pongan 0 o números negativos y el programa falle
    if (pedazos < 1) pedazos = 1;


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
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // =========================================================================
    // 3. INSTANCIAMOS NUESTRO TRANSFORMADOR
    // =========================================================================
    Transformador2D transformador(shaderProgram);
	

    // Generar datos de vértices (usando std::vector)
    int numerodesegment = 100;
    float radio = 0.20f;
    vector<float> verticesdecirculo;
    for(int i = 0; i < numerodesegment; i++) {
        float angulo = 2.0f * 3.1415926f * float(i) / float(numerodesegment);
        float x = radio * cosf(angulo);
        float y = radio * sinf(angulo);
        verticesdecirculo.push_back(x);
        verticesdecirculo.push_back(y);
        verticesdecirculo.push_back(0.0f);
    }
    
    float radio2 = 0.15f;
    vector<float> verticescirculopeque;
    for(int i = 0; i < numerodesegment; i++) {
        float angulo = 2.0f * 3.1415926f * float(i) / float(numerodesegment);
        float x = radio2 * cosf(angulo);
        float y = radio2 * sinf(angulo);
        verticescirculopeque.push_back(x);
        verticescirculopeque.push_back(y);
        verticescirculopeque.push_back(0.0f);
    }
    
    // NOTA: Ya no declaramos 'int pedazos = 8;' aquí, porque lo pedimos arriba.
    float radioCorte = 0.20; 
    vector<float> pizzapartes;

    for(int i = 0; i < pedazos; i++) {
        float angulo = 2.0f * 3.1415926f * float(i) / float(pedazos);
        pizzapartes.push_back(0.0f);
        pizzapartes.push_back(0.0f);
        pizzapartes.push_back(0.0f);

        float x = radioCorte * cosf(angulo);
        float y = radioCorte * sinf(angulo);
        pizzapartes.push_back(x);
        pizzapartes.push_back(y);
        pizzapartes.push_back(0.0f);
    }
	
	
	
	
	//CASAAAA
		
// 5 unique vertices
	float vertices[] = {
		// Square base
		-0.8f, -0.5f, 0.0f, // 0: bottom-left
		-0.4f, -0.5f, 0.0f, // 1: bottom-right
		-0.8f,  0.5f, 0.0f, // 2: top-right
		-0.4f,  0.5f, 0.0f, // 3: top-left
		// Triangle top
		-0.6f,  1.0f, 0.0f,  // 4: peak
		//Square door
		-0.6f, -0.5f, 0.0f, //5
		-0.5f, -0.5f, 0.0f, //6
		-0.6f, 0.1f, 0.0f, //7
		-0.5f, 0.1f, 0.0f, ///8
		
		//first window
		-0.7f, 0.2f, 0.0f, //9
		-0.6f, 0.2f, 0.0f, //10
		-0.7f, 0.3f, 0.0f, //11
		-0.6f, 0.3f, 0.0f, //12
		//second window
		-0.5f, 0.2f, 0.0f, //13
		-0.4f, 0.2f, 0.0f, //14
		-0.5f, 0.3f, 0.0f, //15
		-0.4f, 0.3f, 0.0f //16
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
		0, 1, 3, 2, 
		// Contorno del techo 
		3, 2, 4,    
		// Contorno de la puerta 
		5, 6, 8, 7,  
		//Contorno de la ventana 1
		10, 9, 11, 12,
		//Contorno de la ventana 2
		13, 14, 16, 15
		
	};
	
	
	float vertices_star[] = {
		// Triángulo 1 (Apunta hacia arriba)
		 0.0f,  0.6f, 0.0f, // 0
		-0.6f, -0.4f, 0.0f, // 1
		 0.6f, -0.4f, 0.0f, // 2

		// Triángulo 2 (Apunta hacia abajo)
		 0.0f, -0.6f, 0.0f, // 3 
		-0.6f,  0.4f, 0.0f, // 4
		 0.6f,  0.4f, 0.0f,  // 5 
		 
		//para las lineas arriba
		-0.14f, 0.4f, 0.0f, //6
		0.14f, 0.4f, 0.0f, //7
		//las lineas abajo
		-0.14f, -0.4f, 0.0f, //8
		0.14f, -0.4f, 0.0f, //9
		//para lineas derecha y izquierda
		0.36f, -0.03f, 0.0f, //10
		-0.36f, -0.03f, 0.0f //11
	};

	unsigned int indices_star[] = {
		0, 1, 2, // Índices del triángulo 1
		3, 4, 5,		// Índices del triángulo 2
		
		4, 6, 0, 7, 5, 10, 2, 9, 3, 8, 1, 11 
	};
	
    unsigned int VBO_star, VAO_star, EBO_star;
	
    glGenVertexArrays(1, &VAO_star);
    glGenBuffers(1, &VBO_star);
    glGenBuffers(1, &EBO_star);
	glBindVertexArray(VAO_star);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_star);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_star), vertices_star, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_star);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_star), indices_star, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

	
	
	
	
	
	
	
    
    // VAO / VBO 1 (Masa exterior)
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesdecirculo.size() * sizeof(float), verticesdecirculo.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    // VAO / VBO 2 (Queso interior)
    unsigned int VBO2, VAO2;
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, verticescirculopeque.size() * sizeof(float), verticescirculopeque.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    // VAO / VBO Pizza (Cortes)
    unsigned int VBO_pizza, VAO_pizza;
    glGenVertexArrays(1, &VAO_pizza);
    glGenBuffers(1, &VBO_pizza);
    glBindVertexArray(VAO_pizza);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pizza);
    glBufferData(GL_ARRAY_BUFFER, pizzapartes.size() * sizeof(float), pizzapartes.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);



	unsigned int VBO_CASA, VAO_CASA, EBO_CASA;
		
    glGenVertexArrays(1, &VAO_CASA);
    glGenBuffers(1, &VBO_CASA);
    glGenBuffers(1, &EBO_CASA);
	
	glBindVertexArray(VAO_CASA);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_CASA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_CASA);
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



    glPointSize(10.0f);
    glLineWidth(5.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");

        // =========================================================================
        // 4. APLICAMOS EL EFECTO DE RESPIRACIÓN A TODO LO QUE VIENE A CONTINUACIÓN
        // =========================================================================
        float tiempo = (float)glfwGetTime();
        float factorEscala = 1.0f + 0.15f * sinf(tiempo * 3.0f); // Escala entre 0.85 y 1.15
        
        // --- OPCIÓN 2: TRASLADAR (Mover de un punto A a un punto B) ---
        // Explicación: sinf(tiempo) genera valores fluidos que suben hasta 1.0 y bajan hasta -1.0.
        // Al multiplicarlo por 0.5f, hacemos que la pizza viaje desde X = -0.5 (izquierda) 
        // hasta X = 0.5 (derecha) y regrese constantemente.
        /*
        float posicionX = sinf(tiempo) * 0.5f; 
        float posicionY = 0.0f; // Fijo en el centro vertical
        transformador.trasladar(posicionX, posicionY);
        */


        // --- OPCIÓN 3: ROTAR (Girar sobre su propio centro) ---
        // Explicación: El tiempo siempre aumenta. Si lo multiplicamos por 50, 
        // la figura girará 50 grados por cada segundo que pase de forma continua.
        /*
        float grados = tiempo * 50.0f;
        transformador.rotar(grados);
        
        
        
        
        // ==========================================
        PARA MOVER PARTES DE ALGO 
		// 1. DIBUJAR EL CÍRCULO (Solo escalado)
		// ==========================================

		// Le decimos al shader: "Todo lo que dibuje ahora, escálalo"
		transformador.escalar(factorEscala, factorEscala);

		// Dibujamos el círculo
		glBindVertexArray(VAO_Circulo);
		glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); // Color
		glDrawArrays(GL_TRIANGLE_FAN, 0, numSegmentos);


		// ==========================================
		// 2. DIBUJAR EL TRIÁNGULO (Solo trasladado)
		// ==========================================

		// Cambiamos la orden del shader: "Olvida el escalado anterior, ahora traslada"
		transformador.trasladar(posicionX, posicionY);

		// Dibujamos el triángulo
		glBindVertexArray(VAO_Triangulo);
		glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); // Otro color
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
		
		
		//3 TODO A LA VES
		// Calculamos las animaciones
        float tiempo = (float)glfwGetTime();
        float factorEscala = 1.0f + 0.15f * sinf(tiempo * 3.0f); // Efecto respiración
        float grados = tiempo * 50.0f; // Rotación continua

        // =====================================================================
        // DIBUJAR LA ESTRELLA (A la derecha, rotando y respirando)
        // =====================================================================
        
        // aplicarTodo( X, Y, EscalaX, EscalaY, Grados )
        transformador.aplicarTodo(0.6f, 0.0f, factorEscala, factorEscala, grados);
                
        glBindVertexArray(VAO_star); 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
        
        glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);  
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int))); 
        
        // (Y aquí sigues dibujando las líneas y puntos de tu estrella...)
		
        */
        
		
		
		
		
		float posicionX = sinf(tiempo) * 0.5f; 
        float posicionY = 0.0f; // Fijo en el centro vertical
		//float posicionY = cosf(tiempo) * 0.5f; SE MUEVE EN VERTICAL TMB
		
		
		//ESTO ES PARA QUE SOLAMENTE SE MUEVA EN Y NADA MAS 
		//float posicionY = sinf(tiempo) * 0.5f; 
		//transformador.trasladar(0.0f, posicionY); // X = 0.0 (centro), Y = se mueve
		
        transformador.trasladar(posicionX, posicionY); //CAMBIAR DE POSICION PERO YA CON ANIMACION PS BABOSO
		//transformador.trasladar(1.5f, 0.0f);		//CAMBIAR DE POSICION SIN NECESADI DE ROTAR
		
		
		
		
		glBindVertexArray(VAO_CASA); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //siempre poner esto
		
		glUniform4f(vertexColorLocation, 0.9f, 0.8f, 0.7f, 1.0f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  //casa base
		
		glUniform4f(vertexColorLocation, 0.8f, 0.2f, 0.2f, 1.0f);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int))); //techo
		
		glUniform4f(vertexColorLocation, 0.4f, 0.2f, 0.1f, 1.0f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(9 * sizeof(unsigned int))); //puerta
		
		glUniform4f(vertexColorLocation, 0.0f, 0.7f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void*)(15 * sizeof(unsigned int))); //ventana 1 y ventana 2







		
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
		
		
		
		float grados = tiempo * 50.0f;
        transformador.rotar(grados);
		        
		glBindVertexArray(VAO_star); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //siempre poner esto
		
		glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);  //star
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int))); 

		//lineas
		// Contorno Base 
		glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); // Negro para todas las líneas
		glDrawElements(GL_LINE_LOOP, 12, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int))); //techo
		
		//puntos
		glUniform4f(vertexColorLocation, 0.1f, 0.3f, 1.0f, 1.0f);		
		glDrawArrays(GL_POINTS, 0, 6);
		
		
		
		
		
		
        
        // ¡Una sola línea para activar el escalado animado!
        transformador.escalar(factorEscala, factorEscala);

        // --- DIBUJAR MASA DE LA PIZZA ---
        glBindVertexArray(VAO); 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform4f(vertexColorLocation, 0.54f, 0.27f, 0.07f, 1.0f); // Marrón
        glDrawArrays(GL_TRIANGLE_FAN, 0, numerodesegment);

        glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); // Borde negro
        glDrawArrays(GL_LINE_LOOP, 0, numerodesegment);

        // --- DIBUJAR QUESO ---
        glBindVertexArray(VAO2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); // Amarillo
        glDrawArrays(GL_TRIANGLE_FAN, 0, numerodesegment);

        glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); // Borde negro
        glDrawArrays(GL_LINE_LOOP, 0, numerodesegment);

        // --- DIBUJAR CORTES DE PIZZA ---
        glBindVertexArray(VAO_pizza); 
        glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); 
        glDrawArrays(GL_LINES, 0, pedazos * 2);

        // Si después quisieras dibujar un objeto estático (que no respire):
        // transformador.normal();
        // glBindVertexArray(VAO_de_otro_objeto);
        // glDrawArrays(...);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteVertexArrays(1, &VAO_pizza);
    glDeleteBuffers(1, &VBO_pizza);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}