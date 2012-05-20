#include <iostream>
#include <vector>

template<class T> class standart_func_elem
{
public:
	T operator()(T x)
	{
		return x;
	}
};

template<
	class TVal, class FunForSeg, class TRes = TVal, class FunForElem = standart_func_elem<TRes>>
class segments_tree
{
private:
	std::vector<TVal> data;
	std::vector<TRes> tree;
	FunForElem FuncElem;
	FunForSeg FuncSeg;
	TRes neutral;
	size_t last_pos;
	size_t max_pos;
	void build(size_t pos, size_t left, size_t right)
	{
		tree.resize(4*data.size()+1);
		if(pos==1)
			tree.resize(4*data.size()+1);
		if(left==right)
			tree[pos] = FuncElem(data[left]);
		else
		{
			size_t middle = (left+right)/2;
			build(pos*2, left, middle);
			build(pos*2+1, middle+1, right);
			tree[pos] = FuncSeg(tree[2*pos], tree[2*pos+1]);
		}
	}
	TRes res(size_t left, size_t right, size_t pos, size_t tree_left, size_t tree_right)
	{
		if(left==tree_left && right==tree_right)
			return tree[pos];
		size_t middle = (tree_left+tree_right)/2;
		if(right<=middle)
			return res(left, right, pos*2, tree_left, middle);
		if(left>middle)
			return res(left, right, pos*2+1, middle+1, tree_right);
		return FuncSeg(res(left, middle, pos*2, tree_left, middle), res(middle+1, right, pos*2+1, middle+1, tree_right));
	}
	void mod(size_t pos_el, TVal x, size_t pos, size_t left, size_t right)
	{
		if(left==right)
			tree[pos] = FuncElem(x);
		else
		{
			size_t middle = (left+right)/2;
			if(pos_el<=middle)
				mod(pos_el, x, pos*2, left, middle);
			else
				mod(pos_el, x, pos*2+1, middle+1, right);
			tree[pos] = FuncSeg(tree[2*pos], tree[2*pos+1]);
		}
	}
	void rebuild(size_t size_new)
	{
		max_pos = size_new;
		tree.clear();
		for(size_t i=data.size();i<size_new;++i)
			data.push_back(neutral);
		build(1, 0, data.size()-1);
	}
public:
	template<class iterator>segments_tree(TRes neut, iterator first, iterator last)
	{
		neutral = neut;
		while(first!=last)
		{
			data.push_back(*first);
			++first;
		}
		last_pos = data.size()-1;
		size_t pow = 1;
		while(pow<=data.size())
			pow *= 2;
		rebuild(pow);
	}
	TRes result(size_t left, size_t right)
	{
		if(right>last_pos)
			throw std::runtime_error("Out of range");
		return res(left, right, 1, 0, data.size()-1);
	}
	void modify(size_t pos ,TVal x)
	{
		data[pos] = x;
		mod(pos, x, 1, 0, data.size()-1);
	}
	void add(TVal x)
	{
		++last_pos;
		if(last_pos==max_pos)
		{
			data.push_back(x);
			rebuild(data.size()*2);
		}
		else
		{
			modify(last_pos, x);
		}
	}
};

void main()
{
}