/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef SYNCHRONIZED_H
#define SYNCHRONIZED_H

#include <semLib.h>

#include "Base.h"

#define CRITICAL_REGION(s) { Synchronized _sync(s);
#define END_REGION }

class Synchronized;

/**
 * Wrap a vxWorks semaphore (SEM_ID) for easier use in C++. For a static
 * instance, the constructor runs at program load time before main() can spawn
 * any tasks. Use that to fix race conditions in setup code.
 *
 * This uses a semM semaphore which is "reentrant" in the sense that the owning
 * task can "take" the semaphore more than once. It will need to "give" the
 * semaphore the same number of times to unlock it.
 *
 * This class is safe to use in static variables because it does not depend on
 * any other C++ static constructors or destructors.
 */
class ReentrantSemaphore
{
public:
	explicit ReentrantSemaphore() {
		m_semaphore = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE);
	}
	~ReentrantSemaphore() {
		semDelete(m_semaphore);
	}

	/**
	 * Lock the semaphore, blocking until it's available.
	 * @return 0 for success, -1 for error. If -1, the error will be in errno.
	 */
	int take() {
		return semTake(m_semaphore, WAIT_FOREVER);
	}

	/**
	 * Unlock the semaphore.
	 * @return 0 for success, -1 for error. If -1, the error will be in errno.
	 */
	int give() {
		return semGive(m_semaphore);
	}

private:
	SEM_ID m_semaphore;

	friend class Synchronized;
	DISALLOW_COPY_AND_ASSIGN(ReentrantSemaphore);
};

/**
 * Provide easy support for critical regions.
 *
 * A critical region is an area of code that is always executed under mutual exclusion. Only
 * one task can be executing this code at any time. The idea is that code that manipulates data
 * that is shared between two or more tasks has to be prevented from executing at the same time
 * otherwise a race condition is possible when both tasks try to update the data. Typically
 * semaphores are used to ensure only single task access to the data.
 *
 * Synchronized objects are a simple wrapper around semaphores to help ensure
 * that semaphores are always unlocked (semGive) after locking (semTake).
 *
 * You allocate a Synchronized as a local variable, *not* on the heap. That
 * makes it a "stack object" whose destructor runs automatically when it goes
 * out of scope. E.g.
 *
 *   { Synchronized _sync(aReentrantSemaphore); ... critical region ... }
 */
class Synchronized
{
public:
	explicit Synchronized(SEM_ID);
	explicit Synchronized(ReentrantSemaphore&);
	virtual ~Synchronized();
private:
	SEM_ID m_semaphore;

	DISALLOW_COPY_AND_ASSIGN(Synchronized);
};

#endif
