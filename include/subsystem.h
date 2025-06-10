#pragma once

// Interface for subsystems in the application
class ISubsystem 
{
public:
	ISubsystem() = default;
	virtual ~ISubsystem() = default;
	
	
	virtual bool  init() = 0;
	virtual void  update(float deltaTime) = 0;
	virtual void  shutdown() = 0;

protected:

	bool m_isInitialized = false; // Flag to check if the subsystem is initialized
};