#include "CoreMinimal.h"

/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


namespace CE
{
	JobManager::JobManager(const Name& name, const JobManagerDesc& desc)
		: name (name)
		, defaultTag(desc.defaultTag)
		, numThreads(FixNumThreads(desc))
	{
		SpawnWorkThreads(desc);
	}

	JobManager::~JobManager()
	{
		jobManagerMutex.Lock();

		for (int i = 0; i < workerThreads.GetSize(); i++)
		{
			workerThreads[i]->Deactivate();
			delete workerThreads[i]; // deleting will terminate the worker thread
		}

		workerThreads.Clear();

		jobManagerMutex.Unlock();
	}

	JobManager::WorkThread::~WorkThread()
	{
		delete threadLocal;
		threadLocal = nullptr;
	}

	void JobManager::WorkThread::Deactivate()
	{
		deactivate.store(true, std::memory_order_release);
		Awake();
	}

	void JobManager::WorkThread::DeactivateAndWait()
	{
		Deactivate();
		if (thread.IsJoinable())
			thread.Join();
	}

	void JobManager::WorkThread::Complete()
	{
		complete.store(true, std::memory_order_release);
		Awake();
		if (thread.IsJoinable())
			thread.Join();
	}

	void JobManager::WorkThread::Awake()
	{		
		sleepEvent.release(1);
	}

	void JobManager::WorkThread::Sleep()
	{
		
	}

	bool JobManager::WorkThread::IsLocalQueueEmpty()
	{
		auto local = threadLocal.load();
		if (local != nullptr)
		{
			return local->queue.IsEmpty();
		}
		return false;
	}

	int JobManager::FixNumThreads(const JobManagerDesc& desc)
	{
		int totalThreads = desc.totalThreads;
		
		if (totalThreads == 0)
			totalThreads = Thread::GetHardwareConcurrency() - 1; // Subtract 1 thread so we have extra logical core remaining for Main Thread
		if (totalThreads < 2)
			totalThreads = 2;
		if (totalThreads > 128)
			totalThreads = 128;

		return totalThreads;
	}

	void JobManager::SpawnWorkThreads(const JobManagerDesc& desc)
	{
		if (!workerThreads.IsEmpty())
			return;

		for (int i = 0; i < numThreads; i++)
		{
			JobThreadDesc threadDesc = {};
			if (i < desc.threads.GetSize())
				threadDesc = desc.threads[i];

			WorkThread* worker = new WorkThread();
			worker->name = threadDesc.threadName;
			worker->tag = threadDesc.tag;
			worker->owner = this;
			worker->isMainThread = false;
			worker->index = i;
			worker->isWorker = true;

			// Fix: acquire() will sleep the thread only when acquire() is called the 2nd time.
			//worker->sleepEvent.acquire();

			worker->thread = Thread([this, worker]
				{
					this->ProcessJobsWorker(worker);
				});

			while (worker->threadLocal == nullptr)
			{
				// Wait till thread is initialized
			}
			
			workerThreads.Add(worker);
		}

		threadsCreated.store(true, std::memory_order_release);
	}

	CE_THREAD_LOCAL JobManager::WorkThread* currentThreadInfo = nullptr;

	int JobManager::GetCurrentJobThreadIndex()
	{
		if (currentThreadInfo == nullptr)
			return -1;
		return currentThreadInfo->index;
	}

	void JobManager::SuspendJobUntilReady(Job* job)
	{
		auto currentWorkThread = GetCurrentOrCreateThread();
		CE_ASSERT(currentWorkThread != nullptr, "Could not find current work thread"); // Should never happen

		ProcessJobsInternal(currentThreadInfo, job);
	}

	JobManager::WorkThread* JobManager::GetCurrentOrCreateThread()
	{
		auto threadInfo = currentThreadInfo;

		if (!threadInfo)
		{
#ifndef PAL_TRAIT_BUILD_MONOLITHIC
			threadInfo = FindAndSetCurrentWorkThread();
#endif
			if (!threadInfo)
			{
				threadInfo = new WorkThread();
				threadInfo->name = "Dynamic";
				threadInfo->tag = defaultTag;
				threadInfo->owner = this;
				threadInfo->isMainThread = false;
				threadInfo->index = numThreads.load(std::memory_order_acquire);
				threadInfo->threadId = Thread::GetCurrentThreadId();

				jobManagerMutex.Lock();
				workerThreads.Add(threadInfo);
				jobManagerMutex.Unlock();

				numThreads.fetch_add(1, std::memory_order_acq_rel);
			}
		}

		return threadInfo;
	}

#ifndef PAL_TRAIT_BUILD_MONOLITHIC
	JobManager::WorkThread* JobManager::FindAndSetCurrentWorkThread()
	{
		auto thisThreadId = Thread::GetCurrentThreadId();

		int numThreads = this->numThreads.load(std::memory_order_acquire);

		for (int i = 0; i < numThreads; i++)
		{
			if (workerThreads[i]->threadId == thisThreadId)
			{
				auto info = workerThreads[i];
				currentThreadInfo = info;
				return info;
			}
		}

		return nullptr;
	}
#endif

	void JobManager::ProcessJobsWorker(WorkThread* threadInfo)
	{
		threadInfo->threadId = Thread::GetCurrentThreadId();
		threadInfo->threadLocal = new WorkThreadLocal();

		// Wait for all threads to initialize. Otherwise, we will be stealing jobs from uninitialized threads
		while (!threadsCreated.load(std::memory_order_acquire))
		{
			// Wait till all threads are initialized
		}

		currentThreadInfo = threadInfo;

		ProcessJobsInternal(threadInfo, nullptr);

		currentThreadInfo = nullptr;
	}

	void JobManager::ProcessJobsInternal(WorkThread* threadInfo, Job* suspendedJob)
	{
		WorkQueue* localQueue = threadInfo->isWorker ? &threadInfo->threadLocal.load()->queue : nullptr;
		u32 victim = ((workerThreads.GetSize() > 1) && (workerThreads[0] == threadInfo)) ? 1 : 0;
		
		while (true)
		{
			// Check if suspended job is ready to be executed
			if (suspendedJob != nullptr && suspendedJob->GetDependentCount() == 0)
			{
				return;
			}

			int numWorkerThreads = numThreads.load(std::memory_order_acquire);

			// Try to get an initial job
			Job* job = nullptr;
			{
				// Go to sleep if local queue is empty
				if (threadInfo->isWorker && !suspendedJob)
				{
					if (threadInfo->deactivate)
						return;

					bool shouldSleep = false;
					{
						LockGuard<SharedMutex> lock{ jobManagerMutex };

						if (globalQueue.empty())
						{
							shouldSleep = true;

							numAvailableWorkers.fetch_add(1, std::memory_order_acq_rel);

							threadInfo->isAvailable.exchange(true, std::memory_order_acq_rel);
						}
					}

					if (shouldSleep)
					{
						// no available work, so go to sleep (or we have already been signaled by another thread and will acquire the semaphore but not actually sleep)
						threadInfo->sleepEvent.acquire();

						if (threadInfo->deactivate)
							return;
					}
				}

				// Check if suspended job is ready to be executed
				if (suspendedJob != nullptr && suspendedJob->GetDependentCount() == 0)
				{
					return;
				}

				{
					LockGuard<SharedMutex> lock{ jobManagerMutex };

					if (!globalQueue.empty())
					{
						job = globalQueue.front();
						if (job != nullptr && (job->GetThreadFilter() == JOB_THREAD_UNDEFINED || job->GetThreadFilter() == threadInfo->tag))
						{
							globalQueue.pop_front();
							totalJobsInGlobalQueue--;
						}
						else
						{
							job = nullptr;
							// Job cannot be processed by this thread, activate another thread
							AwakeWorker();
						}
					}
				}
			}

			if (!job && localQueue)
			{
				job = localQueue->LocalPop();
			}

			bool isTerminated = false;
			while (!isTerminated)
			{
				// run current job and jobs from the local queue until it is empty
				while (job)
				{
					threadInfo->currentJob = job;
					Process(job);
					threadInfo->currentJob = nullptr;

					// Check if suspended job is ready to be executed
					if (suspendedJob != nullptr && suspendedJob->GetDependentCount() == 0)
					{
						return;
					}

					if (localQueue)
					{
						job = localQueue->LocalPop();
						if (!localQueue->IsEmpty())
						{
							// not necessary, just an optimization - wakeup sleeping threads, there's work to be done
							AwakeWorker();
						}
					}
					else
					{
						job = nullptr;
					}
				}

				if (workerThreads.GetSize() < 2) // Cannot steal if there is only 1 thread
				{
					isTerminated = true;
				}
				else
				{
					//attempt to steal a job from another thread's queue
					u32 numStealAttempts = 0;
					const u32 maxStealAttempts = (unsigned int)workerThreads.GetSize() * 3; //try every thread a few times before giving up

					while (!job)
					{
						// Check if suspended job is ready to be executed
						if (suspendedJob != nullptr && suspendedJob->GetDependentCount() == 0)
						{
							return;
						}

						WorkQueue* victimQueue = &workerThreads[victim]->threadLocal.load()->queue;

						// attempt steal
						job = victimQueue->TrySteal(workerThreads[victim]->tag);
						if (job)
						{
							// steal success
							break;
						}

						++numStealAttempts;
						if (numStealAttempts > maxStealAttempts)
						{
							// Time to give up, it's likely all the local queues are empty. Note that this does not mean all the jobs
							// are done, some jobs may be in progress, or we may have had terrible luck with our steals. There may be
							// more jobs coming, another worker could create many new jobs right now. But the only way this thread
							// will get a new job is from the global queue or by a steal, so we're going to sleep until a new job is
							// queued.
							// The important thing to note is that all jobs will be processed, even if this thread goes to sleep while
							// jobs are pending.

							isTerminated = true;
							break;
						}

						//steal failed, choose a new victim for next time
						victim = (victim + 1) % workerThreads.GetSize();

						if (workerThreads[victim] == threadInfo)
						{
							// Do not steal from the same thread
							victim = (victim + 1) % workerThreads.GetSize();
						}
					}
				}
			}

			if (threadInfo->complete)
			{
				return;
			}
		}
	}

	void JobManager::Process(Job* job)
	{
		//Job* dependent = job->GetDependent();
		bool isAutoDelete = job->IsAutoDelete();

		job->Process();

		auto oldFlags = job->GetDependentCountAndFlags();
		job->SetDependentCountAndFlags(oldFlags | Job::FLAGS_FINISHED);

		{
			LockGuard lock{ job->dependentJobsMutex };

			for (auto dependent : job->dependentJobs)
			{
				if (dependent != nullptr)
				{
					dependent->DecrementDependentCount();
				}
			}
		}

		job->Finish();

		if (isAutoDelete)
		{
			delete job;
		}
	}

	int JobManager::GetNumThreads()
	{
		return workerThreads.GetSize();
	}

	void JobManager::DeactivateWorkers()
	{
		if (workerThreads.IsEmpty())
			return;

		for (int i = 0; i < workerThreads.GetSize(); i++)
		{
			workerThreads[i]->Deactivate();
		}
	}

	void JobManager::DeactivateWorkersAndWait()
	{
		if (workerThreads.IsEmpty())
			return;

		for (int i = 0; i < workerThreads.GetSize(); i++)
		{
			workerThreads[i]->DeactivateAndWait();
		}
	}

	void JobManager::Complete()
	{
		if (workerThreads.IsEmpty())
			return;

		// Complete() all worker threads
		for (int i = 0; i < workerThreads.GetSize(); i++)
		{
			workerThreads[i]->Complete();
		}
	}

	void JobManager::Finish()
	{
		while (totalJobsInGlobalQueue > 0)
		{
			// Wait for global queue to become empty
		}

		Complete();
	}

	void JobManager::EnqueueJob(Job* job)
	{
		if (job == nullptr || workerThreads.IsEmpty())
			return;

		WorkThread* info = currentThreadInfo;
		if (!info)
		{
			info = FindAndSetCurrentWorkThread();
		}

		auto jobTreadFilterTag = job->GetThreadFilter();

		if (info && info->isWorker && info->owner == this && (jobTreadFilterTag == JOB_THREAD_UNDEFINED || info->tag == jobTreadFilterTag))
		{
			info->threadLocal.load()->queue.LocalPush(job);
			AwakeWorker(info);
			AwakeWorker();
		}
		else
		{
			LockGuard<SharedMutex> lock{ jobManagerMutex };

			globalQueue.push_back(job);
			totalJobsInGlobalQueue++;
			AwakeWorker();
		}
	}

	void JobManager::AwakeOrSleepWorkers()
	{
		// Only activate the most optimal number of threads needed for the workload


	}

	bool JobManager::AwakeWorker(WorkThread* specificWorker)
	{
		if (specificWorker != nullptr)
		{
			if (specificWorker->isAvailable.exchange(false, std::memory_order_acq_rel) == true)
			{
				// decrement number of available workers
				numAvailableWorkers.fetch_sub(1, std::memory_order_acq_rel);
				// resume the thread execution

				specificWorker->sleepEvent.release(1);
				return true;
			}

			return false;
		}

		// find an available worker thread (we do it brute force because the number of threads is small)
		while (numAvailableWorkers.load(std::memory_order_acquire) > 0)
		{
			for (size_t i = 0; i < workerThreads.GetSize(); ++i)
			{
				WorkThread* info = workerThreads[i];
				if (info->isAvailable.exchange(false, std::memory_order_acq_rel) == true)
				{
					// decrement number of available workers
					numAvailableWorkers.fetch_sub(1, std::memory_order_acq_rel);
					// resume the thread execution

					info->sleepEvent.release();
					return true;
				}
			}
		}

		return false;
	}

} // namespace CE
