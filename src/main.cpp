
#include "../include/shape.h"

int main(int argv, char** argc)
{
	shape::transformation<float> t0;
	shape::transformation<float> t1(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(2.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	shape::transformation<float> t2(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(2.0f), glm::mat3(1.0f));

	for (int i = 1; i < argv; i++)
	{
		FILE* file = 0;
		fopen_s(&file, argc[i], "r");
		shape* shape = new ::shape;
		shape->read(file);
		fclose(file);
		shape->clear();
		delete shape;
	}

	return 0;
}