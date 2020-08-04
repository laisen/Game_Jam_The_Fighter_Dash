#pragma once
#include <glm/glm.hpp>
#include <vector>

enum eObstacleTypes
{
	LOW,
	HIGH,
	TBD
};

class cObstacle
{
public:
	cObstacle();
	glm::vec3 position;
	glm::vec3 velocity;	
	eObstacleTypes type;
	
	bool finish;
};

class cObstacleGenerator
{
public:
	cObstacleGenerator();

	void Step(float deltaTime);
	void GetObstacle(std::vector<cObstacle*>& vec_obstacles);
	void Reset();
	bool CheckPlayerCollision(cObstacle* obstacle);
	void KillPlayer();

private:
	void m_RelocateObstacle();

	std::vector<cObstacle*> m_vec_obstacles;
	float m_lastDuration;	
	float m_Duration;

	template <class T>
	T randInRange(T min, T max)
	{
		double value = min + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (static_cast<double>(max - min))));
		return static_cast<T>(value);
	};
};

