#pragma once
#ifndef __PRODUCERCONSUMER__
#define __PRODUCERCONSUMER__

#include <vector>
//#include <deque> //Issue: How to make it work for deque instead of queue?
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

//Question: How to terminate a thread reasonably?

class Q {
	/*deque<int>*/ queue<int> q_;
	std::mutex m_;
	std::condition_variable c;
public:
	Q() = default;
	Q(const Q&) = delete; //Disable copy c'stro
	Q& operator=(const Q&) = delete; //Disable assignment operator

	void printQ() {
		if (q_.empty()) return;
		cout << "content in queue: " << endl;
		//for (auto i : q_) {
		//{
		//	cout << i << " ";
		//}
		//cout << endl;
	}

	void add(int num) {
		//printQ();
		std::unique_lock<std::mutex> mlock(m_);
		q_.push(num);
		//q_.push_back(num);
		mlock.unlock();
		c.notify_one();
	}
	int remove() {
		//printQ();
		std::unique_lock<std::mutex> mlock(m_);
		int tmp;
		while (q_.empty()) {	//check condition to be safe against spurious wakes
			c.wait(mlock);
		}
		tmp = q_.front();
		q_.pop();
		//tmp = q_.front();  //Issue: Why back() is not working?
		//q_.pop_front();
		mlock.unlock();
		return tmp;
	} 
};

class Producer {
//private:
	Q& buffer_;
public:
	Producer(Q& buffer) : buffer_(buffer) {}
	~Producer() {}
	void run() {
		for(int i = 0; i < 130; i++) /*while(true)*/ {
			//int i = rand();
			buffer_.add(i);
			cout << __FUNCTION__ << " added...." << i << endl;
		}
	}
};

class Consumer {
	Q& buffer_;
public:
	Consumer(Q& buffer):buffer_(buffer){}
	~Consumer(){}

	void run() {
		int tmp;
		for (int i = 0; i < 13; i++)  /*while(true)*/{
			tmp = buffer_.remove();
			cout << __FUNCTION__ << " removed....";
			cout << tmp << endl;
		}
	}
};

void ProducerConsumer();
#endif