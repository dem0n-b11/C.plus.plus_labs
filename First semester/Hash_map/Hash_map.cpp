#include <iostream>
#include <ctime>
#include <list>
#include <vector>
#include <string>
#include <map>

template<class T>class standart_hash;

template<>class standart_hash<int>
{
private:
	size_t hash_a, hash_b, hash_m;
public:
	size_t operator()(const long long &key)
	{
		return ((key*hash_a +hash_b)%hash_m);
	}
	standart_hash(size_t table_size)
	{
		srand((size_t)time(NULL));
		hash_a = rand()+1;
		hash_b = rand()+1;
		hash_m = table_size;
	}
};

template<>class standart_hash<std::string>
{
private:
	size_t hash_prime, hash_m;
public:
	size_t operator()(const std::string &key)
	{
		size_t pw = hash_prime;
		size_t h = 0;
		for(size_t i=0;i<key.length();++i)
		{
			h += key[i]*pw;
			pw *= hash_prime;
		}
		return (h%hash_m);
	}
	standart_hash(size_t table_size)
	{
		hash_prime = 3457;
		hash_m = table_size;
	}
};

template<class TKey, class TVal, class H=standart_hash<TKey>>class hash_map
{
public:
	typedef typename std::list<std::pair<const TKey, TVal>>::iterator iterator;
	typedef typename std::list<std::pair<const TKey, TVal>>::const_iterator const_iterator;
private:
	std::list<std::pair<const TKey, TVal>> pairs;
	std::vector<std::vector<iterator>> table;
	H hasher;
	size_t table_size;
	size_t size_of_map;
	void insert_into_ht(const std::pair<const TKey, TVal> &p)
	{
		pairs.push_back(p);
		size_t h = hasher(p.first);
		table[h].push_back(--pairs.end());
		if(table[h].size()>3)
			rehash();
		++size_of_map;
	}
	bool search(const TKey &key)
	{
		size_t h = hasher(key);
		for(size_t i=0;i<table[h].size();++i)
			if((*table[h][i]).first==key)
				return true;
		return false;
	}
	void rehash()
	{
		table_size *= 2;
		H hasher1(table_size);
		std::vector<std::vector<iterator>> table1;
		table1.resize(table_size);
		for(std::list<std::pair<const TKey, TVal>>::iterator i=pairs.begin();i!=pairs.end();++i)
		{
			size_t h = hasher1((*i).first);
			table1[h].push_back(i);
		}
		std::swap(table, table1);
		std::swap(hasher, hasher1);
	}
public:
	hash_map(): table_size(1024), hasher(1024), size_of_map(0)
	{
		table.resize(table_size);
	}
	template<class Iter>hash_map(Iter first, Iter last): table_size(1024), hasher(1024), size_of_map(0)
	{
		table.resize(table_size);
		while(first!=last)
		{
			insert_into_ht(*first);
			++first;
		}
	}
	hash_map &operator=(hash_map<TKey, TVal> &m)
	{
		hash_map<TKey, TVal> tmp(m.begin(), m.end());
		swap(tmp);
	}
	hash_map(hash_map<TKey, TVal> &m): table_size(1024), hasher(1024), size_of_map(0)
	{
		hash_map<TKey, TVal> tmp(m.begin(), m.end());
		swap(tmp);
	}
	void swap(hash_map<TKey, TVal> &m)
	{
		std::swap(pairs, m.pairs);
		std::swap(table, m.table);
		std::swap(hasher, m.hasher);
		std::swap(table_size, m.table_size);
		std::swap(size_of_map, m.size_of_map);
	}
	bool insert(const std::pair<TKey, TVal> &p)
	{
		if(!count(p.first))
		{
			insert_into_ht(p);
			return true;
		}
		else
			return false;
	}
	void clear()
	{
		pairs.clear();
		for(size_t i=0;i<table.size();++i)
			table[i].clear();
	}
	TVal operator[](const TKey &key)
	{
		size_t h = hasher(key);
		for(size_t i=0;i<table[h].size();++i)
			if((*table[h][i]).first==key)
				return (*table[h][i]).second;
		insert_into_ht(std::make_pair(key, TVal()));
		return pairs.back().second;
	}
	size_t count(const TKey &key)
	{
		if(search(key))
			return 1;
		return 0;
	}
	iterator find(const TKey &key)
	{
		size_t h = hasher(key);
		for(size_t i=0;i<table[h].size();++i)
			if((*table[h][i]).first==key)
				return table[h][i];
		return pairs.end();
	}
	iterator begin()
	{
		return pairs.begin();
	}
	iterator end()
	{
		return pairs.end();
	}
	void erase(const TKey &key)
	{
		size_t h = hasher(key);
		for(size_t i=0;i<table[h].size();++i)
			if((*table[h][i]).first==key)
			{
				pairs.erase(table[h][i]);
				std::swap(table[h][i], table[h][table[h].size()-1]);
				table[h].pop_back();
				--size_of_map;
			}
	}
	void erase(const_iterator iter)
	{
		erase((*iter).first);
	}
};

void main()
{
	hash_map<int, int> hm;
	std::map<int, int> m;
	size_t start = clock();
	for(int i=0;i<10000;++i)
		hm.insert(std::make_pair(i, i));
	std::cout << "Hash map function 'insert': " << (double)(clock()-start)/CLOCKS_PER_SEC << "   ";

	start = clock();
	for(int i=0;i<10000;++i)
		m.insert(std::make_pair(i, i));
	std::cout << "STL map function 'insert': " << (double)(clock()-start)/CLOCKS_PER_SEC << "\n";
	//---------------------
	int tmp;
	start = clock();
	for(int i=0;i<10000;++i)
	{
		tmp = hm[i];
		if(tmp!=i)
			std::cout << "Operator[] fail!!!\n";
	}
	std::cout << "Hash map operator '[]': " << (double)(clock()-start)/CLOCKS_PER_SEC << "   ";

	start = clock();
	for(int i=0;i<10000;++i)
	{
		tmp = m[i];
		if(tmp!=i)
			std::cout << "Operator[] fail!!!\n";
	}
	std::cout << "STL map operator '[]': " << (double)(clock()-start)/CLOCKS_PER_SEC << "\n";
	//---------------------
	start = clock();
	for(int i=0;i<10000;++i)
	{
		tmp = (*hm.find(i)).second;
		if(tmp!=i)
			std::cout << "Wite fail!!!\n";
	}
	std::cout << "Hash map function 'find': " << (double)(clock()-start)/CLOCKS_PER_SEC << "   ";

	start = clock();
	for(int i=0;i<10000;++i)
		tmp = (*m.find(i)).second;
	std::cout << "STL map function 'find': " << (double)(clock()-start)/CLOCKS_PER_SEC << "\n";
	//---------------------
	start = clock();
	for(int i = 0;i<10000;++i)
		if(i%2)
			hm.erase(i);
	std::cout << "Hash map function 'erase': " << (double)(clock()-start)/CLOCKS_PER_SEC << "   ";

	start = clock();
	for(int i = 0;i<10000;++i)
		if(i%2)
			m.erase(i);
	std::cout << "STL map function 'erase': " << (double)(clock()-start)/CLOCKS_PER_SEC << "\n";
	for(int i=0;i<10000;++i)
		if(i%2!=1)
			if(hm[i]!=i)
			{
				std::cout << "Erase fail!!!\n";
				break;
			}
			hm.erase(hm.begin());
	//---------------------
	system("pause");
}