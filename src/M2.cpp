/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
float verifyValue(float value);
// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 3840, HEIGHT = 2160;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
//...pode ter mais linhas de código aqui!
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false, translateXPlus=false, translateXMinus=false, translateYPlus=false, translateYMinus=false, translateZPlus=false, translateZMinus=false, scalePlus=false, scaleMinus=false;
float scaleFactor = 1.0f;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Rossana!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Gerando um buffer simples, com a geometria de um triângulo
	GLuint VAO = setupGeometry();


	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

    float x = 0.0f, y = 0.0f, z = 0.0f;
    float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		if (translateXPlus) x += 0.01f;
		if (translateXMinus) x -= 0.01f;
		if (translateYPlus) y += 0.01f;
		if (translateYMinus) y -= 0.01f;
        if (translateZPlus) z += 0.01f;
        if (translateZMinus) z -= 0.01f;
        if (scalePlus) scaleFactor += 0.01f;
        if (scaleMinus) scaleFactor -= 0.01f;

		x = verifyValue(x);
		y = verifyValue(y);
		z = verifyValue(z);

		if (rotateX) angleX += 0.01f;
		if (rotateY) angleY += 0.01f;
		if (rotateZ) angleZ += 0.01f;

		scaleFactor = glm::clamp(scaleFactor, 0.1f, 3.0f);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(x, y, z));
		model = glm::rotate(model, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(scaleFactor));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Chamada de desenho - drawcall
		// CONTORNO - GL_LINE_LOOP
		
		glDrawArrays(GL_POINTS, 0, 36);

		// cubo 2


        glm::mat4 model2 = glm::mat4(1);
		model2 = glm::translate(model2, glm::vec3(-x, -y, -z));
		model2 = glm::rotate(model2, -angleX, glm::vec3(1.0f, 0.0f, 0.0f));
		model2 = glm::rotate(model2, -angleY, glm::vec3(0.0f, 1.0f, 0.0f));
		model2 = glm::rotate(model2, -angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
		model2 = glm::scale(model2, glm::vec3(scaleFactor));


		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS, 0, 36);

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
float verifyValue(float value) {
    if (value > 1.5f) {
        return -1.5f;
    } else if (value < -1.5f) {
        return 1.5f;
    }
    return value;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X)
	{
		if (action == GLFW_PRESS)
			rotateX = true;
		else if (action == GLFW_RELEASE)
			rotateX = false;
	}

	if (key == GLFW_KEY_Y)
	{
		if (action == GLFW_PRESS)
			rotateY = true;
		else if (action == GLFW_RELEASE)
			rotateY = false;
	}

	if (key == GLFW_KEY_Z)
	{
		if (action == GLFW_PRESS)
			rotateZ = true;
		else if (action == GLFW_RELEASE)
			rotateZ = false;
	}
    


    if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS)
			translateYPlus = true;
		else if (action == GLFW_RELEASE)
			translateYPlus = false;
	}

    if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			translateYMinus = true;
		else if (action == GLFW_RELEASE)
			translateYMinus = false;
	}
    
    if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
			translateXMinus = true;
		else if (action == GLFW_RELEASE)
			translateXMinus = false;
	}

    if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
			translateXPlus = true;
		else if (action == GLFW_RELEASE)
			translateXPlus = false;
	}

    if (key == GLFW_KEY_I)
	{
		if (action == GLFW_PRESS)
			translateZPlus = true;
		else if (action == GLFW_RELEASE)
			translateZPlus = false;
	}

    if (key == GLFW_KEY_J)
	{
		if (action == GLFW_PRESS)
			translateZMinus = true;
		else if (action == GLFW_RELEASE)
			translateZMinus = false;
	}

    if (key == GLFW_KEY_RIGHT_BRACKET)
	{
		if (action == GLFW_PRESS)
			scalePlus = true;
		else if (action == GLFW_RELEASE)
			scalePlus = false;
	}

        if (key == GLFW_KEY_LEFT_BRACKET)
	{
		if (action == GLFW_PRESS)
			scaleMinus = true;
		else if (action == GLFW_RELEASE)
			scaleMinus = false;
	}


    // 1 V
// Alterar a geometria da pirâmide, transformando-a em um cubo (adicionar os vértices e triângulos necessários). 
//Sugere-se fazer cada lado do cubo (composto de 2 triângulos, similar à base da pirâmide) de uma cor diferente, para que facilite nossa visualização neste momento que ainda não utilizamos texturas e iluminação adequada.

   
// No projeto de base, ao pressionar as teclas x, y e z, a pirâmide rotaciona nos respectivos eixos. Adicione controle via teclado para:

    // 2 V
// Mover (transladar) o cubo nos 3 eixos (sugestão de teclas WASD para os eixos x e z, IJ para o eixo y)

    // 3 V
// Promover a escala uniforme do cubo (sugestão de teclas [ para diminuir e ] para aumentar)

    // 4
// Instanciar mais de um cubo na cena

}

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {

		// Baixo
		-0.5f, -0.5f, -0.5f, 0.90f, 0.58f, 0.47f,
		-0.5f, -0.5f,  0.5f, 0.84f, 0.40f, 0.63f,
		 0.5f, -0.5f,  0.5f, 0.96f, 0.73f, 0.55f,

		 0.5f, -0.5f,  0.5f, 0.95f, 0.56f, 0.62f,
		 0.5f, -0.5f, -0.5f, 0.88f, 0.47f, 0.52f,
		-0.5f, -0.5f, -0.5f, 0.79f, 0.34f, 0.44f,

		// Cima
		-0.5f,  0.5f, -0.5f, 0.60f, 0.90f, 0.78f,
		 0.5f,  0.5f, -0.5f, 0.52f, 0.84f, 0.73f,
		 0.5f,  0.5f,  0.5f, 0.70f, 0.94f, 0.82f,

		 0.5f,  0.5f,  0.5f, 0.62f, 0.88f, 0.79f,
		-0.5f,  0.5f,  0.5f, 0.54f, 0.80f, 0.70f,
		-0.5f,  0.5f, -0.5f, 0.47f, 0.75f, 0.66f,

		// Frente
		-0.5f, -0.5f,  0.5f, 0.40f, 0.70f, 0.95f,
		 0.5f, -0.5f,  0.5f, 0.30f, 0.55f, 0.88f,
		 0.5f,  0.5f,  0.5f, 0.58f, 0.82f, 0.98f,

		 0.5f,  0.5f,  0.5f, 0.43f, 0.67f, 0.91f,
		-0.5f,  0.5f,  0.5f, 0.36f, 0.58f, 0.85f,
		-0.5f, -0.5f,  0.5f, 0.22f, 0.45f, 0.76f,

		// Tras
		-0.5f, -0.5f, -0.5f, 0.94f, 0.74f, 0.42f,
		 0.5f, -0.5f, -0.5f, 0.85f, 0.69f, 0.31f,
		 0.5f,  0.5f, -0.5f, 0.88f, 0.79f, 0.53f,

		 0.5f,  0.5f, -0.5f, 0.80f, 0.71f, 0.46f,
		-0.5f,  0.5f, -0.5f, 0.74f, 0.64f, 0.38f,
		-0.5f, -0.5f, -0.5f, 0.68f, 0.56f, 0.32f,

		// Esquerda
		-0.5f, -0.5f, -0.5f, 0.78f, 0.68f, 0.90f,
		-0.5f,  0.5f, -0.5f, 0.64f, 0.54f, 0.86f,
		-0.5f,  0.5f,  0.5f, 0.55f, 0.45f, 0.79f,

		-0.5f,  0.5f,  0.5f, 0.68f, 0.58f, 0.92f,
		-0.5f, -0.5f,  0.5f, 0.52f, 0.41f, 0.73f,
		-0.5f, -0.5f, -0.5f, 0.44f, 0.33f, 0.65f,

		// Direita
		 0.5f, -0.5f, -0.5f, 0.98f, 0.72f, 0.58f,
		 0.5f,  0.5f, -0.5f, 0.92f, 0.65f, 0.54f,
		 0.5f,  0.5f,  0.5f, 0.93f, 0.77f, 0.68f,

		 0.5f,  0.5f,  0.5f, 0.85f, 0.61f, 0.48f,
		 0.5f, -0.5f,  0.5f, 0.87f, 0.74f, 0.56f,
		 0.5f, -0.5f, -0.5f, 0.79f, 0.63f, 0.49f,

	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

 