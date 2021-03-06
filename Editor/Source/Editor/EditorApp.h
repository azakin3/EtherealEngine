#pragma once
#include "Runtime/System/Application.h"
#include "EditState.h"
#include "Console/ConsoleLog.h"

class EditorApp : public Application
{
public:
	//-----------------------------------------------------------------------------
	//  Name : frameBegin (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	virtual bool frameBegin(bool runSimulation = true);

	//-----------------------------------------------------------------------------
	//  Name : frameWindowEnd (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	virtual void frameWindowEnd(RenderWindow& window);

	//-----------------------------------------------------------------------------
	//  Name : createMainWindow (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	virtual std::shared_ptr<RenderWindow> createMainWindow();

	//-----------------------------------------------------------------------------
	//  Name : initInstance (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	virtual bool initInstance(const std::string& rootDataDir, const std::string& commandLine);
	
	//-----------------------------------------------------------------------------
	//  Name : initApplication (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	
	virtual bool initApplication();
	//-----------------------------------------------------------------------------
	//  Name : initUI (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	
	virtual bool initUI();
	//-----------------------------------------------------------------------------
	//  Name : initSystems (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	
	virtual bool initSystems();
	//-----------------------------------------------------------------------------
	//  Name : initDocks (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	virtual bool initDocks();

	//-----------------------------------------------------------------------------
	//  Name : initEditState (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	virtual bool initEditState();

	//-----------------------------------------------------------------------------
	//  Name : shutDown (virtual )
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	virtual bool shutDown();

	//-----------------------------------------------------------------------------
	//  Name : getEditState ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	inline EditState& getEditState() { return mEditState; }

	//-----------------------------------------------------------------------------
	//  Name : createProject ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void createProject(const std::string& projectDir);

	//-----------------------------------------------------------------------------
	//  Name : openProject ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void openProject(const std::string& projectDir);

	//-----------------------------------------------------------------------------
	//  Name : openProjectManager ()
	/// <summary>
	/// 
	/// 
	/// 
	/// </summary>
	//-----------------------------------------------------------------------------
	void openProjectManager();
private:
	/// Edit state
	EditState mEditState;
	/// Editor docks
	std::vector<std::unique_ptr<ImGuiDock::Dock>> mDocks;
	/// Editor Console log
	std::shared_ptr<ConsoleLog> mConsoleLog = std::make_shared<ConsoleLog>();
};

template<>
inline Application& Singleton<Application>::create()
{
	static EditorApp t;
	return t;
};

template<>
inline EditorApp& Singleton<EditorApp>::create()
{
	return static_cast<EditorApp&>(Singleton<Application>::getInstance());
};