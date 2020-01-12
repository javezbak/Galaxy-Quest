#include "Entity.h"
#include <ostream>
#include <SDL_image.h>
#include "ShaderProgram.h"

Entity::ImageData Entity::LoadImg(const std::string image, bool pixel)
{
	ImageData imgData;

	//My image 
	//Note to self, image has to be in the same folder as the rest of these files
	//Folder found by right clicking the project and opening explorer
	SDL_Surface *surface = IMG_Load(image.c_str());
	imgData.width = surface->w;
	imgData.height = surface->h;
	std::string error = IMG_GetError();
	if (error != "")
		std::cerr << error;

	//create a new texture id
	GLuint textureID;
	glGenTextures(1, &textureID);

	//bind a texture to a texture target
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Set the texture data of the specified texture target. Slight modifications depending on OS and image format
#if defined(_WINDOWS)
	if (surface->format->BytesPerPixel == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	else if (surface->format->BytesPerPixel == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
	else
		std::cerr << "Having trouble reading image, it will not work with this program";
#else
	if (surface->format->BytesPerPixel == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
	else if (surface->format->BytesPerPixel == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_BGR, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
	else
		cerr << "Having trouble reading image, it will not work with this program";
#endif	

	//Set a texture parameter of the specified texture target
	if (pixel)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	SDL_FreeSurface(surface);
	imgData.texture = textureID;
	return imgData;
}

//entities that come from single images
Entity::Entity(const ImageData imgData, const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_) :
texture(imgData.texture), position(position_), scale(scale_), velocity(velocity_), rotate(rotate_)
{
	vertices[0] = -0.5f;
	vertices[1] = -0.5f;
	vertices[2] = 0.5f;

	vertices[3] = -0.5f;
	vertices[4] = 0.5f;
	vertices[5] = 0.5f;

	vertices[6] = -0.5f;
	vertices[7] = -0.5f;
	vertices[8] = 0.5f;

	vertices[9] = 0.5f;
	vertices[10] = -0.5f;
	vertices[11] = 0.5f;

	textureCoordinates[0] = 0.0;
	textureCoordinates[1] = 1.0;
	textureCoordinates[2] = 1.0;
	textureCoordinates[3] = 1.0 + (1/270);
	textureCoordinates[4] = 1.0 + (1/84);
	textureCoordinates[5] = 0.0 + (1/270);
	textureCoordinates[6] = 0.0;
	textureCoordinates[7] = 1.0;
	textureCoordinates[8] = 1.0 + (1/270); 
	textureCoordinates[9] = 0.0 + (1/84);
	textureCoordinates[10] = 0.0 + (1/270);
	textureCoordinates[11] = 0.0;
}

//entities that have custom xml sheets
Entity::Entity(const std::string sheetName_, const std::string imgName_, const ImageData imgData, const std::string xml,
	const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_, const Vector3 acceleration_, const Vector3 friction_) :
	imgName(imgName_), sheetName(sheetName_), texture(imgData.texture),
	position(position_), scale(scale_), velocity(velocity_), rotate(rotate_), acceleration(acceleration_), friction(friction_)
{
	bool create = false;
	std::ifstream fin(xml);
	std::string line;
	while (std::getline(fin, line))
	{
		//Check if this line has the image
		if (line.find(imgName) != std::string::npos)
		{
			create = true;
			size_t lastIndexOfName = line.find(imgName) + imgName.length();

			//grab the parameters from the line
			//u
			size_t indexOfXValue = line.find("x=\"", lastIndexOfName) + 3;
			size_t indexOfEndOfXValue = line.find("\"", indexOfXValue);
			std::string xValue = line.substr(indexOfXValue, indexOfEndOfXValue - indexOfXValue);
			uvCoords.x = atoi(xValue.c_str()) / imgData.width;

			//v
			size_t indexOfYValue = line.find("y=\"", indexOfEndOfXValue) + 3;
			size_t indexOfEndOfYValue = line.find("\"", indexOfYValue);
			std::string YValue = line.substr(indexOfYValue, indexOfEndOfYValue - indexOfYValue);
			uvCoords.y = atoi(YValue.c_str()) / imgData.height;

			//width
			size_t indexOfWidth= line.find("width=\"", indexOfEndOfYValue) + 7;
			size_t indexOfEndOfWidth = line.find("\"", indexOfWidth);
			std::string widthValue = line.substr(indexOfWidth, indexOfEndOfWidth - indexOfWidth);
			dimensions.x = atoi(widthValue.c_str()) / imgData.width;

			//height
			size_t indexOfHeight = line.find("height=\"", indexOfEndOfWidth) + 8;
			size_t indexOfEndOfHeight = line.find("\"", indexOfHeight);
			std::string HeightValue = line.substr(indexOfHeight, indexOfEndOfHeight - indexOfHeight);
			dimensions.y = atoi(HeightValue.c_str()) / imgData.height;

			//vertex and texture coordinates
			textureCoordinates[0] = uvCoords.x;
			textureCoordinates[1] = uvCoords.y + dimensions.y;
			textureCoordinates[2] = uvCoords.x + dimensions.x;
			textureCoordinates[3] = uvCoords.y;
			textureCoordinates[4] = uvCoords.x;
			textureCoordinates[5] = uvCoords.y;
			textureCoordinates[6] = uvCoords.x + dimensions.x;
			textureCoordinates[7] = uvCoords.y;
			textureCoordinates[8] = uvCoords.x;
			textureCoordinates[9] = uvCoords.y + dimensions.y;
			textureCoordinates[10] = uvCoords.x + dimensions.x;
			textureCoordinates[11] = uvCoords.y + dimensions.y;

			float aspect = dimensions.x / dimensions.y;

			vertices[0] = -0.5f * aspect;
			vertices[1] = -0.5f;
			vertices[2] = 0.5f * aspect;
			vertices[3] = 0.5f; 
			vertices[4] = -0.5f * aspect;
			vertices[5] = 0.5f;
			vertices[6] = 0.5f * aspect;
			vertices[7] = 0.5f;
			vertices[8] = -0.5f * aspect;
			vertices[9] = -0.5f;
			vertices[10] = 0.5f * aspect;
			vertices[11] = -0.5f;

			break;
		}
	}

	//error checking
	if (create == false)
		std::cerr << "The image was not found in the spritesheet." << std::endl;
};

//entities that have unfiorm sheets and animation
Entity::Entity(const std::string sheetName_, int index, int spriteCountX, int spriteCountY, const ImageData imgData, float tileSize,
	const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_, const Vector3 acceleration_, const std::vector<int> frames) :
	sheetName(sheetName_), texture(imgData.texture),
	position(position_), scale(scale_), velocity(velocity_), rotate(rotate_), acceleration(acceleration_),
	indecesOfAnimation(frames)
{
	uvCoords.x = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	uvCoords.y = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	dimensions.x = 1.0 / (float)spriteCountX;
	dimensions.y = 1.0 / (float)spriteCountY;

	textureCoordinates[0] = uvCoords.x;
	textureCoordinates[1] = uvCoords.y + dimensions.y;
	textureCoordinates[2] = uvCoords.x + dimensions.x;
	textureCoordinates[3] = uvCoords.y;
	textureCoordinates[4] = uvCoords.x;
	textureCoordinates[5] = uvCoords.y;
	textureCoordinates[6] = uvCoords.x + dimensions.x;
	textureCoordinates[7] = uvCoords.y;
	textureCoordinates[8] = uvCoords.x;
	textureCoordinates[9] = uvCoords.y + dimensions.y;
	textureCoordinates[10] = uvCoords.x + dimensions.x;
	textureCoordinates[11] = uvCoords.y + dimensions.y;

	vertices[0] = -0.5f * tileSize;
	vertices[1] = -0.5f * tileSize;
	vertices[2] = 0.5f * tileSize;
	vertices[3] = 0.5f * tileSize; //bottom
	vertices[4] = -0.5f * tileSize;
	vertices[5] = 0.5f * tileSize;
	vertices[6] = 0.5f * tileSize;
	vertices[7] = 0.5f * tileSize;
	vertices[8] = -0.5f * tileSize;
	vertices[9] = -0.5f * tileSize;
	vertices[10] = 0.5f * tileSize;
	vertices[11] = -0.5f * tileSize;

	if (indecesOfAnimation.size() != 0)
		frame = indecesOfAnimation[0];

}

//entities that have uniform sheets
Entity::Entity(std::vector<float> vertices_, std::vector<float> textureCoords, const ImageData imgData,
	const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_) :
	texture(imgData.texture), position(position_), scale(scale_), velocity(velocity_), rotate(rotate_)
{
	textureCoordinates[0] = textureCoords[0];
	textureCoordinates[1] = textureCoords[1];
	textureCoordinates[2] = textureCoords[2];
	textureCoordinates[3] = textureCoords[3];
	textureCoordinates[4] = textureCoords[4];
	textureCoordinates[5] = textureCoords[5];
	textureCoordinates[6] = textureCoords[6];
	textureCoordinates[7] = textureCoords[7];
	textureCoordinates[8] = textureCoords[8];
	textureCoordinates[9] = textureCoords[9];
	textureCoordinates[10] = textureCoords[10];
	textureCoordinates[11] = textureCoords[11];

	vertices[0] = vertices_[0];
	vertices[1] = vertices_[1];
	vertices[2] = vertices_[2];
	vertices[3] = vertices_[3];
	vertices[4] = vertices_[4];
	vertices[5] = vertices_[5];
	vertices[6] = vertices_[6];
	vertices[7] = vertices_[7];
	vertices[8] = vertices_[8];
	vertices[9] = vertices_[9];
	vertices[10] = vertices_[10];
	vertices[11] = vertices_[11];
}

//untextured polygon entities
Entity::Entity(const Vector3 position_, const Vector3 scale_,
	const Vector3 velocity_, const Vector3 rotate_, bool triangle_) :
	position(position_), scale(scale_), velocity(velocity_), rotate(rotate_), triangle(triangle_)
{
	if (triangle_ == false)
	{
		vertices[0] = -0.5f;
		vertices[1] = -1.0f;
		vertices[2] = 0.5f;
		vertices[3] = 1.0f;
		vertices[4] = -0.5f;
		vertices[5] = 1.0f;
		vertices[6] = 0.5f;
		vertices[7] = 1.0f;
		vertices[8] = -0.5f;
		vertices[9] = -1.0f;
		vertices[10] = 0.5f;
		vertices[11] = -1.0f;
	}
	else 
	{
		verticesTri[0] = 0.5f;
		verticesTri[1] = -0.5f;
		verticesTri[2] = 0.0f;
		verticesTri[3] = 0.5f;
		verticesTri[4] = -0.5f;
		verticesTri[5] = -0.5f;
	}

};

void Entity::changeFrame(int index, int spriteCountX, int spriteCountY)
{
	uvCoords.x = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	uvCoords.y = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	dimensions.x = 1.0 / (float)spriteCountX;
	dimensions.y = 1.0 / (float)spriteCountY;

	textureCoordinates[0] = uvCoords.x;
	textureCoordinates[1] = uvCoords.y + dimensions.y;
	textureCoordinates[2] = uvCoords.x + dimensions.x;
	textureCoordinates[3] = uvCoords.y;
	textureCoordinates[4] = uvCoords.x;
	textureCoordinates[5] = uvCoords.y;
	textureCoordinates[6] = uvCoords.x + dimensions.x;
	textureCoordinates[7] = uvCoords.y;
	textureCoordinates[8] = uvCoords.x;
	textureCoordinates[9] = uvCoords.y + dimensions.y;
	textureCoordinates[10] = uvCoords.x + dimensions.x;
	textureCoordinates[11] = uvCoords.y + dimensions.y;
}

float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

void Entity::move(float elapsed, bool boundary)
{
	velocity.x = lerp(velocity.x, 0.0f, elapsed * friction.x);
	velocity.y = lerp(velocity.y, 0.0f, elapsed * friction.y);

	velocity.x += acceleration.x * elapsed;
	velocity.y += acceleration.y * elapsed;

	position.x += velocity.x * elapsed;
	position.y += velocity.y * elapsed;

	acceleration.x = 0;
	acceleration.y = 0;
	
	if (boundary)
	{
		if (position.x >= 7.4075f)
		{
			position.x = 7.39f;
			velocity.x = 0.0f;
		}
		if (position.x <= -7.4075f)
		{
			position.x = -7.39f;
			velocity.x = 0.0f;
		}
		if (position.y >= 3.5f)
		{
			position.y = 3.49f;
			velocity.y = 0.0f;
		}
		if (position.y <= -3.5f)
		{
			position.y = -3.49f;
			velocity.y = 0.0f;
		}
	}

}

void Entity::moveConstant(float elapsed)
{
	position.x += velocity.x * elapsed;
	position.y += velocity.y * elapsed;
}

void Entity::Draw(ShaderProgram* program, Matrix &modelMatrix)
{
	//Modify matrix based on vector values
	modelMatrix.identity();

	if (!position.isEmpty())
		modelMatrix.setPosition(position.x, position.y, position.z);
	if (!scale.isEmpty())
		modelMatrix.setScale(scale.x, scale.y, scale.z);
	if (!rotate.isEmpty())
		modelMatrix.setRotation(rotate.x);

	program->setModelMatrix(modelMatrix);
	
	//texture
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoordinates);
	glEnableVertexAttribArray(program->texCoordAttribute);

	//Image
	glBindTexture(GL_TEXTURE_2D, texture);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Entity::DrawPolygon(ShaderProgram* program, Matrix &modelMatrix)
{
	//Modify matrix based on vector values
	modelMatrix.identity();

	if (!position.isEmpty())
		modelMatrix.Translate(position.x, position.y, position.z);
	if (!scale.isEmpty())
		modelMatrix.Scale(scale.x, scale.y, scale.z);
	if (!rotate.isEmpty())
		modelMatrix.Rotate(rotate.x);

	program->setModelMatrix(modelMatrix);

	if (triangle)
	{
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, verticesTri);
		glEnableVertexAttribArray(program->positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	else
	{
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glDisableVertexAttribArray(program->positionAttribute);
}

void Entity::WallCollision(std::vector<Vector3> coords)
{
	for (int i = 0; i < coords.size(); ++i)
	{
		//left and right walls
		if (coords[i].x >= 3.55)
		{
			velocity.x = -velocity.x;
			position.x -= 0.005;
		}
		else if (coords[i].x <= -3.55)
		{
			velocity.x = -velocity.x;
			position.x += 0.005;
		}

		//top and bottom walls
		if (coords[i].y >= 4.0)
		{
			velocity.y = -velocity.y;
			position.y -= 0.005;
		}
		else if (coords[i].y <= -4.0)
		{
			velocity.y = -velocity.y;
			position.y += 0.005;
		}

	}
}

std::vector<Vector3> Entity::worldSpace(Matrix &modelMatrix)
{
	std::vector<Vector3> worldSpaceCoords;

	if (triangle)
	{
		for (int i = 0; i < 6; i += 2)
		{
			Vector3 vertex = modelMatrix.mult(Vector3(verticesTri[i], verticesTri[i+1], 1));
			worldSpaceCoords.push_back(vertex);
		}
	}
	else
	{
		for (int i = 0; i < 11; i += 2)
		{
			Vector3 vertex = modelMatrix.mult(Vector3(vertices[i], vertices[i + 1], 1));
			worldSpaceCoords.push_back(vertex);	
			if (i == 4)
				i = 8;
		}
	}

	return worldSpaceCoords;
}

Vector3 Entity::getDimensions()
{
	return dimensions;
}

