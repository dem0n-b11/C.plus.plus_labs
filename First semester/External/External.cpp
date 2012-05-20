#define _POSIX_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <string>
#include <sstream>

template<class T> void serialize(const T &object, std::ostream &stream)
{
	stream.write((char *) &object, sizeof(T));
}

template<class T> void deserialize(T &object, std::istream &stream)
{
	stream.read((char *) &object, sizeof(T));
}

template<class T> void serialize(const std::string &str, std::ostream &stream)
{
	unsigned long int size = sizeof(str);
	stream.write((char *) &size, sizeof(unsigned long int));
	stream.write((char *) &str, size);
}

template<class T> void deserialize(std::string &str, std::istream &stream)
{
	unsigned long int size;
	stream.read((char *) &size, sizeof(unsigned long int));
	stream.read((char *) &str, size);
}

template<class T> void serialize(std::vector<T> &v, std::ostream &stream)
{
	unsigned long int size = v.size();
	stream.write((char *) &size, sizeof(unsigned long int));
	for(size_t i=0;i<size;++i)
	{
		serialize(v[i], stream);
	}
}
template<class T> void deserialize(std::vector<T> &v, std::ostream &stream)
{
	v.clear();
	unsigned long int size;
	stream.read((char *) &size, sizeof(unsigned long int));
	for(size_t i=0;i<size;++i)
	{
		T tmp;
		deserialize(tmp, stream);
		v.push_back(tmp);
	}
}

const unsigned long size_of_memory = 12;

template<class T> class external
{
public:
	class file_for_sort
	{
	private:
		T value;
		unsigned long long num;
		unsigned long long count;
		unsigned file_num;
		std::string name;
		std::fstream stream;
		file_for_sort(const file_for_sort &f){};

	public:
		file_for_sort()
		{
		}
		file_for_sort(unsigned num)
		{
			file_num = num;
			std::stringstream tmp;
			tmp << "file" << file_num << ".tmp";
			name = tmp.str();
			stream.open(name.c_str(), std::ios::out | std::ios::binary);
			count = 0;
		}
		void reopen()
		{
			stream.close();
			stream.open(name.c_str(), std::ios::in | std::ios::binary);
			next_value();
		}
		void serialize_data(const std::vector<T> &v)
		{
			count = v.size();
			num = 0;
			value = v[0];
			for(unsigned long i=0;i<v.size();++i)
				serialize(v[i], stream);
		}
		void serialize_one_value(T obj)
		{
			++count;
			num = 0;
			serialize<T>(obj, stream);
		}
		T val()
		{
			return value;
		}
		bool operator>(file_for_sort &f)
		{
			return (value > f.val());
		}
		bool end_of_file()
		{
			return num>=count;
		}
		void next_value()
		{
			if(num>count)
				throw std::exception("Size error");
			++num;
			deserialize<T>(value, stream);
		}
		~file_for_sort()
		{
			stream.close();
			unlink(name.c_str());
		}
	};
protected:
	std::vector<T> data;
	std::vector<file_for_sort*> files;
	unsigned count_of_files;
	file_for_sort *final_file;
	size_t count_of_data;

	virtual void before_creating_file()=0;

	void create_file(const std::vector<T> &v)
	{
		before_creating_file();
		count_of_files++;
		std::auto_ptr<file_for_sort> tmp(new file_for_sort(count_of_files));
		files.push_back(tmp.get());
		tmp.release();
		(*(files[count_of_files-1])).serialize_data(data);
		data.clear();
	}
public:
	external()
	{
		count_of_files = 0;
		count_of_data = 0;
	}
	size_t size()
	{
		return count_of_data;
	}
	void push(T obj)
	{
		data.push_back(obj);
		++count_of_data;
		if((data.size()+1)*sizeof(T)>size_of_memory)
		{
			create_file(data);
		}
	}
	virtual size_t select_file()=0;
	virtual void change_files(){}
	virtual void delete_file(size_t i)=0;
	virtual void next_step(size_t i)=0;
	void final()
	{
		final_file = new file_for_sort(0);
		if(data.size())
		{
			create_file(data);
		}
		for(size_t i=0;i<count_of_files;++i)
			(*(files[i])).reopen();
		change_files();
		while(files.size()>0)
		{
			size_t i = select_file();
			(*(final_file)).serialize_one_value((*(files[i])).val());
			if((*(files[i])).end_of_file())
			{
				delete_file(i);
			}
			else
			{
				next_step(i);
			}
		}
		(*final_file).reopen();
	}
	T next()
	{
		T tmp = (*(final_file)).val();
		(*(final_file)).next_value();
		return tmp;
	}
	~external()
	{
		delete final_file;
		for(size_t i=0;i<files.size();++i)
			delete files[i];
	}
};

template<class T>class external_sort:public external<T>
{
private:
	struct CmpByValue {
			bool operator()(file_for_sort* a, file_for_sort* b) const { return (*a)>(*b); };
	};
	void change_files()
	{
		std::make_heap(files.begin(), files.end(), CmpByValue());
	}
	void delete_file(size_t i)
	{
		std::pop_heap(files.begin(), files.end(), CmpByValue());
		delete files[files.size()-1];
		files.pop_back();
	}
	void next_step(size_t i)
	{
		std::pop_heap(files.begin(), files.end(), CmpByValue());
		(*files[files.size()-1]).next_value();
		std::push_heap(files.begin(), files.end(), CmpByValue());
	}
	void before_creating_file()
	{
		std::sort(data.begin(), data.end());
	}
public:
	size_t select_file()
	{
		return 0;
	}
};

template<class T>class external_reverse:public external<T>
{
private:
	void before_creating_file()
	{
		std::reverse(data.begin(), data.end());
	}
	void delete_file(size_t i)
	{
		delete files[i];
		files.pop_back();
	}
	void next_step(size_t i)
	{
		(*files[i]).next_value();
	}
public:
	size_t select_file()
	{
		return files.size()-1;
	}
};

template<class T>class external_random_shuffle:public external<T>
{
private:
	void delete_file(size_t i)
	{
		delete files[i];
		std::swap(files[i], files[files.size()-1]);
		files.pop_back();
	}
	void next_step(size_t i)
	{
		(*files[i]).next_value();
	}
	void before_creating_file()
	{
		std::random_shuffle(data.begin(), data.end());
	}
public:
	external_random_shuffle()
	{
		srand(time(NULL));
	}
	size_t select_file()
	{
		return rand()%files.size();
	}
};

bool test_sort(size_t data_size)
{
	external_sort<int> sort;
	for(size_t i=0;i<data_size;++i)
	{
		int tmp = rand();
		sort.push(tmp);
	}
	sort.final();
	int t1, t2;
	t1 = sort.next();
	for(size_t i=1;i<data_size;++i)
	{
		t2 = sort.next();
		if(t2<t1)
			return false;
		else
			t1 = t2;
	}
	return true;
}

bool test_reverse(size_t data_size)
{
	external_reverse<int> reverse;
	std::vector<int> data;
	for(size_t i=0;i<data_size;++i)
	{
		int tmp = rand();
		data.push_back(tmp);
		reverse.push(tmp);
	}
	reverse.final();
	for(size_t i=data_size;i>0;--i)
	{
		int tmp;
		tmp = reverse.next();
		if(tmp!=data[i-1])
			return false;
	}
	return true;
}

void main()
{
	std::cout << "External sort: ";
	if(test_sort(1000))
		std::cout << "OK!\n";
	else
		std::cout << "FAIL!\n";
	std::cout << "External reverse: ";
	if(test_sort(1000))
		std::cout << "OK!\n";
	else
		std::cout << "FAIL!\n";
	system("pause");
}
