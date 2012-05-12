#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <limits>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <memory>
#include <assert.h>

template<class Graph> std::map<typename Graph::VertexType, typename Graph::EdgeWeightType>
		path_weights(const Graph &graph, const typename Graph::VertexType &start, const typename Graph::EdgeWeightType &inf) {
	assert(graph.includes(start));
	std::map<typename Graph::VertexType, typename Graph::EdgeWeightType> dist;
	std::set<typename Graph::VertexType> used;
	typename Graph::VertexType next_vertex;
	std::transform(graph.begin(), graph.end(), std::inserter(dist, dist.end()), [&inf](typename Graph::VertexType v) {
		return std::make_pair(v, inf);});
	dist[start] = 0;
	next_vertex = start;
	while(used.size() < graph.size()) {
		auto links = graph.return_links(next_vertex);
		size_t minimal = inf;
		used.insert(next_vertex);
		std::for_each(links.begin(), links.end(), [&dist, &next_vertex](std::pair<typename Graph::VertexType, typename Graph::EdgeWeightType> edge) {
			dist[edge.first] = std::min(dist[edge.first], dist[next_vertex] + edge.second);});
		std::for_each(dist.begin(), dist.end(), [&dist, &minimal, &next_vertex, &used](std::pair<typename Graph::VertexType, typename Graph::EdgeWeightType> dist_pair) {
			if(dist_pair.second < minimal && used.find(dist_pair.first) == used.end()) {
				next_vertex = dist_pair.first;
				minimal = dist_pair.second;
			}});
		if(minimal == inf)
			break;
	}
	return dist;
}

template<class T> class BaseVertex {
protected:
	const T info_about_vertex;
public:
	explicit BaseVertex(T inf): info_about_vertex(std::move(inf)) {
	}
	const T& info() const {
		return info_about_vertex;
	}
	virtual void build_links(const std::map<size_t, std::shared_ptr<BaseVertex<T>>> &vertices) = 0;
	virtual std::vector<std::pair<std::shared_ptr<BaseVertex<T>>, size_t>> return_links() const = 0;
	virtual ~BaseVertex() {}
};

template<class T> class UniversalVertex: public BaseVertex<T> {
private:
	std::vector<std::pair<std::shared_ptr<BaseVertex<T>>, size_t>> links;
	std::vector<std::pair<size_t, size_t>> tmp_links;
	bool is_linked;
public:
	explicit UniversalVertex(T inf): BaseVertex<T>(std::move(inf)) {
		is_linked = true;
	}
	UniversalVertex(T inf, std::vector<std::pair<size_t, size_t>> edges): BaseVertex<T>(std::move(inf)), tmp_links(std::move(edges)) {
		is_linked = false;
	}
	UniversalVertex(T inf, std::vector<std::pair<std::shared_ptr<BaseVertex<T>>, size_t>> edges): BaseVertex<T>(std::move(inf)), links(std::move(edges)) {
		is_linked = true;
	}
	std::vector<std::pair<std::shared_ptr<BaseVertex<T>>, size_t>> return_links() const {
		if(!is_linked)
			throw std::runtime_error("This vertex isn't yet linked.");
		return links;
	}
	void add_link(const std::shared_ptr<BaseVertex<T>> &v, size_t w) {
		links.push_back(std::make_pair(v, w));
	}
	void build_links(const std::map<size_t, std::shared_ptr<BaseVertex<T>>> &vertices) {
		if(is_linked)
			throw std::runtime_error("This vertex is already linked.");
		std::transform(tmp_links.begin(), tmp_links.end(), std::inserter(links, links.end()), [&vertices](std::pair<size_t, size_t> id) {
			auto tmp = vertices.find(id.first);
			if(tmp == vertices.end())
				throw std::runtime_error("Not found this vertex in map.");
			return std::make_pair(tmp->second, id.second);});
		is_linked = true;
		tmp_links.clear();
	}
};

template<class T, size_t count_of_links> class ParticularVertex: public BaseVertex<T> {
private:
	std::pair<std::shared_ptr<BaseVertex<T>>, size_t> links[count_of_links];
	std::vector<std::pair<size_t, size_t>> tmp_links;
	bool is_linked;
public:
	ParticularVertex(T inf, std::vector<std::pair<size_t, size_t>> edges): BaseVertex<T>(std::move(inf)), tmp_links(std::move(edges)) {
		if(tmp_links.size() != count_of_links)
			throw std::runtime_error("Incorrect count of edges.");
		is_linked = false;
	}
	ParticularVertex(T inf, const std::vector<std::pair<std::shared_ptr<BaseVertex<T>>, size_t>> &edges): BaseVertex<T>(std::move(inf)) {
		if(edges.size() != count_of_links)
			throw std::runtime_error("Incorrect count of edges.");
		for(size_t i = 0; i < count_of_links; ++i)
			links[i] = edges[i];
		is_linked = true;
	}
	std::vector<std::pair<std::shared_ptr<BaseVertex<T>>, size_t>> return_links() const {
		if(!is_linked)
			throw std::runtime_error("This vertex isn't yet linked.");
		return std::vector<std::pair<std::shared_ptr<BaseVertex<T>>, size_t>>(links, links + count_of_links);
	}
	void build_links(const std::map<size_t, std::shared_ptr<BaseVertex<T>>> &vertices) {
		if(is_linked)
			throw std::runtime_error("This vertex is already linked.");
		for(size_t i = 0; i < count_of_links; ++i) {
			auto tmp = vertices.find(tmp_links[i].first);
			if(tmp == vertices.end())
				throw std::runtime_error("Not found this vertex in map.");
			links[i] = std::make_pair(tmp->second, tmp_links[i].second);
		}
		is_linked = true;
		tmp_links.clear();
	}
};

class Graph {
private:
	std::vector<std::shared_ptr<BaseVertex<size_t>>> vertices;
public:
	typedef std::shared_ptr<BaseVertex<size_t>> VertexType;
	typedef size_t EdgeWeightType;
	typedef typename std::vector<std::shared_ptr<BaseVertex<size_t>>>::const_iterator const_iterator;
	typedef typename std::vector<std::shared_ptr<BaseVertex<size_t>>>::iterator iterator;
	explicit Graph(std::vector<std::shared_ptr<BaseVertex<size_t>>> v): vertices(std::move(v)) {}
	size_t size() const {
		return vertices.size();
	}
	bool includes(const std::shared_ptr<BaseVertex<size_t>> &v) const {
		return !(std::find(vertices.begin(), vertices.end(), v) == vertices.end());
	}
	std::vector<std::pair<std::shared_ptr<BaseVertex<size_t>>, size_t>> return_links(const std::shared_ptr<BaseVertex<size_t>> &v) const {
		if(!includes(v))
			throw std::runtime_error("Graph hasn't this vertex.");
		return v->return_links();
	}
	iterator begin() {
		return vertices.begin();
	}
	iterator end() {
		return vertices.end();
	}
	const_iterator begin() const {
		return vertices.begin();
	}
	const_iterator end() const {
		return vertices.end();
	}
};

/*void test() { --- Сам тест
	std::vector<std::shared_ptr<BaseVertex<size_t>>> vertices;
	size_t n, s, f;
	std::cin >> n >> s >> f;
	std::map<size_t, std::shared_ptr<BaseVertex<size_t>>> num_of_vertices;
	std::vector<std::vector<std::pair<size_t, size_t>>> tmp_edges;
	for(size_t i = 0; i < n; ++i) {
		tmp_edges.push_back(std::vector<std::pair<size_t, size_t>>());
		for(size_t j = 0; j < n; ++j) {
			int in;
			std::cin >> in;
			if(in != -1)
				tmp_edges[i].push_back(std::make_pair(j + 1, (size_t)in));
		}
		if(tmp_edges[i].size() == 2) {
			std::shared_ptr<BaseVertex<size_t>> tmp_v(new ParticularVertex<size_t, 2>(i + 1, tmp_edges[i]));
			vertices.push_back(tmp_v);
			num_of_vertices.insert(std::make_pair(i + 1, tmp_v));
		} else {
			std::shared_ptr<BaseVertex<size_t>> tmp_v(new UniversalVertex<size_t>(i + 1, tmp_edges[i]));
			vertices.push_back(tmp_v);
			num_of_vertices.insert(std::make_pair(i + 1, tmp_v));
		}
	}
	std::for_each(vertices.begin(), vertices.end(), [&num_of_vertices](std::shared_ptr<BaseVertex<size_t>> v) {
		v->build_links(num_of_vertices);});
	Graph graph(vertices);
	auto weights = path_weights(graph, vertices[s - 1], std::numeric_limits<std::size_t>::max());
	if(weights[vertices[f - 1]] == std::numeric_limits<std::size_t>::max())
		std::cout << -1 << std::endl;
	else
		std::cout << weights[vertices[f - 1]] << std::endl;
}*/

int main() {
	//test(); --- Запуск теста
	return 0;
}
