#include <iostream>
#include <map>
#include <stack>
#include <set>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <list>

template<class Graph> std::vector<typename Graph::_Vertex> dfs(const Graph &graph, typename Graph::_Vertex st) {
	typedef typename Graph::_Vertex value_type;
	std::set<value_type> marked_set;
	std::vector<value_type> marked_vector;
	std::set<value_type> black_set;
	std::stack<value_type> stack;
	stack.push(st);
	while(stack.size() > 0) {
		value_type tmp = stack.top();
		if(black_set.find(tmp) != black_set.end()) {
			stack.pop();
			continue;
		}
		if(marked_set.find(tmp) != marked_set.end()) {
			black_set.insert(tmp);
			marked_vector.push_back(tmp);
			stack.pop();
		}
		marked_set.insert(tmp);
		auto links(graph.return_links(tmp));
		for(auto i = links.begin(); i != links.end(); ++i) {
		if(marked_set.find(*i) == marked_set.end()) {
			stack.push(*i);
			}
		}
	}
	return marked_vector;
}

template<class Graph> std::vector<typename Graph::_Vertex> topologicalSort(const Graph &graph) { //const
	typedef typename Graph::_Vertex value_type;
	std::set<value_type> used;
	std::vector<value_type> sorted;
	for(auto i = graph.begin(); i != graph.end(); ++i) {
		if(used.find(*i) == used.end()) {
			std::vector<value_type> tmp = dfs(graph, *i);
			for(auto i = tmp.begin(); i != tmp.end(); ++i) {
				if(used.find(*i) == used.end())
					sorted.push_back(*i);
			}
			used.insert(tmp.begin(), tmp.end());
		}
	}
	std::reverse(sorted.begin(), sorted.end());
	return sorted;
}

class Equation {
private:
	std::string var1, var2;
	std::vector<int> logic_table;
public:
	Equation(std::string v1, std::string v2, const std::string &ltable): var1(std::move(v1)), var2(std::move(v2)) {
		if(ltable.size() != 4)
			throw std::runtime_error("Incorrect input of logic table. Size of table isn't 4.");
		for(size_t i = 0; i < 4; ++i) {
			if(ltable[i] != '0' && ltable[i] != '1')
				throw std::runtime_error("Incorrect answer of logic equalation.");
			logic_table.push_back((int)ltable[i] - (int)'0');
		}
	}
	const std::string& Var1() const {
		return var1;
	}
	const std::string& Var2() const {
		return var2;
	}
	int answer(int i) const {
		if(i > 3)
			throw std::runtime_error("Incorrect value of answer.");
		return logic_table[i];
	}
};

class Var {
private:
	std::string _name;
	bool _negative;
public:
	Var(){}
	Var(std::string n, bool neg): _name(std::move(n)) {
		_negative = neg;
	}
	const std::string& name() const {
		return _name;
	}
	bool negative() const {
		return _negative;
	}
	bool operator==(const Var &v) const {
		return (name() == v.name() && negative() == v.negative());
	}
	bool operator!=(const Var &v) const {
		return !(*this == v);
	}
	Var operator!() {
		_negative = !_negative;
		return *this;
	}
};

class KNF {
private:
	std::vector<Equation> eqs;
	std::vector<std::pair<Var, Var>> knf;
public:
	explicit KNF(std::vector<Equation> e): eqs(std::move(e)) {
		for(auto i = eqs.begin(); i != eqs.end(); ++i) {
			knf.push_back(std::make_pair(Var(i->Var1(), true), Var(i->Var1(), false)));
			knf.push_back(std::make_pair(Var(i->Var2(), true), Var(i->Var2(), false)));
			if(i->answer(0) == 0) { // 1 1
				knf.push_back(std::make_pair(Var(i->Var1(), true), Var(i->Var2(), true)));
			}
			if(i->answer(1) == 0) { // 1 0
				knf.push_back(std::make_pair(Var(i->Var1(), true), Var(i->Var2(), false)));
			}
			if(i->answer(2) == 0) { // 0 1
				knf.push_back(std::make_pair(Var(i->Var1(), false), Var(i->Var2(), true)));
			}
			if(i->answer(3) == 0) { // 0 0
				knf.push_back(std::make_pair(Var(i->Var1(), false), Var(i->Var2(), false)));
			}
		}
	}
	std::vector<std::pair<Var, Var>>::iterator begin() {
		return knf.begin();
	}
	std::vector<std::pair<Var, Var>>::const_iterator begin() const {
		return knf.begin();
	}
	std::vector<std::pair<Var, Var>>::iterator end() {
		return knf.end();
	}
	std::vector<std::pair<Var, Var>>::const_iterator end() const {
		return knf.end();
	}
	size_t size() const {
		return knf.size();
	}	
};

class Vertex {
private:
	std::vector<Var> vars;
	std::vector<Vertex *> out_links;
	std::vector<Vertex *> in_links;
public:
	Vertex() {}
	Vertex(const Vertex &v): vars(v.vars), out_links(v.out_links), in_links(v.in_links) {
	}
	void union_with(Vertex *v) {
		vars.insert(vars.end(), v->begin(), v->end());
		auto v_o_links = v->links_out();
		for(auto i = v_o_links.begin(); i != v_o_links.end(); ++i) {
			if(*i == this) {
				delete_in_link(v);
				continue;
			}
			auto tmp = std::find(out_links.begin(), out_links.end(), *i);
			if(tmp == out_links.end()) {
				this->add_out_link(*i);
				(*i)->add_in_link(this);
			}
			(*i)->delete_in_link(v);
		}
		auto v_i_links = v->links_in();
		for(auto i = v_i_links.begin(); i != v_i_links.end(); ++i) {
			if(*i == this) {
				(*i)->delete_out_link(v);
				continue;
			}
			auto tmp = std::find(in_links.begin(), in_links.end(), *i);
			if(tmp == in_links.end()) {
				this->add_in_link(*i);
				(*i)->add_out_link(this);
			}
			(*i)->delete_out_link(v);
		}
		v->clear();
	}
	void clear() {
		vars.clear();
		out_links.clear();
		in_links.clear();
	}
	void delete_out_link(Vertex *v) {
		auto tmp = std::find(out_links.begin(), out_links.end(), v);
		if(tmp == out_links.end())
			throw std::runtime_error("Link not fount.");
		out_links.erase(tmp);
	}
	void delete_in_link(Vertex *v) {
		auto tmp = std::find(in_links.begin(), in_links.end(), v);
		if(tmp == in_links.end())
			throw std::runtime_error("Link not fount.");
		in_links.erase(tmp);
	}
	void add_var(const Var &v) {
		vars.push_back(v);
	}
	void add_out_link(Vertex *v) {
		out_links.push_back(v);
	}
	void add_in_link(Vertex *v) {
		in_links.push_back(v);
	}
	const std::vector<Vertex *>& links_out() const {
		return out_links;
	}
	const std::vector<Vertex *>& links_in() const {
		return in_links;
	}
	std::vector<Var>::iterator begin() {
		return vars.begin();
	}
	std::vector<Var>::iterator end() {
		return vars.end();
	}
	std::vector<Var>::const_iterator begin() const {
		return vars.begin();
	}
	std::vector<Var>::const_iterator end() const {
		return vars.end();
	}
	bool operator==(const Vertex &v) const {
		for(auto i = vars.begin(); i != vars.end(); ++i) {
			if(std::find(v.vars.begin(), v.vars.end(), *i) == v.vars.end())
				return false;
		}
		return true;
	}
	bool operator!=(const Vertex &v) const {
		return !(*this == v);
	}
	bool belong(const Var &v) const { //belongs
		if(std::find(this->begin(), this->end(), v) != this->end())
			return true;
		return false;
	}
};

class Graph {
private:
	std::list<Vertex> graph;
	std::vector<Vertex *> pointers_to_vertexes;
	bool transpose;
	bool no_solution;
	void insert(Vertex &tmp_ver1, Vertex &tmp_ver2) {
		auto pos_v1 = std::find(graph.begin(), graph.end(), tmp_ver1);
		if(pos_v1 == graph.end()) {
			pos_v1 = graph.insert(graph.end(), tmp_ver1);
		}
		auto pos_v2 = std::find(graph.begin(), graph.end(), tmp_ver2);
		if(pos_v2 == graph.end()) {
			pos_v2 = graph.insert(graph.end(), tmp_ver2);
		}
		pos_v1->add_out_link(&*pos_v2);
		pos_v2->add_in_link(&*pos_v1);
	}
public:
	typedef Vertex* _Vertex;
	explicit Graph(KNF &knf) {
		transpose = false;
		no_solution = false;
		for(auto i = knf.begin(); i != knf.end(); ++i) {
			Vertex tmp_ver1;
			tmp_ver1.add_var(Var(i->first.name(), !i->first.negative()));
			Vertex tmp_ver2;
			tmp_ver2.add_var(i->second);
			insert(tmp_ver1, tmp_ver2);
			Vertex tmp_neg_ver1;
			tmp_neg_ver1.add_var(i->first);
			Vertex tmp_neg_ver2;
			tmp_neg_ver2.add_var(Var(i->second.name(), !i->second.negative()));
			insert(tmp_neg_ver2, tmp_neg_ver1);
		}
		for(Vertex &v: graph) {
			pointers_to_vertexes.push_back(&v);
		}
	}
	const std::vector<Vertex *>& return_links(Vertex *v) const {
		if(!transpose)
			return v->links_out();
		else
			return v->links_in();
	}
	std::vector<Vertex *>::iterator begin() {
		return pointers_to_vertexes.begin();
	}
	std::vector<Vertex *>::iterator end() {
		return pointers_to_vertexes.end();
	}
	std::vector<Vertex *>::const_iterator begin() const {
		return pointers_to_vertexes.begin(); // vertices
	}
	std::vector<Vertex *>::const_iterator end() const {
		return pointers_to_vertexes.end();
	}
	void transp() {
		transpose = !transpose;
	}
	bool is_transponse() const {
		return transpose;
	}
	void condanse() {
		bool transp_now = transpose;
		try {
			auto sorted = topologicalSort<Graph>(*this);
			transp();
			while(sorted.size() > 0) {
				auto tmp = dfs<Graph>(*this, *(sorted.begin()));
				auto main = *(tmp.end() - 1);
				auto tmp_iter = std::find(sorted.begin(), sorted.end(), *(tmp.end() - 1));
				if(tmp_iter != sorted.end())
					sorted.erase(tmp_iter);
				for(auto i = tmp.begin(); i != (tmp.end() - 1); ++i) {
					if(std::find(sorted.begin(), sorted.end(), *i) == sorted.end()) {
						continue;
					}
					main->union_with(*i);
					main = main;
					pointers_to_vertexes.erase(std::find(pointers_to_vertexes.begin(), pointers_to_vertexes.end(), *i));
					graph.erase(std::find(graph.begin(), graph.end(), **i));
					sorted.erase(std::find(sorted.begin(), sorted.end(), *i));
				}
			}
			for(auto i = this->begin(); i != this->end(); ++i) {
				for(auto j = (*i)->begin(); j != (*i)->end(); ++j) {
					if(std::find((*i)->begin(), (*i)->end(), Var(j->name(), !(j->negative()))) != (*i)->end()) {
						no_solution = true;
					}
				}
			}
			transp();
		} catch(...) {
			transpose = transp_now;
			throw;
		}
	}
	std::vector<std::pair<std::string, bool>> solution() {
		std::vector<std::pair<Var, bool>> solution;
		std::vector<std::pair<std::string, bool>> name_sol;
		if(no_solution)
			return name_sol;
		auto sorted = topologicalSort(*this);
		while(sorted.size() > 0) {
			auto first = sorted.begin();
			for(auto i = (*first)->begin(); i != (*first)->end(); ++i) {
				solution.push_back(std::make_pair((*i), false));
			}
			auto var = Var((*first)->begin()->name(), !((*first)->begin()->negative()));
			auto pos = sorted.begin();
			for(auto i = sorted.begin() + 1; i != sorted.end(); ++i) {
				if((*i)->belong(var)) {
					pos = i;
					break;
				}
			}
			sorted.erase(pos);
			sorted.erase(first);
		}
		for(auto i = solution.begin(); i != solution.end(); ++i) {
			if(i->first.negative()) {
				i->first = !i->first;
				i->second = !i->second;
			}
			name_sol.push_back(std::make_pair(i->first.name(), i->second));
		}
		return name_sol;
	}
};

int main() {
	size_t n;
	std::cin >> n;
	std::vector<Equation> eqs;
	for(size_t i = 0; i < n; ++i) {
		std::string x1, x2, a;
		std::cin >> x1 >> x2 >> a;
		eqs.push_back(Equation(x1, x2, a));
	}
	KNF knf(eqs);
	Graph graph(knf);
	graph.condanse();
	auto sol = graph.solution();
	if(sol.size() == 0) {
		std::cout << "No solution";
		return 0;
	}
	for(auto i = sol.begin(); i != sol.end(); ++i) {
		std::cout << i->first << " ";
		std::cout << i->second << "\n";
	}
	return 0;
}
