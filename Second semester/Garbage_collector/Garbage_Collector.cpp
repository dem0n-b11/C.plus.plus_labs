#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <typeinfo.h>
#include <set>
#include <stack>
#include <assert.h>

size_t g_mem_usage = 0;

void *::operator new(size_t size) {
	void *p = malloc(size + sizeof(size_t));
	*(size_t*)p = size;
	g_mem_usage += size;
	return (size_t*)p + 1;
}

void ::operator delete(void *p) {
	size_t* smart_point = (size_t*) p;
	g_mem_usage -= *--smart_point;
	free(smart_point);
}

template<class Graph> class DFS
{
private:
	typedef typename Graph::Vertex value_type;
	typedef typename Graph::Links_container container;
	std::set<value_type> marked_set;
	Graph *graph;
	std::vector<value_type> marked_vector;
	void process(value_type st) {
		std::stack<value_type> stack;
		stack.push(st);
		while(stack.size() > 0) {
			value_type tmp = stack.top();
			stack.pop();
			container links = (*graph).return_links(*tmp);
			marked_set.insert(tmp);
			for(std::vector<value_type>::iterator i = links.begin(); i != links.end(); ++i) {
				if(marked_set.find(*i) == marked_set.end()) {
					stack.push(*i);
				}
			}
		}
	}
public:
	DFS(Graph &g) {
		graph = &g;
	}
	void start(value_type st) {
		clear();
		process(st);
		marked_vector.insert(marked_vector.end(), marked_set.begin(), marked_set.end());
		marked_set.clear();
	}
	void clear() {
		marked_set.clear();
		marked_vector.clear();
	}
	std::vector<value_type> return_marked() {
		return marked_vector;
	}
};

struct Parametres {
	bool local;
	bool marked;
	Parametres(bool mark, bool loc) {
		marked = mark;
		local = loc;
	}
};

class SmartObject {
public:
	std::map<SmartObject *, Parametres>::iterator my_iter;
	SmartObject();
	virtual std::vector<SmartObject *> return_pointers() = 0 {};
	void *operator new(size_t size);
	virtual ~SmartObject() = 0;
};

class GCollector {
	friend class SmartObject;
private:
	GCollector(){}
	GCollector(const GCollector &tmp){}
	GCollector &operator=(const GCollector &tmp){}
	std::map<SmartObject *, Parametres> objects;
	void reg_local_obj(SmartObject *obj) {
		//std::cout << "SmartObject registered!\n";
		objects.insert(std::make_pair(obj, Parametres(false, true)));
	}
	void reg_heap_obj(SmartObject *obj) {
		reg_local_obj(obj);
		std::map<SmartObject *, Parametres>::iterator tmp = objects.find(obj);
		//std::cout << "Last SmartObject is in heap!\n";
		tmp->second.local = false;
	}
	void unreg_obj(SmartObject *obj) {
		//std::cout << "SmartObject unregistered!\n";
		std::map<SmartObject *, Parametres>::iterator tmp = objects.find(obj);
		objects.erase(tmp);
	}
	bool is_reg(SmartObject *obj) {
		return (objects.find(obj) != objects.end());
	}
public:
	typedef SmartObject* Vertex;
	typedef std::vector<Vertex> Links_container;
	static GCollector &instance() {
		static GCollector tmp;
		return tmp;
	}
	std::vector<SmartObject *> return_links(SmartObject &obj) {
		return obj.return_pointers();
	}
	void run() {
		for(std::map<SmartObject *, Parametres>::iterator i = objects.begin(); i != objects.end(); ++i) {
			if(i->second.local) {
				DFS<GCollector> _dfs(this->instance());
				_dfs.start(i->first);
				std::vector<SmartObject *> tmp = _dfs.return_marked();
				for(std::vector<SmartObject *>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
					std::map<SmartObject *, Parametres>::iterator fnd = objects.find(*i);
					if(fnd == objects.end())
						throw std::runtime_error("Incorrect work of GCollector. Some object is't register.");
					fnd->second.marked = true;
				}
			}
		}
		for(std::map<SmartObject *, Parametres>::iterator i = objects.begin(); i != objects.end();) {
			if(!(*i).second.marked) {
				delete i++->first;
				//std::cout << "SmartObject in heap is deleted!\n";
			} else {
				i->second.marked = false;
				++i;
			}
		}
		//std::cout << "Mr. Proper finished!\n";
	}
	~GCollector() {
		for(std::map<SmartObject *, Parametres>::iterator i = objects.begin(); i != objects.end();) {
			if(!i->second.local) {
				delete i++->first;
			} else {
				++i;
			}
		}
		objects.clear();
	}
};

GCollector &g_collector = GCollector::instance();

SmartObject::SmartObject() {
	if(!(g_collector.is_reg(this)))
		g_collector.reg_local_obj(this);
}

SmartObject::~SmartObject() {
	g_collector.unreg_obj(this);
	//std::cout << "SmartObject destructor!\n";
}

void *SmartObject::operator new(size_t size) {
	void *tmp = ::operator new(size);
	g_collector.reg_heap_obj(static_cast<SmartObject *>(tmp));
	return tmp;
}

class BadBadObject: public SmartObject {
private:
	int *i;
public:
	BadBadObject() {
		i = (int *)malloc(sizeof(int) * 100);
	}
	std::vector<SmartObject *> return_pointers() {
		return std::vector<SmartObject *>();
	}
	~BadBadObject() {
		free(i);
		//std::cout << "BadBadObject destructor!\n";
	}
};

class BadObject: public SmartObject {
public:
	BadBadObject *tmp;
	BadObject() {
		tmp = new BadBadObject();
	}
	std::vector<SmartObject *> return_pointers() {
		std::vector<SmartObject *> l;
		l.push_back(tmp);
		return l;
	}
	~BadObject() {
		//std::cout << "BadObject destructor!\n";
	}
};

class VeryBadObject:public SmartObject {
public:
	std::vector<SmartObject *> return_pointers() {
		std::vector<SmartObject *> tmp;
		tmp.push_back(obj);
		return tmp;
	}
	VeryBadObject *obj;

};

class NotBadObject:public SmartObject {
private:
	VeryBadObject *obj1;
	VeryBadObject *obj2;
public:
	std::vector<SmartObject *> return_pointers() {
		std::vector<SmartObject *> tmp;
		tmp.push_back(obj1);
		tmp.push_back(obj2);
		return tmp;
	}
	NotBadObject() {
		obj1 = new VeryBadObject();
		obj2 = new VeryBadObject();
		obj1->obj = obj2;
		obj2->obj = obj1;
	}
};

void test1() {
	BadObject tmp;
	g_collector.run();
	assert(g_mem_usage != 0);
}

void test2() {
	if(1) {
		BadObject tmp;
	}
	assert(g_mem_usage != 0);
	g_collector.run();
	assert(g_mem_usage == 0);
}

void test3() {
	if(1) {
		NotBadObject obj;
		g_collector.run();
		assert(g_mem_usage != 0);
	}
	g_collector.run();
	assert(g_mem_usage == 0);
}

int main() {
	g_mem_usage = 0;
	test1();
	test2();
	test3();
	system("pause");
	return 0;
}