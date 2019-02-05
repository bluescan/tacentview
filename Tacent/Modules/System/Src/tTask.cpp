// tTask.cpp
//
// Simple and efficient task management using a heap-based priority queue.
//
// Copyright (c) 2006, 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "System/tTask.h"


tTaskSet::tTaskSet(int64 counterFreq, double maxTimeDelta) :
	ExecuteTime(0),
	CounterFreq(counterFreq),
	MaxTimeDelta(maxTimeDelta),
	PriorityQueue(NumTasks, GrowSize)
{
}


tTaskSet::tTaskSet() :
	ExecuteTime(0),
	CounterFreq(0),
	MaxTimeDelta(0),
	PriorityQueue(NumTasks, GrowSize)
{
}


void tTaskSet::Update(int64 counter)
{
	bool runningTasks = true;
	while (runningTasks)
	{
		if (PriorityQueue.GetNumItems() == 0)
			return;

		if (PriorityQueue.GetMin().Key <= counter)
		{
			tPQ<tTask*>::tItem qn = PriorityQueue.GetRemoveMin();
			tTask* t = (tTask*)qn.Data;

			// If there is no function tTask pointer we're all done. The node is already removed from the queue.
			if (t)
			{
				double td = double(counter - ExecuteTime) / double(CounterFreq);
				if (td > MaxTimeDelta)
					td = MaxTimeDelta;

				double nextTime = t->Execute(td);
				int64 nextTimeDelta = int64( nextTime*double(CounterFreq) );

				// The 1 guarantees no infinite loop here.
				if (nextTimeDelta <= 0)
					nextTimeDelta = 1;

				qn.Key = counter + nextTimeDelta;
				PriorityQueue.Insert(qn);
			}
		}
		else
		{
			runningTasks = false;
		}
	}

	ExecuteTime = counter;
}
