#include <iostream>
#include "test.h"

using namespace std;

int blub(int a) {
	cout << "a=" << a << endl;
	return true;
}

void caller(int (*cc)(int), int b) {
	cc(b);
}

void callback_c(void) {
	caller(&blub, 5); caller(blub, 5);
	return;
}

void myPrint(int **src, int w, int h)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			cout << src[i][j] << "\t";
		}
		cout << endl;
	}
}

void move(int **src, int w, int h)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			src[i][j] = 1;
		}
	}

}

void myFree(int **src, int w, int h)
{
	for (int i = 0; i < h; i++)
	{
		free(src[i]);
	}
	free(src);
}

void testPointerOfPointer()
{
	int **src;
	int width = 10;
	int height = 5;

	src = (int **)malloc(sizeof(int *) * height);

	for (int i = 0; i < height; i++)
	{
		src[i] = (int *)malloc(sizeof(int) *  width);
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			src[i][j] = 2;
		}
	}

	myPrint(src, width, height);

	move(src, width, height);

	myPrint(src, width, height);

	myFree(src, width, height);
}

void printa(int i)
{
	cout << i << endl;
}

void printz(vector<int> v)
{
	for_each(v.begin(), v.end(), printa);
}

class Source 
{
public:
	Source() { };
	~Source() { };

	vector<vector<int>> getMySrc() { return mySrc; }
	void setMySrc(vector<vector<int>> src) { mySrc = src; }

	void printOut()
	{
		vector<vector<int>>::iterator it;
		for (it = mySrc.begin(); it != mySrc.end(); it++)
		{
			vector<int>::iterator rit;
			for (rit = it->begin(); rit != it->end(); rit++)
			{
				cout << *rit << '\t';
			}
			cout << endl;
		}
	}

	static void printx(vector<int> i)
	{
		for_each(i.begin(), i.end(), myClass::printy);
		//for_each(i.begin(), i.end(), myObject::myInnerObject);
	}

	struct myClass
	{
		static void printy(int i)
		{
			cout << i << endl;
		}

		struct myInner
		{
			void operator()(int i)
			{
				cout << i << endl;
			}
		} myInnerObject;


		void operator()(vector<int> i)
		{
			for_each(i.begin(), i.end(), printy);
			for_each(i.begin(), i.end(), myInnerObject);
		}
	} myObject;

	void printOut1()
	{
		for_each(mySrc.begin(), mySrc.end(), printx);
		//cin.get();
		for_each(mySrc.begin(), mySrc.end(), myObject);
		//cin.get();
		for_each(mySrc.begin(), mySrc.end(), printz);
		//cin.get();
	}

	void change()
	{
		vector<vector<int>>::iterator it;
		for (it = mySrc.begin(); it != mySrc.end(); it++)
		{
			vector<int>::iterator rit;
			for (rit = it->begin(); rit != it->end(); rit++)
			{
				*rit = 1;
			}
		}
	}

private:
	vector<vector<int>> mySrc;
};

void classPointerOfPointer()
{
	Source *src = new Source();

	vector<int> row;
	vector<vector<int>> rows;
	int w = 10;
	int h = 5;
	int x = 2;

	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			row.push_back(x);
		}
		rows.push_back(row);
		row.clear();
	}
	src->setMySrc(rows);

	src->printOut();

	src->change();

	src->printOut();

	src->printOut1();

	delete src;
}

int main() {
	//cout << "xxx";
	
	//testPointerOfPointer();  //2-d array, actually

	classPointerOfPointer();

	//callback_c();

	//ProducerConsumer();

	cin.get(); //getchar();

	return 0;
}