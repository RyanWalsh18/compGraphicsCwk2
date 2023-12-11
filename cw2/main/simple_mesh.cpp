#include "simple_mesh.hpp"

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
	return aM;
}


GLuint create_vao( SimpleMeshData const& aMeshData )
{
	//TODO: implement me
	//set up vbo for 3d positions of vertexes
	GLuint positionVBO = 0; //vertex Buffer Object
	glGenBuffers(1, &positionVBO); //generate 1 name at position vbo
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO); // bind the buffer
	glBufferData(GL_ARRAY_BUFFER, aMeshData.positions.size() * sizeof(Vec3f), aMeshData.positions.data(), GL_STATIC_DRAW);

	// set up vbo for colours of each vertex
	GLuint colourVBO = 0; //vertex Buffer Object
	glGenBuffers(1, &colourVBO); //generate 1 name at position vbo
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO); // bind the buffer
	glBufferData(GL_ARRAY_BUFFER, aMeshData.colors.size() * sizeof(Vec3f) , aMeshData.colors.data(), GL_STATIC_DRAW);

	// set up vbo for normals of each vertex
	GLuint normalVBO = 0; //vertex Buffer Object
	glGenBuffers(1, &normalVBO); //generate 1 name at position vbo
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO); // bind the buffer
	glBufferData(GL_ARRAY_BUFFER, aMeshData.normals.size() * sizeof(Vec3f) , aMeshData.normals.data(), GL_STATIC_DRAW);

	// set up vbo for normals of each vertex
	GLuint textureVBO = 0; //vertex Buffer Object
	glGenBuffers(1, &textureVBO); //generate 1 name at position vbo
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO); // bind the buffer
	glBufferData(GL_ARRAY_BUFFER, aMeshData.texcoords.size() * sizeof(Vec3f) , aMeshData.texcoords.data(), GL_STATIC_DRAW);

	//create VAO
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Bind Positions VBO to VAO
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glVertexAttribPointer(
		0, //location is 0 in vertex shader
		3, GL_FLOAT, GL_FALSE, //3 floats, not normalised to [0..1] (GL_FALSE)
		0, //stride = 0 indicates no padding between inputs
		0 //data starts at offset 0 in the VBO
	);
	glEnableVertexAttribArray(0);

	//Bind Colors VBO to VAO
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glVertexAttribPointer(
		1, //location is 1 in vertex shader
		3, GL_FLOAT, GL_FALSE, //3 floats, not normalised to [0..1] (GL_FALSE)
		0, //stride = 0 indicates no padding between inputs
		0 //data starts at offset 0 in the VBO
	);
	glEnableVertexAttribArray(1);

		//Bind Positions VBO to VAO
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glVertexAttribPointer(
		2, //location is 0 in vertex shader
		3, GL_FLOAT, GL_FALSE, //3 floats, not normalised to [0..1] (GL_FALSE)
		0, //stride = 0 indicates no padding between inputs
		0 //data starts at offset 0 in the VBO
	);
	glEnableVertexAttribArray(2);

	//Bind Positions VBO to VAO
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	glVertexAttribPointer(
		3, //location is 0 in vertex shader
		2, GL_FLOAT, GL_FALSE, //3 floats, not normalised to [0..1] (GL_FALSE)
		sizeof(Vec2f), //stride = 0 indicates no padding between inputs
		nullptr //data starts at offset 0 in the VBO
	);
	glEnableVertexAttribArray(3);

	//reset State
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//clean up buffers
	//buffers not deleted fully as VAO has reference to them
	glDeleteBuffers(1, &positionVBO);
	glDeleteBuffers(1, &colourVBO);
	glDeleteBuffers(1, &normalVBO);
	glDeleteBuffers(1, &textureVBO);

	return vao;
}