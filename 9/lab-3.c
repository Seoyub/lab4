#include <pthread.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#define THREAD_POOL_SIZE 5

using namespace std;

class Thread
{
	private:
		pthread_mutex_t mmutex;
		pthread_cond_t mcond;
		int value;
	public:
		int id;
		int LockFlag;
		Thread();
		int GetLock();
		int TryLock();
		int UnLock();
		int Job();
		int Signal();
		void SetValue(int a)
		{
			value = a;
		}
};

// 뮤텍스와 조건변수 초기화 
Thread::Thread()
{
	LockFlag = 1;
	value = 0;
	pthread_mutex_init(&mmutex, NULL);
	pthread_cond_init(&mcond, NULL);
}


// 모텍스 잠금 얻기 시도
int Thread::TryLock()
{
	int rtv;
	if(LockFlag != 1)
	{
		return -1;
	}
   	LockFlag = 0;
	rtv = pthread_mutex_lock(&mmutex);
	return 0;
}

// 조건변수에 시그널을 전송한다.
// 시그널을 전송한 후에는 뮤텍스 잠금을 되돌려준다.
int Thread::Signal()
{
	pthread_cond_signal(&mcond);
	UnLock();
}

// 자식 쓰레드에서 실행할 작업 메서드
int Thread::Job()
{
	char fname[30];
	FILE *fp;
	sprintf(fname, "Walk_%d.txt", id);
	fp = fopen(fname,"w");
	while(1)
	{
		pthread_mutex_lock(&mmutex);

		LockFlag = 1;		
		pthread_cond_wait(&mcond, &mmutex);
		LockFlag = 0;
		
		cout << "Walk " << id << endl;
		fputs("Job\n", fp);
		usleep(1000);
		pthread_mutex_unlock(&mmutex);
	}
}

// 뮤텍스 잠금 되돌려준다.
int Thread::UnLock()
{
	pthread_mutex_unlock(&mmutex);
}


// 쓰레드 함수
void *thread_func(void *arg)
{
	Thread *lThread = (Thread *)arg;
	lThread->Job();
}

int main(int argc, char **argv)
{
	int i = 0;
	int mstat;
	pthread_t p_thread;
	vector<Thread *> ThreadList;
	Thread *lThread;
	pthread_mutex_t mutex_lock;
	pthread_mutexattr_t attr;
	int kind;



	// 쓰레드 풀을 만든다.
	for(i = 0; i < THREAD_POOL_SIZE; i++)
	{
		lThread = new Thread();
		lThread->id = i+1;
		ThreadList.push_back(lThread);
		pthread_create(&p_thread, NULL, thread_func, (void *)lThread);
		usleep(100);
	}

	FILE *fp;
	fp = fopen("Job.txt","w");
	int j = 0;
	while(1)
	{
		// 작업 가능한 쓰레드를 찾아서
		// 조건변수 시그널을 전송한다.
		for (i = 0; i < THREAD_POOL_SIZE; i++)
		{
			if((mstat = ThreadList[i]->TryLock()) == 0)
			{
				cout << "Job Signal " << i + 1<< endl;
				fputs("Job Signal\n", fp);
				ThreadList[i]->SetValue(i+1);
				ThreadList[i]->Signal();
				break;
			}
		}
		j ++;
		if(j == 10000) break;
		usleep(10);
	}
}
