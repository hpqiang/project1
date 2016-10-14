#include <vector>
#include <thread>
#include <random>
#include <exception>
#include <iostream>
#include <chrono>

#include "ProducerConsumer.h"

void doSomething(int num, char c) {
	try {
		default_random_engine dre(42*c);
		uniform_int_distribution<int> id(10, 1000);
		for (int i = 0; i < num; i++) {
			this_thread::sleep_for(chrono::milliseconds(id(dre)));
			cout.put(c).flush();
		}
	}
	catch (const exception& e) {
		cerr << "Thread exception: " << e.what() << endl;
	}
}


//std::mutex mu;

//void shared_print(string msg, int id) {
//	std::lock_guard<std::mutex> guard(mu); //RAII, but it's not guarding cout.
//	//mu.lock(); //bad: because what if cout in next line create an exception?
//	cout << msg.c_str() << id << endl;
//	//mu.unlock();
//}

#include <string>
#include <fstream>

class LogFile {
	std::mutex _mu;
	//std::mutex _mu_open;
	std::once_flag _flag;
	ofstream _f;
public:
	LogFile() {
		//_f.open("log.txt");
	} // Need d'tor to close file
	  //void shared_print(string msg, int id) {
	void shared_print(string id, int value) {
		//{ // for thread safe
		//	std::unique_lock<mutex> locker2(_mu_open);
		//	if (!_f.is_open()) { //Lazy initialization
		//		std::unique_lock<mutex> locker2(_mu_open); // Not thread safe is putting heree
		//		_f.open("log.txt");
		//	}
		//} //RAII
		//Issue: call_once will cause one thread to run after the other as a whole? 
		std::call_once(_flag, [&]() {_f.open("log.txt"); }); // Be called only once by one thread
		std::lock_guard<std::mutex> guard(_mu);
		//Unique lock
		/*std::unique_lock<mutex> locker(_mu, std::defer_lock);
		locker.lock();*/
		_f/*cout*/ << "From " << id << ": " << value << endl;
		//locker.unlock();
		////....
		//locker.lock();
		////....
		//locker.unlock();

		//std::unique_lock<mutex> locker2 = std::move(locker);
	}
	//Caution: Never return f to the outside world
	ofstream& getStream() { return _f;  }
	// Never pass f as an argument to user provided function
	void processf(void fun(ofstream&)) {
		fun(_f);
	}
};

//Thread safe and exception safe for stack. Lecture 3 on concurrent programming

//Avoid deadlock
//1. prefer locking single mutex
//2. avoid locking a mutex and then call a user defined function
//3. use std::lock to lock more than one mutex
//4. Lock the mutex in the same order

void function_1(LogFile& log) {
	for (int i = 0; i > -100; i--) {
		//cout << "From t1: " << i << endl;
		log.shared_print(string("From t1: "), i);
	}
}

//mutex, deadlock, unique_lock, lazy initialization
void BoQian() {
	LogFile log;
	std::thread t1(function_1, std::ref(log));

	for (int i = 0; i < 100; i++) {
		//cout << "From main: " << i << endl;
		log.shared_print(string("From main: "), i);
	}
	t1.join();
	return;
}

#include <deque>

std::deque<int> q;
std::mutex mu;
std::condition_variable cond;

//producer
void f1() {
	int count = 10;
	while (count > 0) {
		std::unique_lock<mutex> locker(mu);
		q.push_front(count);
		locker.unlock();
		cond.notify_one();
		//cond.notify_all(); 
		std::this_thread::sleep_for(chrono::seconds(1));
		count--;
	}
}

//consumer
void f2() {
	int data = 0;
	while (data != 1) {
		std::unique_lock<mutex> locker(mu);  //Have to use unique_lock if using conditional variable
//		if (!q.empty()) {
		cond.wait(locker, []() {return !q.empty(); });  //spurious wake
		data = q.back();
		q.pop_back();
		locker.unlock();
		cout << "t2 got a value from t1: " << data << endl;
//		}
		//else {
		//	locker.unlock();
		//	std::this_thread::sleep_for(chrono::milliseconds(10));
		//}
	}
}
//conditional variable: For sync of threads
void BoQian2() {
	thread t1(f1);
	thread t2(f2);
	t1.join();
	t2.join();
}

//std::mutex m3;
//std::condition_variable cond3;
//
#include <future>

/*void*/ int factorial(std::future<int>& f/*int N*//*, int& x*/) {
	int res = 1;
	int N = f.get();

	for (int i = N; i > 1; i--) {
		res *= i;
	}
	cout << "result is: " << res << endl;
	//x = res;
	return res;
}

void BoQian3() {
	int x;
	//thread t1(factorial, 4, std::ref(x));

	std::promise<int> p;
	//std::promise<int> p2 = std::move(p); // p2 = p won't compile
	std::future<int> f = p.get_future();

	//use future to get value from child thread
	std::future<int> fu = std::async(std::launch::async,factorial, std::ref(f)/*4*/); //std::launch::deferred will not create a thread

	//....
	std::this_thread::sleep_for(chrono::milliseconds(20));
	p.set_value(4);

	x = fu.get();  //can call get() function only once
	cout << "Get from child thread: " << x << endl;
	//fu.get(); //crash
	//t1.join();
}

class A {
public:
	void f(int x, char c){}
	long g(double x) { return 0; }
	int operator()(int N) { return 0; }
};

void foo(int x){}

//using callable object. Note BoQian4 is for explanation, may not be running OK
void BoQian4() {
	A a;
	std::thread t1(a, 6); //copy_of_a() in a different thread
	std::thread t2(/*std::ref(a)*/ std::move(a), 6); // a() in a different thread. if std::move, then a is no longer usable in parent thread
	std::thread t3(A(), 6); // temp A
	std::thread t4([](int x) {return x*x; }, 6);
	std::thread t5(foo, 7);
	std::thread t6(&A::f, a, 8, 'w'); //copy_of_a.f(8,'w') in a different thread
	std::thread t7(&A::f, &a, 8, 'w'); //a.f(8,'w') in a different thread

	//std::async(std::launch::async, a, 6);
	//std::bind(a, 6);
	//std::call_once(std::once_flag, a, 6);
}

int factorial2(int N) {
	int res = 1;
	for (int i = N; i > 1; i--) {
		res *= i;
	}
	cout << "result is: " << res << endl;
	return res;
}

std::deque<std::packaged_task<int()>> task_q;
std::mutex mu5;
std::condition_variable cond5;

void thread_1() {
	std::packaged_task<int()> t;
	{
		std::unique_lock<std::mutex> locker(mu5);
		cond5.wait(locker, []() {return !task_q.empty();});
		t = std::move(task_q.front());
		task_q.pop_front();
	}
	t();
}


//packaged task
/* 
3 ways to get future
*/
//Issues to make it run when using task_q?
void BoQian5() {
	//std::thread t1(thread_1);
	//std::packaged_task<int(int)> t(factorial2);
	std::packaged_task<int()> t(bind(factorial2,6));
	//auto t = std::bind(factorial2, 6);
	std::future<int> fu = t.get_future();
	{
		std::lock_guard<std::mutex> locker(mu5);
		task_q.push_back(std::move(t));
	}
	cond5.notify_one();

	//....

	//t(6);  // In a different context
	//t();
	//int x = t.get_future().get();
	//cout << "x= " << x << endl;

	cout << fu.get();
	//t1.join();
}

void BoQianRandomEngine() {
	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine e(seed);
	cout << e() << endl; //range: [e.min(),e.max()];

	//Range [0,5]
	cout << e() % 6 << endl;
	//1. Bad quality of randomness
	//2. con only provide uniform distrubution

	std::uniform_int_distribution<int> distr(0, 5); //Range [0,5]
	cout << distr(e) << endl;

	cout << "normal distribution" << endl;
	std::normal_distribution<double> distrN(10.0, 6.0);
	vector<int> v(20);
	for (int i = 0; i < 800; i++) {
		int num = distrN(e); //convert double to int
		if (num > 0 && num < 20)
			v[num]++; //E.g. v[10] records num of times 10 appears
	}
	for (int i = 0; i < 20; i++) {
		cout << i << ": " << std::string(v[i], '*') << endl;
	}
}

void test() {
	try {
		thread t(doSomething,5,'.');
		//cin.get();
		cout << "- join fg thread" << endl;
		t.join();
	}
	catch (const exception& e) {
		cerr << "error: " << e.what();
	}
}

void ProducerConsumer() {


	//test();

	//BoQian();

	//BoQian2();

	//BoQian3();

	//BoQian4();
	
	//BoQian5();

	BoQianRandomEngine();

	//vector<int> v = { 2,3,4,5,1 };

	//Q* q = new Q();
	//Producer* p = new Producer(*q);
	//Consumer* c = new Consumer(*q);

	//thread tp(&Producer::run,p);
	//thread tc(&Consumer::run,c);

	//tp.join();
	//tc.join();

	//cout << "Thread joined" << endl;
}