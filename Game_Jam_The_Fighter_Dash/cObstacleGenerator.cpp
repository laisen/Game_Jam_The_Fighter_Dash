#include "cObstacleGenerator.h"
#include "cGameObject.h"
#include "cParticleEmitter.h"
#define UpLaneZ -4.0f
#define MidLaneZ 0.0f
#define DownLaneZ 4.0f
#define StartLineX 20.0f
#define FinishLineX -20.0f
#define OffsetX 7.0f
#define Duration 2.5f
#define ObstacleMaxAmount 12

extern cGameObject* player1;
extern cParticleEmitter* dustParticles;
extern float camerShakeAmplitude;
extern bool playerInPlace;

cObstacle::cObstacle()
{
	this->position = glm::vec3(0.0f, 0.0f, 0.0f);
	this->velocity = glm::vec3(-6.0f, 0.0f, 0.0f);
	this->type = TBD;
	this->finish = true;
}

cObstacleGenerator::cObstacleGenerator()
{
	this->m_lastDuration = 0.0f;
	this->m_Duration = Duration;	

	for (size_t i = 0; i < ObstacleMaxAmount; i++)
	{
		cObstacle* newObstacle = new cObstacle();
		this->m_vec_obstacles.push_back(newObstacle);
	}
}

void cObstacleGenerator::Step(float deltaTime)
{
	if (player1->dead || !playerInPlace)	return;

	for (std::vector<cObstacle*>::iterator itO = this->m_vec_obstacles.begin();
		itO != this->m_vec_obstacles.end(); itO++)
	{
		cObstacle* pCurObstacle = *itO;

		if (pCurObstacle->position.x <= FinishLineX)
		{
			pCurObstacle->finish = true;
			continue;
		}

		if(!pCurObstacle->finish)		// Active
		{
			pCurObstacle->position += (pCurObstacle->velocity * deltaTime);
			if (CheckPlayerCollision(pCurObstacle))	return;
		}
	}

	this->m_lastDuration += deltaTime;

	if (this->m_lastDuration >= this->m_Duration)
	{		
		this->m_RelocateObstacle();
		// Reset the timer
		this->m_lastDuration = 0.0f;
	}
}

void cObstacleGenerator::GetObstacle(std::vector<cObstacle*>& vec_obstacles)
{
	vec_obstacles.clear();
	vec_obstacles.reserve(this->m_vec_obstacles.size());

	for (std::vector<cObstacle*>::iterator itO = this->m_vec_obstacles.begin();
		itO != this->m_vec_obstacles.end(); itO++)
	{
		cObstacle* pCurObstacle = *itO;

		if (!pCurObstacle->finish)
		{
			vec_obstacles.push_back(pCurObstacle);
		}
	}

}

void cObstacleGenerator::Reset()
{
	this->m_lastDuration = 0.0f;	
	this->m_vec_obstacles.clear();

	for (size_t i = 0; i < ObstacleMaxAmount; i++)
	{
		cObstacle* newObstacle = new cObstacle();
		this->m_vec_obstacles.push_back(newObstacle);
	}

}

bool cObstacleGenerator::CheckPlayerCollision(cObstacle* obstacle)
{
	if ( glm::distance(obstacle->position, player1->getPosition()) <= 1.8f
		&& (player1->isMovingDown || player1->isMovingUp) )
	{
		KillPlayer();
		return true;
	}
	else if (glm::distance(obstacle->position, player1->getPosition()) <= 1.5f)
	{
		if (obstacle->type == LOW && !player1->isJumping)
		{
			KillPlayer();
			return true;
		}
		
		if (obstacle->type == HIGH && !player1->isRolling)
		{
			KillPlayer();
			return true;
		}
		
	}

	return false;
}

void cObstacleGenerator::KillPlayer()
{
	player1->dead = true;
	player1->setCurrentAnimationName("Die");
	player1->loopAnimationState = false;
	player1->setAnimationFrameTime(0.0f);
	dustParticles->disableNewParticles();
	camerShakeAmplitude = 0.7f;
}

void cObstacleGenerator::m_RelocateObstacle()
{	

	// Down lane
	for (std::vector<cObstacle*>::iterator itO = this->m_vec_obstacles.begin();
		itO != this->m_vec_obstacles.end(); itO++)
	{
		cObstacle* pCurObstacle = *itO;

		if (pCurObstacle->finish)
		{
			int randType = randInRange<int>(1, 30);
			if (randType <= 10)
			{
				pCurObstacle->type = LOW;
			}
			else if (randType > 10 && randType <= 20)
			{
				pCurObstacle->type = HIGH;
			}
			else
			{
				pCurObstacle->type = TBD;		// Place no obstacle
				break;
			}
			
			pCurObstacle->position = glm::vec3(StartLineX + randInRange<float>(0.0f, OffsetX), 0.0f, DownLaneZ);
			pCurObstacle->finish = false;
			break;
		}		
	}

	// Middle lane
	for (std::vector<cObstacle*>::iterator itO = this->m_vec_obstacles.begin();
		itO != this->m_vec_obstacles.end(); itO++)
	{
		cObstacle* pCurObstacle = *itO;

		if (pCurObstacle->finish)
		{
			int randType = randInRange<int>(1, 30);
			if (randType <= 10)
			{
				pCurObstacle->type = LOW;
			}
			else if (randType > 10 && randType <= 20)
			{
				pCurObstacle->type = HIGH;
			}
			else
			{
				pCurObstacle->type = TBD;		// Place no obstacle
				break;
			}

			pCurObstacle->position = glm::vec3(StartLineX + randInRange<float>(0.0f, OffsetX), 0.0f, MidLaneZ);
			pCurObstacle->finish = false;
			break;
		}
	}

	// Up lane
	for (std::vector<cObstacle*>::iterator itO = this->m_vec_obstacles.begin();
		itO != this->m_vec_obstacles.end(); itO++)
	{
		cObstacle* pCurObstacle = *itO;

		if (pCurObstacle->finish)
		{
			int randType = randInRange<int>(1, 30);
			if (randType <= 10)
			{
				pCurObstacle->type = LOW;
			}
			else if (randType > 10 && randType <= 20)
			{
				pCurObstacle->type = HIGH;
			}
			else
			{
				pCurObstacle->type = TBD;		// Place no obstacle
				break;
			}

			pCurObstacle->position = glm::vec3(StartLineX + randInRange<float>(0.0f, OffsetX), 0.0f, UpLaneZ);
			pCurObstacle->finish = false;
			return;
		}
	}

}
