#ifndef ENTITY_H
#define ENTITY_H
#include "ShaderProgram.h"
#include <string>
#include <vector>
#include "Vector3.h"

class Entity
{
private:
	std::string sheetName;
	std::string imgName;
	Vector3 dimensions; 
	
	GLuint texture;
	Vector3 uvCoords; 
	GLfloat textureCoordinates[12];
	int width;
	int height;
	bool triangle;

public:

	//Wrapper for texture/image data
	struct ImageData{
		GLuint texture;
		float width;
		float height;
	};

	//Constructor for lone image file
	Entity(const ImageData imgData, const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_);

	//Constructor for uneven sprite sheets
	Entity(const std::string sheetName_, const std::string imgName_, const ImageData imgData_, const std::string xml,
		const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_, const Vector3 acceleration_, const Vector3 friction_);

	//Constructor for evenly spaced sprite sheets
	Entity(const std::string sheetName_, int index, int spriteCountX, int spriteCountY, const ImageData imgData, float tileSize,
		const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_, const Vector3 acceleration_, const std::vector<int> frames);

	//Constructor for tiles
	Entity(std::vector<float> vertices, std::vector<float> textureCoords, const ImageData imgData,
		const Vector3 position_, const Vector3 scale_, const Vector3 velocity_, const Vector3 rotate_);

	//Constructor for untextured polygons
	Entity::Entity(const Vector3 position_, const Vector3 scale_,
		const Vector3 velocity_, const Vector3 rotate_, bool triangle_);

	//change entity to the next frame in its animation sequence
	void changeFrame(int index, int spriteCountX, int spriteCountY);

	std::vector<Vector3> worldSpace(Matrix &modelMatrix);
	void Entity::WallCollision(std::vector<Vector3> coords);

	static ImageData LoadImg(const std::string image, bool pixel);
	void Draw(ShaderProgram* program, Matrix &modelMatrix);
	void DrawPolygon(ShaderProgram* program, Matrix &modelMatrix);
	void move(float elapsed, bool boundary);
	void moveConstant(float elapsed);
	int frame;
	bool remove = false;
	float timeOnScreen = 0.0f;

	Vector3 getDimensions();
	Vector3 position;
	Vector3 scale; 
	Vector3 rotate;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 friction;

	float vertices[12];
	float verticesTri[6];

	//frames of animation
	std::vector<int> indecesOfAnimation;
};

#endif 