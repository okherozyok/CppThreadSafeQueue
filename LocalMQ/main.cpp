﻿// LocalMQ.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <vector>
#include "ThreadSafeQueue.h"
#include "localMQ.h"

using namespace std;

void createQueueTest()
{
	LocalMQ<int> mq;
	vector<thread> runThreads;
	for (int i = 0; i < 10; i++)
	{
		runThreads.emplace_back([&mq]()
			{
				localQueueHandle_uint queueId = mq.registerListen(100, 10);
				cout << static_cast<int>(queueId) << endl;
			}
		);
	}

	for (auto& thread : runThreads) {
		thread.join(); // 阻塞当前线程，直到thread线程执行完毕
	}
}

void pubsubTest()
{
	struct Message
	{
		int i;
		Message(int value) :i(value)
		{

		}

		~Message()
		{
			cout << "Message 释放了" << endl;
		}
	};

	LocalMQ<Message> mq;
	localMessageType_uint msgType = 100;

	int subcribers = 10;

	vector< localQueueHandle_uint> subscribeQueues;
	for (int i = 0; i < subcribers; i++)
	{
		localQueueHandle_uint queueHandle = mq.registerListen(msgType, 10);
		subscribeQueues.emplace_back(queueHandle);
	}

	thread publish([&mq, msgType]()
		{
			this_thread::sleep_for(chrono::seconds(1));
			shared_ptr<Message> value = make_shared<Message>(9999);
			mq.publish(msgType, value);
		}
	);

	vector<thread> queueThreads;
	mutex mutex_;
	for (localQueueHandle_uint queueHandle : subscribeQueues)
	{
		queueThreads.emplace_back([&mq, queueHandle, &mutex_]()
			{
				shared_ptr<const Message> value = mq.subscribe(queueHandle);
				lock_guard<mutex> lock(mutex_);
				cout << queueHandle << "取到消息 " << value->i << endl;
			});
	}

	for (auto& queueThread : queueThreads)
	{
		queueThread.join();
	}
	publish.join();

	cout << "pubsub结束" << endl;
}

int main()
{
	//createQueueTest();
	pubsubTest();

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
