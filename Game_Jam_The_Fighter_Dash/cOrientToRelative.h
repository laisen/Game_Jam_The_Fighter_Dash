#pragma once
#include "iCommand.h"

class cOrientToRelative : public iCommand
{
public:
	cOrientToRelative()
	{
		this->m_UpdateHasBeenCalled = false;
	}
	// Rotate object RELATIVE to where command starts
	// Pass: 
	// - End rotation (Euler degrees) (vec3)
	// - Number of seconds to move (x)
	virtual void Init(std::vector<sPair> vecDetails);
	virtual void SetGameObject(cGameObject* pGO);
	virtual void SetTarget(cGameObject* pTargetGO) { return; }
	virtual void Update(double deltaTime);
	virtual bool IsDone(void);
	virtual void Reset(void);
	virtual void setName(std::string name) { this->m_Name = name; }
	virtual std::string getName(void) { return this->m_Name; }
	virtual void SetLoopTimes(int times) { return; }
	virtual void AddCommandSerial(iCommand* pCommand) { return; }
	virtual void AddCommandParallel(iCommand* pCommand) { return; }

private:	
	bool m_UpdateHasBeenCalled;
	std::string m_Name;
	cGameObject* m_pTheGO;
	glm::quat m_startOrentation;		// Starting location
	glm::quat m_endOrientation;			// Ending location
	glm::quat m_relativeOrientation;
	//glm::quat m_rotationalSpeed;		// Like velocity
	//glm::vec3 m_rotationalSpeedEuler;		// Like velocity
	float m_TimeToRotate;
	double m_TimeElapsed;
	unsigned int m_SmoothLevel;
};